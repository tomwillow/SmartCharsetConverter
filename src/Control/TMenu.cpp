#include "TMenu.h"

#include <cassert>

TMenu::TMenu() noexcept {
    hMenu = CreateMenu();
}

void TMenu::SetItemEnable(int itemId, bool enabled) noexcept {
    int lastStatus = EnableMenuItem(hMenu, itemId, enabled ? MF_ENABLED : MF_DISABLED);
    assert(lastStatus >= 0);
}

TMenu &TMenu::SetItemToBeContainer(int itemId) noexcept {
    assert(children.find(itemId) == children.end());

    TMenu child;

    MENUITEMINFO menuItemInfo = {0};
    menuItemInfo.cbSize = sizeof(MENUITEMINFO);
    menuItemInfo.fMask = MIIM_SUBMENU;
    menuItemInfo.hSubMenu = child.hMenu;

    BOOL ok = SetMenuItemInfo(hMenu, itemId, FALSE, &menuItemInfo);
    assert(ok);

    children.insert({itemId, child});
    return children[itemId];
}

void TMenu::AppendItem(int id, const std::wstring &s) noexcept {
    BOOL ok = AppendMenu(hMenu, MF_STRING, id, s.c_str());
    assert(ok);
}

void TMenu::InsertItem(int posId, int newItemid, const std::wstring &s) noexcept {
    HMENU hMenuPopup = CreateMenu();

    MENUITEMINFO menuItemInfo = {0};
    menuItemInfo.cbSize = sizeof(MENUITEMINFO);
    menuItemInfo.wID = newItemid;
    menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
    menuItemInfo.cch = s.length() + 1;
    menuItemInfo.dwTypeData = const_cast<wchar_t *>(s.c_str());
    menuItemInfo.hSubMenu = hMenu;

    BOOL ok = InsertMenuItem(hMenu, posId, FALSE, &menuItemInfo);
    assert(ok);
}

std::wstring TMenu::GetItemString(int pos) noexcept {
    MENUITEMINFO menuItemInfo = {0};

    menuItemInfo.cbSize = sizeof(MENUITEMINFO);
    menuItemInfo.fMask = MIIM_STRING;
    BOOL ok = GetMenuItemInfo(hMenu, pos, TRUE, &menuItemInfo);
    assert(ok);

    menuItemInfo.cch++;
    std::wstring ws(menuItemInfo.cch, '\0');
    menuItemInfo.dwTypeData = const_cast<wchar_t *>(ws.data());
    ok = GetMenuItemInfo(hMenu, pos, TRUE, &menuItemInfo);
    assert(ok);
    return ws;
}

TMenu &TMenu::GetChild(int itemId) {
    return children.at(itemId);
}

TPopupMenu::TPopupMenu(int menuId) noexcept {
    hRoot = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(menuId));
    assert(hRoot);
    hMenu = GetSubMenu(hRoot, 0);
    assert(hMenu);
}

void TPopupMenu::Popup(HWND hParent) noexcept {
    POINT pt;
    GetCursorPos(&pt);

    BOOL ok = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hParent, NULL);
    assert(ok);
}