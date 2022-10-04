#include "DialogMain.h"

#include <tstring.h>
#include <FileFunction.h>
#include "Control/TMenu.h"

#ifdef _DEBUG
#include <iostream>
#endif

#include <stdexcept>
#include <sstream>
#include <set>

using namespace std;

DialogMain::DialogMain() :core(TEXT("SmartCharsetConverter.ini"))
{
}


void DialogMain::OnClose()
{
	EndDialog(0);
}

BOOL DialogMain::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	// 设置窗口的大小图标
	// 大图标：按下alt+tab键切换窗口时对应的图标
	// 小图标：就是窗口左上角对应的那个图标
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	SetWindowText(TEXT("智能编码集转换器 v0.1"));

	//
	SetFilterMode(core.GetConfig().filterMode);
	static_cast<CButton>(GetDlgItem(IDC_CHECK_INCLUDE_TEXT)).SetCheck(core.GetConfig().enableIncludeRule);
	static_cast<CButton>(GetDlgItem(IDC_CHECK_EXCLUDE_TEXT)).SetCheck(core.GetConfig().enableExcludeRule);
	//GetDlgItem(IDC_EDIT_INCLUDE_TEXT).SetWindowTextW(core.GetConfig().includeRule);

	// target
	SetOutputTarget(core.GetConfig().outputTarget);
	GetDlgItem(IDC_EDIT_OUTPUT_DIR).SetWindowTextW(core.GetConfig().outputDir.c_str());
	static_cast<CEdit>(GetDlgItem(IDC_EDIT_OUTPUT_DIR)).SetReadOnly(true);

	SetOutputCharset(core.GetConfig().outputCharset);

	// listview
	listview.Attach(GetDlgItem(IDC_LISTVIEW));
	listview.ModifyStyle(0, LVS_REPORT);
	listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	listview.AddColumn(TEXT("序号"), static_cast<int>(ListViewColumn::INDEX));
	listview.SetColumnWidth(0, 40);

	listview.AddColumn(TEXT("文件名"), static_cast<int>(ListViewColumn::FILENAME));
	listview.SetColumnWidth(1, 300);

	listview.AddColumn(TEXT("大小"), static_cast<int>(ListViewColumn::FILESIZE));
	listview.SetColumnWidth(2, 60);

	listview.AddColumn(TEXT("编码"), static_cast<int>(ListViewColumn::ENCODING));
	listview.SetColumnWidth(3, 60);

	listview.AddColumn(TEXT("文本片段"), static_cast<int>(ListViewColumn::TEXT_PIECE));
	listview.SetColumnWidth(4, 200);

	setlocale(LC_CTYPE, "");

	CenterWindow();

	return 0;
}

void DialogMain::SetFilterMode(Configuration::FilterMode mode)
{
	core.SetFilterMode(mode);
	bool isSmart = (mode == Configuration::FilterMode::SMART);
	CButton(GetDlgItem(IDC_RADIO_STRETEGY_SMART)).SetCheck(isSmart);
	CButton(GetDlgItem(IDC_RADIO_STRETEGY_MANUAL)).SetCheck(!isSmart);

	GetDlgItem(IDC_CHECK_INCLUDE_TEXT).EnableWindow(!isSmart);
	GetDlgItem(IDC_EDIT_INCLUDE_TEXT).EnableWindow(!isSmart);
	GetDlgItem(IDC_CHECK_EXCLUDE_TEXT).EnableWindow(!isSmart);
	GetDlgItem(IDC_EDIT_EXCLUDE_TEXT).EnableWindow(!isSmart);
}

void DialogMain::SetOutputTarget(Configuration::OutputTarget outputTarget)
{
	core.SetOutputTarget(outputTarget);
	bool isToOrigin = (outputTarget == Configuration::OutputTarget::ORIGIN);

	CButton(GetDlgItem(IDC_RADIO_TO_ORIGIN)).SetCheck(isToOrigin);
	CButton(GetDlgItem(IDC_RADIO_TO_DIR)).SetCheck(!isToOrigin);

	GetDlgItem(IDC_EDIT_OUTPUT_DIR).EnableWindow(!isToOrigin);
	GetDlgItem(IDC_BUTTON_SET_OUTPUT_DIR).EnableWindow(!isToOrigin);
}

void DialogMain::SetOutputCharset(Configuration::OutputCharset charset)
{
	core.SetOutputCharset(charset);
	bool isNormalCharset = Configuration::IsNormalCharset(charset);

	CButton(GetDlgItem(IDC_RADIO_UTF8)).SetCheck(charset == Configuration::OutputCharset::UTF8);
	CButton(GetDlgItem(IDC_RADIO_UTF8BOM)).SetCheck(charset == Configuration::OutputCharset::UTF8BOM);
	CButton(GetDlgItem(IDC_RADIO_GB18030)).SetCheck(charset == Configuration::OutputCharset::GB18030);
	CButton(GetDlgItem(IDC_RADIO_OTHER)).SetCheck(Configuration::IsNormalCharset(charset) == false);

	GetDlgItem(IDC_COMBO_OTHER_CHARSET).EnableWindow(!isNormalCharset);

}

void DialogMain::AddItem(const std::tstring &filename)
{
	auto count = listview.GetItemCount();
	listview.AddItem(count, static_cast<int>(ListViewColumn::INDEX), to_tstring(count + 1).c_str());
	listview.AddItem(count, static_cast<int>(ListViewColumn::FILENAME), filename.c_str());
	listview.AddItem(count, static_cast<int>(ListViewColumn::FILESIZE), FileSizeToTString(GetFileSize(filename)).c_str());
	listview.AddItem(count, static_cast<int>(ListViewColumn::ENCODING), core.GetEncodingStr(filename).c_str());

	unique_ptr<char[]> buf;
	uint64_t bufSize;
	ReadFileToBuffer(filename, buf, bufSize, 64);

	// TODO 使用icu解码

	listview.AddItem(count, static_cast<int>(ListViewColumn::TEXT_PIECE), to_tstring(buf.get()).c_str());
}

void DialogMain::AddItems(const std::vector<std::tstring> &filenames)
{
	vector<pair<tstring, tstring>> failed;
	for (auto &filename : filenames)
	{
		try
		{
			// 剔除重复的
			if (listFileNames.find(filename) != listFileNames.end())
			{
				failed.push_back({ filename,TEXT("重复添加") });
			}

			AddItem(filename);

			listFileNames.insert(filename);
		}
		catch (runtime_error &e)
		{
			failed.push_back({ filename,to_tstring(e.what()) });

			// 移除
			listview.DeleteItem(listview.GetItemCount() - 1);
		}
	}

	if (failed.empty() == false)
	{
		tstring info = TEXT("以下文件添加失败：\r\n");
		for (auto &pr : failed)
		{
			info += pr.first + TEXT(" 原因：") + pr.second + TEXT("\r\n");
		}
		MessageBox(info.c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
	}
}

LRESULT DialogMain::OnBnClickedRadioStretegySmart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetFilterMode(Configuration::FilterMode::SMART);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioStretegyManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetFilterMode(Configuration::FilterMode::MANUAL);
	return 0;
}


LRESULT DialogMain::OnBnClickedCheckIncludeText(WORD /*wNotifyCode*/, WORD wID/*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	core.EnableRule(true, static_cast<CButton>(GetDlgItem(wID)).GetCheck());
	return 0;
}


LRESULT DialogMain::OnBnClickedCheckExcludeText(WORD /*wNotifyCode*/, WORD wID/*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	core.EnableRule(false, static_cast<CButton>(GetDlgItem(wID)).GetCheck());
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioToOrigin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputTarget(Configuration::OutputTarget::ORIGIN);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioToDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputTarget(Configuration::OutputTarget::TO_DIR);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioUtf8(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(Configuration::OutputCharset::UTF8);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioUtf8bom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(Configuration::OutputCharset::UTF8BOM);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioGb18030(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(Configuration::OutputCharset::GB18030);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioOther(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(Configuration::OutputCharset::OTHER_UNSPECIFIED);
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	//

	TFileDialog dialog(*this, { {L"所有文件",L"*.*"}, { L"文本文件",L"*.txt" } }, true);
	dialog.SetTitle(L"dkfjdk");
	if (dialog.Open())
	{
		auto ans = dialog.GetResult();

		AddItems(ans);
	}
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonAddDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	tstring dir;

	TFolderBrowser folderBrowser(*this);
	if (folderBrowser.Open(dir))
	{
		auto filenames = TraversalAllFileNames(dir);
		AddItems(filenames);
	}

	return 0;
}


LRESULT DialogMain::OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码

	return 0;
}


LRESULT DialogMain::OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	listview.DeleteAllItems();
	listFileNames.clear();
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonSetOutputDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	tstring dir = core.GetConfig().outputDir;

	TFolderBrowser folderBrowser(*this);
	if (folderBrowser.Open(dir))
	{
		core.SetOutputDir(dir);
		GetDlgItem(IDC_EDIT_OUTPUT_DIR).SetWindowTextW(dir.c_str());
	}

	return 0;
}


LRESULT DialogMain::OnCbnSelchangeComboOtherCharset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码

	return 0;
}


LRESULT DialogMain::OnNMRclickListview(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & /*bHandled*/)
{
	auto selectedItems = listview.GetSelectedItems();
	if (selectedItems.empty() == false)
	{
		PopupMenu(this->m_hWnd, IDR_MENU_RIGHT);
	}

	return 0;
}


LRESULT DialogMain::OnOpenWithNotepad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	auto selectedItems = listview.GetSelectedItems();
	for (auto i : selectedItems)
	{
		auto filename = listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME));

		wstring cmd = L"notepad " + filename;

		WinExec(to_string(cmd).c_str(), SW_SHOWNORMAL);
	}

	return 0;
}


LRESULT DialogMain::OnRemoveItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	auto selectedItems = listview.GetSelectedItems();
	for (auto itor = selectedItems.rbegin(); itor != selectedItems.rend(); ++itor)
	{
		int i = *itor;
		auto filename = listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME));
		listview.DeleteItem(i);
		listFileNames.erase(filename);
	}
	return 0;
}
