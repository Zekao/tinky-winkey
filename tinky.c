#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>

#pragma comment(lib, "advapi32.lib")
#define TINKY_NAME L"Tinky"

SERVICE_STATUS status;
HANDLE service_stop_event;

HANDLE query_system_token(void)
{
    /*
        In this function we will query the system token and use it to spawn the `winkey.exe` process.
        This is done by querying the `winlogon.exe` process and using its token.

        The `winlogon.exe` process is the process that is responsible for the logon screen.        
        @return If successful, it returns the handle to the queried system token, otherwise it returns NULL.
    */

    PROCESSENTRY32 processEntry32;
    processEntry32.dwSize = sizeof(PROCESSENTRY32);

    // Get a snapshot of the current running processes on the system.
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    DWORD sessionID = WTSGetActiveConsoleSessionId();
    
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to create a snapshot of the processes. (error code %d)", (int)GetLastError());
        return NULL;
    }
    
    if (Process32First(hSnapshot, &processEntry32)) {
        do {
            // Check if the current process is winlogon.exe
            if (_stricmp(processEntry32.szExeFile, "winlogon.exe") == 0) {
                DWORD processID = processEntry32.th32ProcessID;
                if (processID == sessionID) {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
                    if (hProcess != NULL) {
                        DWORD tokenAccess = TOKEN_QUERY | TOKEN_DUPLICATE;
                        HANDLE hToken = NULL;
                        if (OpenProcessToken(hProcess, tokenAccess, &hToken)) {
                            CloseHandle(hProcess);
                            CloseHandle(hSnapshot);
                            return hToken;
                        } else {
                            fprintf(stderr, "Failed to open the process token. (error code %d)", (int)GetLastError());
                            CloseHandle(hProcess);
                            CloseHandle(hSnapshot);
                            return NULL;
                        }
                    } else {
                        fprintf(stderr, "Failed to open the process. (error code %d)", (int)GetLastError());
                        CloseHandle(hSnapshot);
                        return NULL;
                    }
                }
            }
        } while (Process32Next(hSnapshot, &processEntry32));
    }

    fprintf(stderr, "Failed to find the winlogon.exe process.");
    CloseHandle(hSnapshot);
    return NULL;
}

// This function is called when an even occurs. For example, when the user clicks on the `stop` button in the service manager.
DWORD WINAPI svc_ctrl_handler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    (void)lpContext;
    (void)lpEventData;
    (void)dwEventType;
    if (dwControl == SERVICE_CONTROL_STOP) {
        if (status.dwCurrentState != SERVICE_RUNNING)
            return 0;
        
        SetEvent(service_stop_event);
    }
    return 0;
}

// The service entry point called by the operating system when initializing the service.
//
// I'm not sure why we need this, but I think the service manager will call us on a special thread.
VOID WINAPI svc_main(DWORD argc, LPTSTR *argv)
{
    (void)argc;
    (void)argv;
    fprintf(stderr, "Initializing the service...\n");

    SERVICE_STATUS_HANDLE status_handle = RegisterServiceCtrlHandlerW(TINKY_NAME, svc_ctrl_handler); // warning C4113: 'void (__stdcall *)(DWORD,DWORD,LPVOID,LPVOID)' est différent de 'LPHANDLER_FUNCTION' dans les listes de paramètres
    if (!status_handle)
    {
        fprintf(stderr, "Failed to register the service handler. (error code %d)", (int)GetLastError());
        return ;
    }

    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;  // We own a process.
    status.dwCurrentState = SERVICE_START_PENDING;     // We are currently starting the service.
    status.dwControlsAccepted = 0;                     // The service cannot be stopped yet (we're starting...)
    status.dwWin32ExitCode = 0;                        // There is currently no error.
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
    SetServiceStatus(status_handle, &status);

    fprintf(stderr, "Starting the subprocess...");

    // TODO: Use `CreateProcessWithTokenW` here and use the system token.
    STARTUPINFOW startup_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(STARTUPINFOW);
    startup_info.lpDesktop = L"";
    startup_info.lpTitle = L"Tinkey";
    
    HANDLE system_token = query_system_token();
    PROCESS_INFORMATION process_info;
    if (CreateProcessAsUserW(system_token, L"C:\\Users\\Buste\\Documents\\GitHub\\tinky-winkey\\winkey.exe", L"winkey.exe", NULL, NULL, TRUE, BELOW_NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &process_info) == FALSE)
    {
        fprintf(stderr, "Failed to spawn Tinkey. (error code %d)", (int)GetLastError());

        status.dwCurrentState = SERVICE_STOPPED;
        status.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &status);

        return ;
    }

    service_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!service_stop_event)
    {
        fprintf(stderr, "Failed to create the STOP event. (error code %d)", (int)GetLastError());

        status.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &status);

        return ;
    }

    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;                 // The service can now be stopped.
    status.dwCurrentState = SERVICE_RUNNING;                         // The service is now running.
    SetServiceStatus(status_handle, &status);

    // Wait for the STOP event.
    if (WaitForSingleObject(service_stop_event, INFINITE) == WAIT_FAILED) 
    {
        fprintf(stderr, "Failed to wait for the STOP event. (error code %d)", (int)GetLastError());

        status.dwCurrentState = SERVICE_STOPPED;
        status.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &status);

        CloseHandle(service_stop_event);
        return ;
    }

    CloseHandle(service_stop_event);

    status.dwCurrentState = SERVICE_STOP_PENDING;
    SetServiceStatus(status_handle, &status);

    TerminateProcess(process_info.hProcess, 0);

    status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(status_handle, &status);

    return ;
}

int main(void)
{
    SERVICE_TABLE_ENTRYW table[] = {
        { TINKY_NAME, (LPSERVICE_MAIN_FUNCTIONW)svc_main },
        {NULL, NULL}, // The table is terminated with a null entry.
    };

    if (StartServiceCtrlDispatcherW(table) == FALSE)
    {
        fprintf(stderr, "Failed to connect the main thread to the Service Controll Dispatcher. (error code %d)\n", (int)GetLastError());
        return 1;
    }

    return 0;
}