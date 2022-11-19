#include "TListView.h"

#include <stdexcept>
#include <cassert>

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
	assert(::IsWindow(this->m_hWnd));

	tstring ans;
	LVITEM lvi = {};
	lvi.iSubItem = nSubItem;

	for (int nLen = 256; ; nLen *= 2)
	{
		ans.resize(nLen);

		lvi.cchTextMax = nLen;
		lvi.pszText = ans.data();
		int nRes = (int)::SendMessage(this->m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)&lvi);
		if (nRes < nLen - 1)
		{
			ans.resize(nRes);
			break;
		}
	}

	return ans;
}
