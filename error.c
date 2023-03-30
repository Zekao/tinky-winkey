#include <windows.h>
#include <stdio.h>

static void print_last_error(char const *message)
{
    DWORD code = GetLastError();
    wchar_t buf[256] = L"";

    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        code,
        0,
        buf,
        sizeof(buf),
        NULL
    );

    if (size == 0)
    {
        fprintf(stderr, "%s (error code %ld)\n", message, code);
        return;
    }

    char utf8[256] = "";

    int size_utf8 = WideCharToMultiByte(CP_UTF8, 0, buf, (int)size, utf8, 255, NULL, NULL);

    utf8[size_utf8] = '\0';

    fprintf(stderr, "%s: %s (error code %ld)\n", message, utf8, code);
}