#include "TMenu.h"

void PopupMenu(HWND hParent, int menu_id) {
    //µ¯³öÓÒ¼ü²Ëµ¥
    HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(menu_id));
    hMenu = GetSubMenu(hMenu, 0);
    POINT pt;
    GetCursorPos(&pt);

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hParent, NULL);
}