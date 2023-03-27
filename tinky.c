#include <stdio.h>
#include <windows.h>

#define TINKY_NAME L"Tinky"

SERVICE_STATUS status;
HANDLE service_stop_event;

DWORD WINAPI svc_ctrl_handler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    if (dwControl == SERVICE_CONTROL_STOP) {
        if (status.dwCurrentState != SERVICE_RUNNING)
            return 0;
        
        SetEvent(service_stop_event);
    }
    return 0;
}

// The service entry point called by the operating system when initializing the service.
VOID WINAPI svc_main(DWORD argc, LPTSTR *argv)
{
    fprintf(stderr, "Initializing the service...\n");

    SERVICE_STATUS_HANDLE status_handle = RegisterServiceCtrlHandlerW(TINKY_NAME, svc_ctrl_handler);
    if (!status_handle)
    {
        fprintf(stderr, "Failed to register the service handler. (error code %d)", GetLastError());
        return 1;
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
    
    PROCESS_INFORMATION process_info;
    if (CreateProcessW(L"C:\\Users\\Buste\\Documents\\GitHub\\tinky-winkey\\winkey.exe", L"winkey.exe", NULL, NULL, TRUE, BELOW_NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &process_info) == FALSE)
    {
        fprintf(stderr, "Failed to spawn Tinkey. (error code %d)", GetLastError());

        status.dwCurrentState = SERVICE_STOPPED;
        status.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &status);

        return 1;
    }

    service_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!service_stop_event)
    {
        fprintf(stderr, "Failed to create the STOP event. (error code %d)", GetLastError());

        status.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &status);

        return 1;
    }

    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;                 // The service can now be stopped.
    status.dwCurrentState = SERVICE_RUNNING;                         // The service is now running.
    SetServiceStatus(status_handle, &status);

    // Wait for the STOP event.
    if (WaitForSingleObject(service_stop_event, INFINITE) == WAIT_FAILED) 
    {
        fprintf(stderr, "Failed to wait for the STOP event. (error code %d)", GetLastError());

        status.dwCurrentState = SERVICE_STOPPED;
        status.dwWin32ExitCode = 1;
        SetServiceStatus(status_handle, &status);

        CloseHandle(service_stop_event);
        return 1;
    }

    CloseHandle(service_stop_event);

    status.dwCurrentState = SERVICE_STOP_PENDING;
    SetServiceStatus(status_handle, &status);

    TerminateProcess(process_info.hProcess, 0);

    status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(status_handle, &status);

    return 0;
}

int main(void)
{
    SERVICE_TABLE_ENTRYW table[] = {
        { TINKY_NAME, (LPSERVICE_MAIN_FUNCTIONW)svc_main },
        {NULL, NULL}, // The table is terminated with a null entry.
    };

    if (StartServiceCtrlDispatcherW(table) == FALSE)
    {
        fprintf(stderr, "Failed to connect the main thread to the Service Controll Dispatcher. (error code %d)\n", GetLastError());
        return 1;
    }

    return 0;
}