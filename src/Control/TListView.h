#pragma once

#include <tstring.h>

#include <atlbase.h>        // 基本的ATL类
#include <atlwin.h>         // ATL窗口类
#include <atlapp.h>     // WTL 主框架窗口类
#include <atlctrls.h>

#include <vector>

class TListView : public CListViewCtrl
{
public:

	std::vector<int> GetSelectedItems() const;

	std::tstring GetItemText(int nItem, int nSubItem) const;
};