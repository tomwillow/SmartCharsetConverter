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
#include <regex>

const std::tstring appTitle = TEXT("智能编码集转换器 v0.1 by Tom Willow");

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

	SetWindowText(appTitle.c_str());

	// 包含/排除指定后缀
	SetFilterMode(core.GetConfig().filterMode);
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

	GetDlgItem(IDC_EDIT_INCLUDE_TEXT).EnableWindow(!isSmart);
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

void DialogMain::SetOutputCharset(CharsetCode charset)
{
	core.SetOutputCharset(charset);
	bool isNormalCharset = Configuration::IsNormalCharset(charset);

	CButton(GetDlgItem(IDC_RADIO_UTF8)).SetCheck(charset == CharsetCode::UTF8);
	CButton(GetDlgItem(IDC_RADIO_UTF8BOM)).SetCheck(charset == CharsetCode::UTF8BOM);
	CButton(GetDlgItem(IDC_RADIO_GB18030)).SetCheck(charset == CharsetCode::GB18030);
	CButton(GetDlgItem(IDC_RADIO_OTHER)).SetCheck(Configuration::IsNormalCharset(charset) == false);

	GetDlgItem(IDC_COMBO_OTHER_CHARSET).EnableWindow(!isNormalCharset);

}

bool DialogMain::AddItem(const std::tstring &filename)
{
	// 识别字符集
	auto [charsetName, content, contentSize] = core.GetEncodingStr(filename);

	try
	{
		auto count = listview.GetItemCount();
		listview.AddItem(count, static_cast<int>(ListViewColumn::INDEX), to_tstring(count + 1).c_str());
		listview.AddItem(count, static_cast<int>(ListViewColumn::FILENAME), filename.c_str());
		listview.AddItem(count, static_cast<int>(ListViewColumn::FILESIZE), FileSizeToTString(GetFileSize(filename)).c_str());

		listview.AddItem(count, static_cast<int>(ListViewColumn::ENCODING), charsetName.c_str());

		listview.AddItem(count, static_cast<int>(ListViewColumn::TEXT_PIECE), reinterpret_cast<wchar_t *>(content.get()));

	}
	catch (runtime_error &err)
	{
		// 如果AddItem之后出错，移除掉错误条目
		listview.DeleteItem(listview.GetItemCount() - 1);
		throw err;
	}

	return content != nullptr;
}

void DialogMain::AddItems(const std::vector<std::tstring> &filenames)
{
	vector<pair<tstring, tstring>> failed;
	for (auto &filename : filenames)
	{
		try
		{
			// 如果重复了
			if (listFileNames.find(filename) != listFileNames.end())
			{
				failed.push_back({ filename,TEXT("重复添加") });
				continue;	// 不重复添加了
			}
			AddItem(filename);

			listFileNames.insert(filename);
		}
		catch (runtime_error &e)
		{
			failed.push_back({ filename,to_tstring(e.what()) });
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
	SetFilterMode(Configuration::FilterMode::ONLY_SOME_EXTANT);
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
	SetOutputCharset(CharsetCode::UTF8);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioUtf8bom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(CharsetCode::UTF8BOM);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioGb18030(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(CharsetCode::GB18030);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioOther(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	//SetOutputCharset(Configuration::OutputCharset::OTHER_UNSPECIFIED);
	return 0;
}

void DialogMain::CheckAndTraversalIncludeRule(std::function<void(const std::tstring &dotExt)> fn)
{
	// 后缀字符串
	auto &extsStr = core.GetConfig().includeRule;

	// 切分
	auto exts = Split(extsStr, TEXT(" "));

	// 如果为空
	if (exts.empty())
	{
		throw runtime_error("指定的后缀无效。\r\n\r\n例子：*.h *.hpp *.c *.cpp *.txt");
	}

	// 逐个检查
	for (auto ext : exts)
	{
		tstring extStr(ext);
		wstring pattern = TEXT(R"(\*(\.\w+))");	// 匹配 *.xxx 的正则
		wregex r(pattern);
		wsmatch results;
		if (regex_match(extStr, results, r) == false)
		{
			throw runtime_error("指定的后缀无效：" + to_string(extStr) + "。\r\n\r\n例子： * .h * .hpp * .c * .cpp * .txt");
		}

		fn(results.str(1));
	}

}

LRESULT DialogMain::OnBnClickedButtonAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)try
{
	vector<pair<tstring, tstring>> dialogFilter;
	if (core.GetConfig().filterMode == Configuration::FilterMode::SMART)
	{
		// 智能识别所有文本

		dialogFilter = { { L"所有文件*.*", L"*.*" } };
	}
	else
	{
		// 只包括指定后缀

		tstring filterExtsStr;	// dialog的过滤器要求;分割
		CheckAndTraversalIncludeRule([&](const tstring &dotExt)
			{
				filterExtsStr += TEXT("*") + dotExt + TEXT(";");
			});

		// dialog过滤器
		dialogFilter.push_back(make_pair(filterExtsStr, filterExtsStr));
	}

	// 打开文件对话框
	TFileDialog dialog(*this, dialogFilter, true);
	if (dialog.Open())
	{
		auto ans = dialog.GetResult();

		AddItems(ans);
	}
	return 0;
}
catch (runtime_error &err)
{
	MessageBox(to_tstring(err.what()).c_str(), TEXT("出错"), MB_OK | MB_ICONERROR);
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonAddDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)try
{
	// 存储遍历文件时要保留的后缀
	vector<tstring> filterDotExts;

	if (core.GetConfig().filterMode == Configuration::FilterMode::SMART)
	{
		// 智能识别所有文本
	}
	else
	{
		// 只包括指定后缀

		CheckAndTraversalIncludeRule([&](const tstring &dotExt)
			{
				filterDotExts.push_back(dotExt);
			});
	}

	tstring dir;

	TFolderBrowser folderBrowser(*this);
	if (folderBrowser.Open(dir))
	{
		// 遍历指定目录
		auto filenames = TraversalAllFileNames(dir, filterDotExts);

		if (filenames.empty())
		{
			MessageBox((TEXT("指定的目录没有符合的文件：") + dir).c_str(), TEXT("提示"), MB_OK | MB_ICONERROR);
			return 0;
		}

		AddItems(filenames);
	}

	return 0;
}
catch (runtime_error &err)
{
	MessageBox(to_tstring(err.what()).c_str(), TEXT("出错"), MB_OK | MB_ICONERROR);
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL &bHandle /*bHandled*/)try
{
	// 如果没有内容
	if (listview.GetItemCount() == 0)
	{
		throw runtime_error("没有待转换的文件。");
	}

	// 检查输出目录
	if (core.GetConfig().outputTarget != Configuration::OutputTarget::ORIGIN)
	{
		if (core.GetConfig().outputDir.empty())
		{
			throw runtime_error("输出目录无效。");
		}
	}

	vector<tstring> allOutputFileNames;	// 全部文件（成功失败均有）
	vector<pair<tstring, tstring>> failed;	// 失败文件/失败原因
	vector<tstring> succeed;	// 成功的文件

	// 目标编码
	auto targetCode = core.GetConfig().outputCharset;

	// 逐个转换
	for (int i = 0; i < listview.GetItemCount(); ++i)
	{
		auto filename = listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME));
		try
		{
			// 计算目标文件名
			auto outputFileName = filename;
			if (core.GetConfig().outputTarget != Configuration::OutputTarget::ORIGIN)
			{
				// 纯文件名
				auto pureFileName = GetNameAndExt(outputFileName);

				outputFileName = core.GetConfig().outputDir + TEXT("\\") + pureFileName;
			}

			// 加入到任务列表
			allOutputFileNames.push_back(outputFileName);

			// 取出原编码集
			auto originCode = ToCharsetCode(listview.GetItemText(i, static_cast<int>(ListViewColumn::ENCODING)));
			if (originCode == CharsetCode::UNKNOWN)
			{
				throw runtime_error("未探测出编码集");
			}

			// 编码不一样才转换，否则复制过去
			if (originCode != targetCode)
			{
				auto filesize = GetFileSize(filename);

				// 暂时不做分块转换 TODO

				{
					auto [raw, rawSize] = ReadFileToBuffer(filename);

					// 根据BOM偏移
					const char *rawStart = raw.get();

					// 如果需要抹掉BOM，则把起始位置设置到BOM之后，确保UChar[]不带BOM
					if (HasBom(originCode) && !HasBom(targetCode))
					{
						auto bomSize = BomSize(originCode);
						rawStart += bomSize;
						rawSize -= bomSize;
					}

					// 根据原编码得到Unicode字符串
					auto [buf, bufSize] = Decode(rawStart, rawSize, originCode);

					// 转到目标编码
					auto [ret, retLen] = Encode(buf, bufSize, targetCode);

					// 写入文件

					FILE *fp = _tfopen(outputFileName.c_str(), TEXT("wb"));
					unique_ptr<FILE, function<void(FILE *)>> upFile(fp, [](FILE *fp) { fclose(fp); });

					// 如果需要额外加上BOM，先写入BOM
					if (!HasBom(originCode) && HasBom(targetCode))
					{
						auto bomData = GetBomData(targetCode);

						// 写入BOM
						size_t wrote = fwrite(bomData, BomSize(targetCode), 1, fp);
						if (wrote != 1)
						{
							throw runtime_error("写入失败：" + to_string(outputFileName));
						}
					}

					// 写入正文
					size_t wrote = fwrite(ret.get(), retLen, 1, fp);
					if (wrote != 1)
					{
						throw runtime_error("写入失败：" + to_string(outputFileName));
					}
				}
			}
			else
			{
				// 编码一样

				// 如果不是原位置转换，复制过去
				if (core.GetConfig().outputTarget == Configuration::OutputTarget::TO_DIR)
				{
					bool ok = CopyFile(filename.c_str(), outputFileName.c_str(), false);
					if (!ok)
					{
						throw runtime_error("写入失败：" + to_string(outputFileName));
					}
				}
			}

			// 这个文件成功了
			succeed.push_back(filename);
		}
		catch (runtime_error &e)
		{
			// 这个文件失败了
			failed.push_back({ filename,to_tstring(e.what()) });
		}
	}

	// 已经完成处理

	// 如果有失败的
	if (failed.empty() == false)
	{
		tstringstream ss;
		ss << TEXT("转换成功 ") << succeed.size() << TEXT(" 个文件。\r\n\r\n");
		ss << TEXT("以下文件转换失败：\r\n");
		for (auto &pr : failed)
		{
			ss << pr.first << TEXT(" 原因：") << pr.second << TEXT("\r\n");
		}
		MessageBox(ss.str().c_str(), TEXT("转换结果"), MB_OK | MB_ICONERROR);
	}
	else
	{
		// 全部成功之后
		tstringstream ss;
		ss << TEXT("转换完成！");

		if (targetCode == CharsetCode::GB18030)
		{
			ss << TEXT("\r\n\r\n注意：GB18030在纯英文的情况下和UTF-8编码位重合，所以可能会出现转换后显示为UTF-8编码的情况。");
		}
		MessageBox(ss.str().c_str(), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
	}

	// 清空列表
	OnBnClickedButtonClear(0, 0, 0, bHandle);

	// 把转出的结果再次加载到列表中
	AddItems(allOutputFileNames);

	return 0;
}
catch (runtime_error &err)
{
	MessageBox(to_tstring(err.what()).c_str(), TEXT("出错"), MB_OK | MB_ICONERROR);
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


LRESULT DialogMain::OnEnChangeEditIncludeText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL & /*bHandled*/)
{
	// 取得字符串
	tstring filterStr;

	BSTR bstr = nullptr;
	CEdit edit(hWndCtl);
	edit.GetWindowTextW(bstr);
	filterStr = bstr;
	SysReleaseString(bstr);

	// 直接写入
	core.SetFilterRule(filterStr);

	return 0;
}
