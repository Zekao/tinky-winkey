#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#pragma warning(disable:4820)
#pragma warning(disable: 5045)
#include <tlhelp32.h>

#include "error.c"

#pragma comment(lib, "advapi32.lib")
#define TINKY_NAME L"Tinky"

SERVICE_STATUS st;
HANDLE service_stop_event;

static HANDLE query_system_token(void) {

    PROCESSENTRY32 pe32;

    pe32.dwSize = sizeof( PROCESSENTRY32 );
    HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    DWORD sessionID = WTSGetActiveConsoleSessionId();

    if (Process32First(hProcessSnap, &pe32)) {

        do {
            if (lstrcmpi(pe32.szExeFile, TEXT("winlogon.exe")) == 0) {
                DWORD ProcessSessionId = 0;
                ProcessIdToSessionId(pe32.th32ProcessID, &ProcessSessionId);
                if (ProcessSessionId == sessionID) {
                    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
                    HANDLE hToken = NULL;
	                OpenProcessToken(hProcess, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &hToken);
                    HANDLE hNewToken = NULL;
                    DuplicateTokenEx(hToken, PROCESS_ALL_ACCESS, NULL, SecurityImpersonation, TokenImpersonation, &hNewToken);
                    CloseHandle(hProcessSnap);
                    CloseHandle(hProcess);
	                CloseHandle(hToken);
                    return hNewToken;
                }
            }
        } while(Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);
    return NULL;
}

// This function is called when an even occurs. For example, when the user clicks on the `stop` button in the service manager.
static DWORD WINAPI svc_ctrl_handler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    (void)lpContext;
    (void)lpEventData;
    (void)dwEventType;
    if (dwControl == SERVICE_CONTROL_STOP) {
        if (st.dwCurrentState != SERVICE_RUNNING)
            return 0;
        
        SetEvent(service_stop_event);
    }
    return 0;
}

static DWORD wait_for_service_state(SC_HANDLE service, DWORD status)
{
    /**
     * Waits until the specified services changes to a state other than `status`.
     *
     * The new state of the service is returned.
     */
    
    SERVICE_STATUS_PROCESS ft_st;
    DWORD bytes_needed;
    
    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ft_st, sizeof(ft_st), &bytes_needed))
    {
        print_last_error("QueryServiceStatus");
        return 0;
    }

    if (ft_st.dwCurrentState != status)
        return ft_st.dwCurrentState;
    
    size_t count = 0;
    do
    {
        count++;
        Sleep(500);
        printf("waiting...\n");
        if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ft_st, sizeof(ft_st), &bytes_needed))
        {
            print_last_error("QueryServiceStatus");
            return 0;
        }
    } while (ft_st.dwCurrentState == status && count < 5);

    return ft_st.dwCurrentState;
}

// The service entry point called by the operating system when initializing the service.
//
// I'm not sure why we need this, but I think the service manager will call us on a special thread.
static VOID WINAPI svc_main(DWORD argc, LPTSTR *argv)
{
    (void)argc;
    (void)argv;
    fprintf(stderr, "Initializing the service...\n");

    SERVICE_STATUS_HANDLE status_handle = RegisterServiceCtrlHandlerExW(TINKY_NAME, svc_ctrl_handler, NULL);
    if (!status_handle)
    {
        fprintf(stderr, "Failed to register the service handler. (error code %d)", (int)GetLastError());
        return ;
    }

    st.dwServiceType = SERVICE_WIN32_OWN_PROCESS;  // We own a process.
    st.dwCurrentState = SERVICE_START_PENDING;     // We are currently starting the service.
    st.dwControlsAccepted = 0;                     // The service cannot be stopped yet (we're starting...)
    st.dwWin32ExitCode = 0;                        // There is currently no error.
    st.dwServiceSpecificExitCode = 0;
    st.dwCheckPoint = 0;
    st.dwWaitHint = 0;
    SetServiceStatus(status_handle, &st);

    fprintf(stderr, "Starting the subprocess...");

    // TODO: Use `CreateProcessWithTokenW` here and use the system token.
    STARTUPINFOW startup_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(STARTUPINFOW);
    startup_info.lpDesktop = L"";
    startup_info.lpTitle = L"Tinkey";
    
    HANDLE system_token = query_system_token();
    if (!system_token)
    {
        // on passe pas le service en STOPPED ici.
        
        fprintf(stderr, "Failed to impersonate the SYSTEM token. (error code %d)", (int)GetLastError());

        st.dwCurrentState = SERVICE_STOPPED;
        st.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &st);

        return;
    }

    PROCESS_INFORMATION process_info;
    if (CreateProcessAsUserW(system_token, L"C:\\Users\\Zekao\\Documents\\GitHub\\tinky-winkey\\winkey.exe", L"winkey.exe", NULL, NULL, TRUE, BELOW_NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &process_info) == FALSE)
    {
        fprintf(stderr, "Failed to spawn Tinkey. (error code %d)", (int)GetLastError());

        st.dwCurrentState = SERVICE_STOPPED;
        st.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &st);

        return ;
    }

    service_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!service_stop_event)
    {
        fprintf(stderr, "Failed to create the STOP event. (error code %d)", (int)GetLastError());

        st.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &st);

        return ;
    }

    st.dwControlsAccepted = SERVICE_ACCEPT_STOP;                 // The service can now be stopped.
    st.dwCurrentState = SERVICE_RUNNING;                         // The service is now running.
    SetServiceStatus(status_handle, &st);

    // Wait for the STOP event.
    if (WaitForSingleObject(service_stop_event, INFINITE) == WAIT_FAILED) 
    {
        fprintf(stderr, "Failed to wait for the STOP event. (error code %d)", (int)GetLastError());

        st.dwCurrentState = SERVICE_STOPPED;
        st.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &st);

        CloseHandle(service_stop_event);
        return ;
    }

    CloseHandle(service_stop_event);

    st.dwCurrentState = SERVICE_STOP_PENDING;
    SetServiceStatus(status_handle, &st);

    Sleep(500);
    TerminateProcess(process_info.hProcess, 0);

    st.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(status_handle, &st);

    return ;
}



int main(int ac, char **av)
{
    if (ac > 2)
    {
        fprintf(stderr, "usage: [install|start|stop|delete]");
        return 2;
    }

    if (ac <= 1)
    {
        SERVICE_TABLE_ENTRYW table[] = {
            { TINKY_NAME, (LPSERVICE_MAIN_FUNCTIONW)svc_main },
            {NULL, NULL}, // The table is terminated with a null entry.
        };

        if (StartServiceCtrlDispatcherW(table) == FALSE)
        {
            print_last_error("StartServiceCtrlDispatcher");
            return 1;
        }

        return 0;
    }
    
    if (strcmp(av[1], "install") == 0)
    {
        // Create the service.

        // Open a connection with the local service manager.

        SC_HANDLE scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (!scm)
        {
            print_last_error("OpenSCManager");
            return 1;
        }

        wchar_t name_buf[MAX_PATH];
        if (GetModuleFileNameW(NULL, name_buf, sizeof(name_buf)) == 0)
        {
            print_last_error("GetModuleFileName");
            return 1;
        }

        SC_HANDLE service = CreateServiceW(
            scm,
            TINKY_NAME,
            L"Tinky Winkey",
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            name_buf,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
        );

        if (!service)
        {
            CloseServiceHandle(scm);

            print_last_error("CreateService");
            return 1;
        }

        printf("Tinky has been created.");

        CloseServiceHandle(scm);
        CloseServiceHandle(service);
    }
    else if (strcmp(av[1], "start") == 0)
    {
        SC_HANDLE scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!scm)
        {
            print_last_error("OpenSCManager");
            return 1;
        }

        SC_HANDLE service = OpenServiceW(scm, TINKY_NAME, SERVICE_ALL_ACCESS);
        if (!service)
        {
            CloseServiceHandle(scm);
            print_last_error("OpenService");
            return 1;
        }

        DWORD cur_state = wait_for_service_state(service, SERVICE_STOP_PENDING);
        
        if (cur_state == 0)
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return 1;
        }

        if (cur_state != SERVICE_STOPPED)
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            fprintf(stderr, "The service is already running!");
            return 1;
        }

        if (!StartServiceW(service, 0, NULL))
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            print_last_error("StartService");
            return 1;
        }
        
        cur_state = wait_for_service_state(service, SERVICE_START_PENDING);

        if (cur_state == 0)
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return 1;
        }

        if (cur_state == SERVICE_RUNNING)
            printf("Tinky has started!");
        else
        {
            printf("Failed to start Tinky. (state %ld)", cur_state);
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return 1;
        }

        CloseServiceHandle(service);
        CloseServiceHandle(scm);
    }
    else if (strcmp(av[1], "stop") == 0)
    {
        SC_HANDLE scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!scm)
        {
            print_last_error("OpenSCManager");
            return 1;
        }

        SC_HANDLE service = OpenServiceW(scm, TINKY_NAME, SERVICE_ALL_ACCESS);
        if (!service)
        {
            CloseServiceHandle(scm);
            print_last_error("OpenSevice");
            return 1;
        }
        
        DWORD cur_state = wait_for_service_state(service, SERVICE_START_PENDING);

        if (cur_state == 0)
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return 1;
        }

        if (cur_state != SERVICE_RUNNING)
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            fprintf(stderr, "Tinky is not running...");
            return 1;
        }

        SERVICE_STATUS service_status;

        if (!ControlService(service, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&service_status))
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            print_last_error("ControlService");
            return 1;
        }

        cur_state = wait_for_service_state(service, SERVICE_STOP_PENDING);

        if (cur_state == 0)
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return 1;
        }

        if (cur_state == SERVICE_STOPPED)
            printf("Tinky has been killed. What a monster!");
        else
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            printf("Failed to stop Tinky. (state %ld)", cur_state);
            return 1;
        }
        
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
    }
    else if (strcmp(av[1], "delete") == 0) {
        
        SC_HANDLE service;

        SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!scm) {
            print_last_error("OpenSCManager");
            return 1;
        }

        service = OpenServiceW(scm, TINKY_NAME, SC_MANAGER_ALL_ACCESS);
        if (!service)
        {
            print_last_error("OpenService");
            CloseServiceHandle(scm);
            return 1;
        }
        
        DWORD cur_state = wait_for_service_state(service, SERVICE_STOP_PENDING);
    
        if (cur_state == 0)
        {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return 1;
        }

        if (cur_state != SERVICE_STOPPED)
        {
            fprintf(stderr, "Tinky is running.");
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return 1;
        }

        if (!DeleteService(service)) {
            print_last_error("DeleteService");
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return 1;
        }

        printf("Tinky has been deleted.");

        CloseServiceHandle(service);
        CloseServiceHandle(scm);
    }
    else
    {
        fprintf(stderr, "usage: [install|start|stop|delete]");
        return 2;
    }

    return 0;
}