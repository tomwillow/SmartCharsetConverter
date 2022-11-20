#include "DialogMain.h"

#include <tstring.h>
#include <FileFunction.h>
#include "Control/TMenu.h"

#ifdef _DEBUG
#include <iostream>
#include <cassert>
#endif

#include <stdexcept>
#include <sstream>
#include <set>
#include <regex>

#undef min
#undef max

const std::tstring appTitle = TEXT("智能编码集转换器 v0.31 by Tom Willow");

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

	BOOL bHandle = true;

	// 包含/排除指定后缀
	SetFilterMode(core.GetConfig().filterMode);
	//GetDlgItem(IDC_EDIT_INCLUDE_TEXT).SetWindowTextW(core.GetConfig().includeRule);

	// target
	SetOutputTarget(core.GetConfig().outputTarget);
	GetDlgItem(IDC_EDIT_OUTPUT_DIR).SetWindowTextW(core.GetConfig().outputDir.c_str());
	static_cast<CEdit>(GetDlgItem(IDC_EDIT_OUTPUT_DIR)).SetReadOnly(true);

	SetOutputCharset(core.GetConfig().outputCharset);

	// enable/disable line breaks
	CButton(GetDlgItem(IDC_CHECK_CONVERT_RETURN)).SetCheck(core.GetConfig().enableConvertLineBreaks);
	OnBnClickedCheckConvertReturn(0, 0, 0, bHandle);
	CButton(GetDlgItem(IDC_RADIO_CRLF + static_cast<int>(core.GetConfig().lineBreak))).SetCheck(true);

	// listview
	listview.SubclassWindow(GetDlgItem(IDC_LISTVIEW));	// 必须用SubclassWindow传入句柄，才能让MSG_MAP生效

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

	listview.AddColumn(TEXT("换行符"), static_cast<int>(ListViewColumn::LINE_BREAK));
	listview.SetColumnWidth(4, 60);

	listview.AddColumn(TEXT("文本片段"), static_cast<int>(ListViewColumn::TEXT_PIECE));
	listview.SetColumnWidth(5, 200);

	// 启用拖放
	::DragAcceptFiles(listview, true);

	setlocale(LC_CTYPE, "");

	CenterWindow();

	return 0;
}

void DialogMain::SetFilterMode(Configuration::FilterMode mode)
{
	core.SetFilterMode(mode);

	CButton(GetDlgItem(IDC_RADIO_STRETEGY_NO_FILTER)).SetCheck(false);
	CButton(GetDlgItem(IDC_RADIO_STRETEGY_SMART)).SetCheck(false);
	CButton(GetDlgItem(IDC_RADIO_STRETEGY_MANUAL)).SetCheck(false);
	switch (mode)
	{
	case Configuration::FilterMode::NO_FILTER:
		CButton(GetDlgItem(IDC_RADIO_STRETEGY_NO_FILTER)).SetCheck(true);
		break;
	case Configuration::FilterMode::SMART:
		CButton(GetDlgItem(IDC_RADIO_STRETEGY_SMART)).SetCheck(true);
		break;
	case Configuration::FilterMode::ONLY_SOME_EXTANT:
		CButton(GetDlgItem(IDC_RADIO_STRETEGY_MANUAL)).SetCheck(true);
		break;
	default:
		assert(0);
	}

	GetDlgItem(IDC_EDIT_INCLUDE_TEXT).EnableWindow(mode == Configuration::FilterMode::ONLY_SOME_EXTANT);
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

class io_error_ignore : public std::runtime_error
{
public:
	io_error_ignore() :runtime_error("ignored") {}
};

void DialogMain::AddItem(const std::tstring &filename, const std::unordered_set<std::tstring> &filterDotExts)
{
	// 如果是只包括指定后缀的模式，且文件后缀不符合，则忽略掉，且不提示
	if (core.GetConfig().filterMode == Configuration::FilterMode::ONLY_SOME_EXTANT &&
		filterDotExts.find(TEXT(".") + GetExtend(filename)) == filterDotExts.end())
	{
		return;
	}

	// 如果重复了
	if (listFileNames.find(filename) != listFileNames.end())
	{
		throw runtime_error("重复添加");
		return;	// 不重复添加了
	}

	// 识别字符集
	auto [charsetCode, content, contentSize] = core.GetEncoding(filename);

	// 如果是智能模式，且没有识别出编码集，则忽略掉，但要提示
	if (core.GetConfig().filterMode == Configuration::FilterMode::SMART &&
		charsetCode == CharsetCode::UNKNOWN)
	{
		throw io_error_ignore();
		return;
	}

	auto charsetName = ToCharsetName(charsetCode);

	try
	{
		auto count = listview.GetItemCount();
		listview.AddItem(count, static_cast<int>(ListViewColumn::INDEX), to_tstring(count + 1).c_str());
		listview.AddItem(count, static_cast<int>(ListViewColumn::FILENAME), filename.c_str());
		listview.AddItem(count, static_cast<int>(ListViewColumn::FILESIZE), FileSizeToTString(GetFileSize(filename)).c_str());

		listview.AddItem(count, static_cast<int>(ListViewColumn::ENCODING), charsetName.c_str());

		auto lineBreak = GetLineBreaks(content, contentSize);
		listview.AddItem(count, static_cast<int>(ListViewColumn::LINE_BREAK), lineBreaksMap[lineBreak].c_str());

		listview.AddItem(count, static_cast<int>(ListViewColumn::TEXT_PIECE), reinterpret_cast<wchar_t *>(content.get()));

		// 成功添加
		listFileNames.insert(filename);

		return;
	}
	catch (runtime_error &err)
	{
		// 如果AddItem之后出错，移除掉错误条目
		listview.DeleteItem(listview.GetItemCount() - 1);
		throw err;
	}
}

std::vector<std::tstring> DialogMain::AddItems(const std::vector<std::tstring> &pathes)
{
	// 后缀
	unordered_set<tstring> filterDotExts;

	switch (core.GetConfig().filterMode)
	{
	case Configuration::FilterMode::NO_FILTER:
		break;
	case Configuration::FilterMode::SMART:	// 智能识别文本
		break;
	case Configuration::FilterMode::ONLY_SOME_EXTANT:
		// 只包括指定后缀
		CheckAndTraversalIncludeRule([&](const tstring &dotExt)
			{
				filterDotExts.insert(dotExt);
			});
		break;
	default:
		assert(0);
	}

	vector<pair<tstring, tstring>> failed;	// 失败的文件
	vector<tstring> ignored; // 忽略的文件

	auto AddItemNoException = [&](const std::tstring &filename)
	{
		try
		{
			AddItem(filename, filterDotExts);
		}
		catch (io_error_ignore)
		{
			ignored.push_back(filename);
		}
		catch (runtime_error &e)
		{
			failed.push_back({ filename, to_tstring(e.what())});
		}
	};

	for (auto &path : pathes)
	{
		// 如果是目录
		if (IsFolder(path))
		{
			// 遍历指定目录
			auto filenames = TraversalAllFileNames(path);

			for (auto &filename : filenames)
			{
				AddItemNoException(filename);
			}
			continue;
		}

		// 如果是文件
		AddItemNoException(path);
	}

	if (!failed.empty())
	{
		tstring info = TEXT("以下文件添加失败：\r\n");
		for (auto &pr : failed)
		{
			info += pr.first + TEXT(" 原因：") + pr.second + TEXT("\r\n");
		}
		MessageBox(info.c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
	}

	if (!ignored.empty())
	{
		tstringstream ss;
		ss << to_tstring(ignored.size()) << TEXT(" 个文件被判定为非文本文件、为空文件、或者没有探测出字符集：\r\n");

		int count = 0;
		for (auto &filename : ignored)
		{
			ss << filename << TEXT("\r\n");
			count++;

			if (count >= 5)
			{
				ss << TEXT("......等");
				break;
			}
		}

		MessageBox(ss.str().c_str(), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
		return ignored;
	}
	return ignored;
}

LRESULT DialogMain::OnBnClickedRadioStretegyNoFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetFilterMode(Configuration::FilterMode::NO_FILTER);
	return 0;
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
	switch (core.GetConfig().filterMode)
	{
	case Configuration::FilterMode::NO_FILTER:
	case Configuration::FilterMode::SMART:	// 智能识别文本
		dialogFilter = { { L"所有文件*.*", L"*.*" } };
		break;
	case Configuration::FilterMode::ONLY_SOME_EXTANT:
	{
		// 只包括指定后缀
		tstring filterExtsStr;	// dialog的过滤器要求;分割
		CheckAndTraversalIncludeRule([&](const tstring &dotExt)
			{
				filterExtsStr += TEXT("*") + dotExt + TEXT(";");
			});

		// dialog过滤器
		dialogFilter.push_back(make_pair(filterExtsStr, filterExtsStr));

		break;
	}
	default:
		assert(0);
	}

	// 打开文件对话框
	TFileDialog dialog(*this, dialogFilter, true);
	if (dialog.Open())
	{
		auto filenames = dialog.GetResult();

		AddItems(filenames);
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
	static tstring dir;	// 可用于赋予TFolderBrowser初始路径

	TFolderBrowser folderBrowser(*this);
	if (folderBrowser.Open(dir))
	{
		AddItems({ dir });
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

			// 取出原换行符
			auto originLineBreak = lineBreaksMap[listview.GetItemText(i, static_cast<int>(ListViewColumn::LINE_BREAK))];

			do
			{
				// 如果原编码和目标编码一样，且不变更换行符
				if (originCode == targetCode)
				{
					// 如果不变更换行符，或者前后换行符一样
					if (core.GetConfig().enableConvertLineBreaks == false || core.GetConfig().lineBreak == originLineBreak)
					{
						// 那么只需要考虑是否原位转换，原位转换的话什么也不做，否则复制过去

						// 如果不是原位置转换，复制过去
						if (core.GetConfig().outputTarget == Configuration::OutputTarget::TO_DIR)
						{
							bool ok = CopyFile(filename.c_str(), outputFileName.c_str(), false);
							if (!ok)
							{
								throw runtime_error("写入失败：" + to_string(outputFileName));
							}
						}
						else
						{
							// 原位转换，什么也不做
							break;
						}

						// 不会到达这里
					}

					// 要变更换行符，且前后换行符不一样
				}

				// 前后编码不一样
				auto filesize = GetFileSize(filename);

				// 暂时不做分块转换 TODO

				{
					// 读二进制
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
					auto [buf, bufLen] = Decode(rawStart, rawSize, originCode);

					// 如果需要转换换行符
					if (core.GetConfig().enableConvertLineBreaks && core.GetConfig().lineBreak != originLineBreak)
					{
						ChangeLineBreaks(buf, bufLen, core.GetConfig().lineBreak);
					}

					// 转到目标编码
					auto [ret, retLen] = Encode(buf, bufLen, targetCode);

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

			} while (0);

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


LRESULT DialogMain::OnEnChangeEditIncludeText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL & /*bHandled*/)try
{
	// 取得字符串
	tstring filterStr;

	BSTR bstr = nullptr;
	CEdit edit(hWndCtl);
	if (edit.GetWindowTextLengthW() != 0)
	{
		bool ok = edit.GetWindowTextW(bstr);
		if (!ok)
			throw runtime_error("出错：内存不足。");
		filterStr = bstr;
		SysReleaseString(bstr);
	}

	// 保存到core
	core.SetFilterRule(filterStr);

	return 0;
}
catch (runtime_error &err)
{
	MessageBox(to_tstring(err.what()).c_str(), TEXT("出错"), MB_OK | MB_ICONERROR);
	return 0;
}

LRESULT DialogMain::OnNMClickSyslink1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & /*bHandled*/)
{
	HINSTANCE r = ShellExecute(NULL, L"open", L"https://github.com/tomwillow/SmartCharsetConverter/releases", NULL, NULL, SW_SHOWNORMAL);

	return 0;
}

LRESULT DialogMain::OnBnClickedCheckConvertReturn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	bool enableLineBreaks = CButton(GetDlgItem(IDC_CHECK_CONVERT_RETURN)).GetCheck();
	core.SetEnableConvertLineBreak(enableLineBreaks);

	CButton(GetDlgItem(IDC_RADIO_CRLF)).EnableWindow(enableLineBreaks);
	CButton(GetDlgItem(IDC_RADIO_LF)).EnableWindow(enableLineBreaks);
	CButton(GetDlgItem(IDC_RADIO_CR)).EnableWindow(enableLineBreaks);

	return 0;
}

LRESULT DialogMain::OnBnClickedRadioCrlf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	core.SetLineBreaks(Configuration::LineBreaks::CRLF);

	return 0;
}

LRESULT DialogMain::OnBnClickedRadioLf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	core.SetLineBreaks(Configuration::LineBreaks::LF);

	return 0;
}

LRESULT DialogMain::OnBnClickedRadioCr(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	core.SetLineBreaks(Configuration::LineBreaks::CR);

	return 0;
}

LRESULT DialogMain::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)try
{
	HDROP hDrop = reinterpret_cast<HDROP>(wParam);

	vector<tstring> ret;
	UINT nFileNum = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); // 拖拽文件个数
	TCHAR strFileName[MAX_PATH];
	for (UINT i = 0; i < nFileNum; i++)
	{
		DragQueryFile(hDrop, i, strFileName, MAX_PATH);//获得拖曳的文件名
		ret.push_back(strFileName);
	}
	DragFinish(hDrop);      //释放hDrop

	// 添加文件
	AddItems(ret);

	return 0;
}
catch (runtime_error &e)
{
	MessageBox(to_tstring(e.what()).c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
	return 0;
}
