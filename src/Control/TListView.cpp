#include "TListView.h"

#include <stdexcept>

using namespace std;

std::vector<int> TListView::GetSelectedItems() const
{
	std::vector<int> ans;
	int iPos = -1;
	while (1)
	{
		// Get the next selected item
		iPos = GetNextItem(iPos, LVNI_SELECTED);
		if (iPos == -1)
			break;

		ans.push_back(iPos);
	}
	return ans;
}

std::tstring TListView::GetItemText(int nItem, int nSubItem) const
{
	tstring ans;
	BSTR str = NULL;
	auto ok = CListViewCtrl::GetItemText(nItem, nSubItem, str);
	if (!ok)
	{
		throw runtime_error("GetItem fail");
	}
	ans = str;

	::SysReleaseString(str);
	return ans;
}
