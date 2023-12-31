#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <string>
#include <map>

class TMenu {
public:
    TMenu() noexcept;
    virtual ~TMenu() = default;

    void SetItemEnable(int itemId, bool enabled = true) noexcept;

    TMenu &SetItemToBeContainer(int itemId) noexcept;

    void AppendItem(int id, const std::wstring &s) noexcept;

    void InsertItem(int posId, int newItemid, const std::wstring &s) noexcept;

    std::wstring GetItemString(int pos) noexcept;

    void SetItemString(int pos, const std::wstring &s) noexcept;

    TMenu &GetChild(int itemId);

protected:
    HMENU hMenu;
    std::map<int, TMenu> children;
};

class TPopupMenu : public TMenu {
public:
    TPopupMenu(int menuId) noexcept;

    void Popup(HWND hParent) noexcept;

private:
    HMENU hRoot;
};