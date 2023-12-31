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

std::wstring TMenu::GetItemTextByPosition(int pos) noexcept {
    return GetItemTextByPositionOrId(true, pos);
}

std::wstring TMenu::GetItemTextById(int id) noexcept {
    return GetItemTextByPositionOrId(false, id);
}

void TMenu::SetItemTextByPosition(int pos, const std::wstring &s) noexcept {
    SetItemTextByPositionOrId(true, pos, s);
}

void TMenu::SetItemTextById(int id, const std::wstring &s) noexcept {
    SetItemTextByPositionOrId(false, id, s);
}

void TMenu::SetItemTextByPositionOrId(bool byPosition, int posOrId, const std::wstring &s) noexcept {
    std::wstring ws(s);
    ws.push_back(L'\0');

    MENUITEMINFO menuItemInfo = {0};

    menuItemInfo.cbSize = sizeof(MENUITEMINFO);
    menuItemInfo.fMask = MIIM_STRING;
    menuItemInfo.cch = ws.size();
    menuItemInfo.dwTypeData = const_cast<wchar_t *>(ws.data());
    BOOL ok = SetMenuItemInfo(hMenu, posOrId, byPosition, &menuItemInfo);
    assert(ok);
}

TMenu &TMenu::GetChild(int itemId) {
    return children.at(itemId);
}

std::wstring TMenu::GetItemTextByPositionOrId(bool byPosition, int posOrId) noexcept {
    MENUITEMINFO menuItemInfo = {0};

    // 第一次，先拿到cch，即文本长度
    menuItemInfo.cbSize = sizeof(MENUITEMINFO);
    menuItemInfo.fMask = MIIM_STRING;
    BOOL ok = GetMenuItemInfo(hMenu, posOrId, byPosition, &menuItemInfo);
    assert(ok);

    // 增加长度，以容纳尾后0
    menuItemInfo.cch++;
    std::wstring ws(menuItemInfo.cch, L'\0');

    // 把字符串指针指向ws，让系统往ws里写内容
    menuItemInfo.dwTypeData = const_cast<wchar_t *>(ws.data());
    ok = GetMenuItemInfo(hMenu, posOrId, byPosition, &menuItemInfo);
    assert(ok);
    return ws;
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