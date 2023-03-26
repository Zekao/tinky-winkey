#include <windows.h>
#include <stdio.h>
#include <stdint.h>

HHOOK kbd_hook = NULL;

LRESULT hookproc(int code, WPARAM wparam, LPARAM lparam) {
    if (code < 0)
        return CallNextHookEx(kbd_hook, code, wparam, lparam);
        
    KBDLLHOOKSTRUCT const *info = (KBDLLHOOKSTRUCT const *)lparam;

    printf("VK: %d\n", info->vkCode);
    printf("Scan Code: %d\n", info->scanCode);
    printf("UP: %d\n", info->flags & LLKHF_UP != 0);
    GetKeyNameTextW(
        lparam,
        
    )
        
    return 0;
}

int main(void) {
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    
    kbd_hook = SetWindowsHookExW(WH_KEYBOARD_LL, hookproc, hInstance, 0);
    if (!kbd_hook)
    {
        printf("hook installation failed: %d\n", GetLastError());
        return (1);
    }

    MSG msg;
    GetMessageW(&msg, NULL, 0, 0);
}