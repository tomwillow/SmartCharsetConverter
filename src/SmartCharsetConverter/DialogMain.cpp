#include "DialogMain.h"

// self
#include "Control/TMenu.h"

#include <Common/tstring.h>
#include <Common/FileFunction.h>

#include <cassert>

#include <stdexcept>
#include <sstream>
#include <set>
#include <regex>
#include <filesystem>

#undef min
#undef max

const std::tstring appTitle = TEXT("SmartCharsetConverter v0.9.3 by Tom Willow");

const std::string configFileName = "SmartCharsetConverter.json";

const std::vector<int> innerLanguageIds = {
    IDR_LANGUAGEJSON_ENGLISH,
    IDR_LANGUAGEJSON_SIMPLIFIED_CHINESE,
    IDR_LANGUAGEJSON_SPANISH,
};

using namespace std;

DialogMain::DialogMain(const std::vector<std::string> &filenames) : inputFilenames(filenames) {

    CoreInitOption coreOpt;
    coreOpt.fnUIUpdateItem = [this](int index, std::string filename, std::string fileSizeStr, std::string charsetStr,
                                    std::string lineBreakStr, std::u16string textPiece) {
        PostUIFunc([=]() {
            listview.SetItemText(index, static_cast<int>(ListViewColumn::FILENAME), utf8_to_wstring(filename).c_str());
            listview.SetItemText(index, static_cast<int>(ListViewColumn::FILESIZE),
                                 utf8_to_wstring(fileSizeStr).c_str());
            listview.SetItemText(index, static_cast<int>(ListViewColumn::ENCODING),
                                 utf8_to_wstring(charsetStr).c_str());
            listview.SetItemText(index, static_cast<int>(ListViewColumn::LINE_BREAK),
                                 utf8_to_wstring(lineBreakStr).c_str());
            listview.SetItemText(index, static_cast<int>(ListViewColumn::TEXT_PIECE),
                                 reinterpret_cast<const wchar_t *>(textPiece.c_str()));
        });
    };

    try {
        core = make_unique<Core>(configFileName, coreOpt);

        //
        LanguageServiceOption option;
        option.languageName = core->GetConfig().language;
        option.resourceIds = innerLanguageIds;
        option.resourceType = L"LanguageJson";

        languageService = std::make_unique<LanguageService>(option);
    } catch (const nlohmann::json::exception &err) {
        (err);
        throw;
    } catch (const std::exception &err) {
        (err);
        throw;
    }
}

DialogMain::~DialogMain() {}

void DialogMain::OnClose() {
    if (thRunning) {
        doCancel = true;
        fu.get();
    }
    EndDialog(0);
}

void DialogMain::RefreshInterfaceByCurrentLanguage() noexcept {
    // set controls by language settings
    GetDlgItem(IDC_STATIC_FILE_LISTS).SetWindowTextW(languageService->GetWString(v0_2::StringId::FILE_LISTS).c_str());
    GetDlgItem(IDC_STATIC_SET_FILTER_MODE)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::SET_FILTER_MODE).c_str());
    GetDlgItem(IDC_RADIO_STRETEGY_NO_FILTER)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::NO_FILTER).c_str());
    GetDlgItem(IDC_RADIO_STRETEGY_SMART)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::SMART_FILE_DETECTION).c_str());
    GetDlgItem(IDC_RADIO_STRETEGY_MANUAL)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::USE_FILE_EXTENSION).c_str());
    GetDlgItem(IDC_STATIC_ADD_FILES_OR_FOLDER)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::ADD_FILES_OR_FOLDER).c_str());
    GetDlgItem(IDC_BUTTON_ADD_FILES).SetWindowTextW(languageService->GetWString(v0_2::StringId::ADD_FILES).c_str());
    GetDlgItem(IDC_BUTTON_ADD_DIR).SetWindowTextW(languageService->GetWString(v0_2::StringId::ADD_FOLDER).c_str());
    GetDlgItem(IDC_STATIC_SET_OUTPUT).SetWindowTextW(languageService->GetWString(v0_2::StringId::SET_OUTPUT).c_str());
    GetDlgItem(IDC_RADIO_TO_ORIGIN)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::OUTPUT_TO_ORIGIN).c_str());
    GetDlgItem(IDC_RADIO_TO_DIR).SetWindowTextW(languageService->GetWString(v0_2::StringId::OUTPUT_TO_FOLDER).c_str());
    GetDlgItem(IDC_BUTTON_SET_OUTPUT_DIR)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::SELECT_FOLDER).c_str());
    GetDlgItem(IDC_STATIC_SET_OUTPUT_CHARSET)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::SET_OUTPUT_CHARSET).c_str());
    GetDlgItem(IDC_RADIO_OTHER).SetWindowTextW(languageService->GetWString(v0_2::StringId::OTHERS).c_str());
    GetDlgItem(IDC_CHECK_CONVERT_RETURN)
        .SetWindowTextW(languageService->GetWString(v0_2::StringId::CHANGE_LINE_BREAKS).c_str());
    GetDlgItem(IDC_BUTTON_START).SetWindowTextW(languageService->GetWString(v0_2::StringId::START_CONVERT).c_str());
    GetDlgItem(IDC_BUTTON_CLEAR).SetWindowTextW(languageService->GetWString(v0_2::StringId::CLEAR_LISTS).c_str());

    listview.SetColumnText(static_cast<int>(ListViewColumn::INDEX),
                           languageService->GetWString(v0_2::StringId::INDEX).c_str());
    listview.SetColumnText(static_cast<int>(ListViewColumn::FILENAME),
                           languageService->GetWString(v0_2::StringId::FILENAME).c_str());
    listview.SetColumnText(static_cast<int>(ListViewColumn::FILESIZE),
                           languageService->GetWString(v0_2::StringId::SIZE).c_str());
    listview.SetColumnText(static_cast<int>(ListViewColumn::ENCODING),
                           languageService->GetWString(v0_2::StringId::ENCODING).c_str());
    listview.SetColumnText(static_cast<int>(ListViewColumn::LINE_BREAK),
                           languageService->GetWString(v0_2::StringId::LINE_BREAKS).c_str());
    listview.SetColumnText(static_cast<int>(ListViewColumn::TEXT_PIECE),
                           languageService->GetWString(v0_2::StringId::TEXT_PIECE).c_str());

    rightMenu->SetItemTextById(ID_OPEN_WITH_NOTEPAD, languageService->GetWString(v0_2::StringId::OPEN_WITH_NOTEPAD));
    rightMenu->SetItemTextById(ID_SPECIFY_ORIGIN_CHARSET,
                               languageService->GetWString(v0_2::StringId::SPECIFY_ORIGIN_ENCODING));
    rightMenu->SetItemTextById(ID_REMOVE_ITEM, languageService->GetWString(v0_2::StringId::REMOVE));
}

BOOL DialogMain::OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
    // 设置窗口的大小图标
    // 大图标：按下alt+tab键切换窗口时对应的图标
    // 小图标：就是窗口左上角对应的那个图标
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

    SetWindowText(appTitle.c_str());

    BOOL bHandle = true;

    // 包含/排除指定后缀
    SetFilterMode(core->GetConfig().filterMode);
    GetDlgItem(IDC_EDIT_INCLUDE_TEXT).SetWindowTextW(utf8_to_wstring(core->GetConfig().includeRule).c_str());

    // target
    SetOutputTarget(core->GetConfig().outputTarget);
    GetDlgItem(IDC_EDIT_OUTPUT_DIR).SetWindowTextW(utf8_to_wstring(core->GetConfig().outputDir).c_str());
    static_cast<CEdit>(GetDlgItem(IDC_EDIT_OUTPUT_DIR)).SetReadOnly(true);

    // 要从列表框“其他”中排除的字符集（也就是 能直接通过单选按钮选中的字符集）
    const std::vector<CharsetCode> comboBoxOtherExcludes = {CharsetCode::UTF8, CharsetCode::UTF8BOM,
                                                            CharsetCode::GB18030};
    comboBoxOther.Attach(GetDlgItem(IDC_COMBO_OTHER_CHARSET).m_hWnd);
    for (int icode = static_cast<int>(CharsetCode::UTF8), i = 0;
         icode < static_cast<int>(CharsetCode::CHARSET_CODE_END); ++icode) {
        CharsetCode code = static_cast<CharsetCode>(icode);
        if (std::find(comboBoxOtherExcludes.begin(), comboBoxOtherExcludes.end(), code) !=
            comboBoxOtherExcludes.end()) {
            continue;
        }

        comboBoxOther.AddString(utf8_to_wstring(ToViewCharsetName(code)).c_str());
        comboBoxOther.SetItemData(i, static_cast<int>(code));
        i++;
    }
    comboBoxOther.SetCurSel(0);

    SetOutputCharset(core->GetConfig().outputCharset);

    // enable/disable line breaks
    CButton(GetDlgItem(IDC_CHECK_CONVERT_RETURN)).SetCheck(core->GetConfig().enableConvertLineBreaks);
    OnBnClickedCheckConvertReturn(0, 0, 0, bHandle);
    CButton(GetDlgItem(IDC_RADIO_CRLF + static_cast<int>(core->GetConfig().lineBreak))).SetCheck(true);

    // listview
    listview.SubclassWindow(GetDlgItem(IDC_LISTVIEW)); // 必须用SubclassWindow传入句柄，才能让MSG_MAP生效

    listview.ModifyStyle(0, LVS_REPORT);
    listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    listview.AddColumn(languageService->GetWString(v0_2::StringId::INDEX).c_str(),
                       static_cast<int>(ListViewColumn::INDEX));
    listview.SetColumnWidth(0, 40);

    listview.AddColumn(languageService->GetWString(v0_2::StringId::FILENAME).c_str(),
                       static_cast<int>(ListViewColumn::FILENAME));
    listview.SetColumnWidth(1, 280);

    listview.AddColumn(languageService->GetWString(v0_2::StringId::SIZE).c_str(),
                       static_cast<int>(ListViewColumn::FILESIZE));
    listview.SetColumnWidth(2, 60);

    listview.AddColumn(languageService->GetWString(v0_2::StringId::ENCODING).c_str(),
                       static_cast<int>(ListViewColumn::ENCODING));
    listview.SetColumnWidth(3, 60);

    listview.AddColumn(languageService->GetWString(v0_2::StringId::LINE_BREAKS).c_str(),
                       static_cast<int>(ListViewColumn::LINE_BREAK));
    listview.SetColumnWidth(4, 80);

    listview.AddColumn(languageService->GetWString(v0_2::StringId::TEXT_PIECE).c_str(),
                       static_cast<int>(ListViewColumn::TEXT_PIECE));
    listview.SetColumnWidth(5, 200);

    // 右键菜单
    rightMenu = std::make_unique<TPopupMenu>(IDR_MENU_RIGHT);
    TMenu &specifyOriginCharsetMenu = rightMenu->SetItemToBeContainer(ID_SPECIFY_ORIGIN_CHARSET);
    for (auto commandId = SPECIFY_ORIGIN_CHARSET_ID_START; commandId < SPECIFY_ORIGIN_CHARSET_ID_END; ++commandId) {
        CharsetCode code = CommandIdToCharsetCode(commandId);
        specifyOriginCharsetMenu.AppendItem(commandId, utf8_to_wstring(ToViewCharsetName(code)));
    }

    selectLanguageMenu = std::make_unique<TPopupMenu>(IDR_MENU_SELECT_LANGUAGES);
    TMenu &languageSubMenu = selectLanguageMenu->SetItemToBeContainer(ID_LANGUAGE);
    for (auto commandId = SELECT_LANUAGE_ID_START; commandId < GetSelectLanguageIdEnd(); ++commandId) {
        std::string languageName = CommandIdToLanguageName(commandId);
        languageSubMenu.AppendItem(commandId, utf8_to_wstring(languageName));
    }

    // 启用拖放
    ::DragAcceptFiles(listview, true);

    setlocale(LC_CTYPE, "");

    RefreshInterfaceByCurrentLanguage();

    CenterWindow();

    AddItemsAsync(inputFilenames);

    return 0;
}

void DialogMain::SetFilterMode(Configuration::FilterMode mode) {
    core->SetFilterMode(mode);

    CButton(GetDlgItem(IDC_RADIO_STRETEGY_NO_FILTER)).SetCheck(false);
    CButton(GetDlgItem(IDC_RADIO_STRETEGY_SMART)).SetCheck(false);
    CButton(GetDlgItem(IDC_RADIO_STRETEGY_MANUAL)).SetCheck(false);
    switch (mode) {
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

void DialogMain::SetOutputTarget(Configuration::OutputTarget outputTarget) {
    core->SetOutputTarget(outputTarget);
    bool isToOrigin = (outputTarget == Configuration::OutputTarget::ORIGIN);

    CButton(GetDlgItem(IDC_RADIO_TO_ORIGIN)).SetCheck(isToOrigin);
    CButton(GetDlgItem(IDC_RADIO_TO_DIR)).SetCheck(!isToOrigin);

    GetDlgItem(IDC_EDIT_OUTPUT_DIR).EnableWindow(!isToOrigin);
    GetDlgItem(IDC_BUTTON_SET_OUTPUT_DIR).EnableWindow(!isToOrigin);
}

void DialogMain::SetOutputCharset(CharsetCode charset) {
    assert(charset != CharsetCode::UNKNOWN);
    assert(charset != CharsetCode::EMPTY);
    assert(charset != CharsetCode::NOT_SUPPORTED);
    assert(charset != CharsetCode::CHARSET_CODE_END);

    core->SetOutputCharset(charset);
    bool isNormalCharset = Configuration::IsNormalCharset(charset);

    CButton(GetDlgItem(IDC_RADIO_UTF8)).SetCheck(charset == CharsetCode::UTF8);
    CButton(GetDlgItem(IDC_RADIO_UTF8BOM)).SetCheck(charset == CharsetCode::UTF8BOM);
    CButton(GetDlgItem(IDC_RADIO_GB18030)).SetCheck(charset == CharsetCode::GB18030);
    CButton(GetDlgItem(IDC_RADIO_OTHER)).SetCheck(Configuration::IsNormalCharset(charset) == false);

    GetDlgItem(IDC_COMBO_OTHER_CHARSET).EnableWindow(!isNormalCharset);

    if (!isNormalCharset) {
        for (int i = 0; i < comboBoxOther.GetCount(); ++i) {
            if (comboBoxOther.GetItemData(i) == static_cast<int>(charset)) {
                comboBoxOther.SetCurSel(i);
            }
        }
    }
}

std::vector<std::string> DialogMain::AddItems(const std::vector<std::string> &pathes) noexcept {
    // 后缀
    unordered_set<string> filterDotExts;

    switch (core->GetConfig().filterMode) {
    case Configuration::FilterMode::NO_FILTER:
        break;
    case Configuration::FilterMode::SMART: // 智能识别文本
        break;
    case Configuration::FilterMode::ONLY_SOME_EXTANT:
        // 只包括指定后缀
        try {
            CheckAndTraversalIncludeRule([&](const string &dotExt) {
                filterDotExts.insert(dotExt);
            });
        } catch (const std::runtime_error &err) {
            MessageBoxW(utf8_to_wstring(err.what()).c_str(),
                        languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(), MB_OK | MB_ICONERROR);
            return {};
        }
        break;
    default:
        assert(0);
    }

    vector<pair<string, string>> failed; // 失败的文件
    vector<string> ignored;              // 忽略的文件

    auto AddItemNoException = [&](const std::string &filename) {
        try {
            Core::AddItemResult ret = core->AddItem(filename, filterDotExts);
            if (ret.isIgnore) {
                return;
            }
            PostUIFunc([filename, ret, this]() {
                AppendListViewItem(filename, ret.filesize, ret.srcCharset, ret.srcLineBreak, ret.strPiece);
            });
        } catch (io_error_ignore) {
            ignored.push_back(filename);
        } catch (const MyRuntimeError &err) {
            failed.push_back({filename, err.ToLocalString(languageService.get())});
        } catch (const runtime_error &err) {
            failed.push_back({filename, err.what()});
        }
    };

    for (auto &path : pathes) {
        // 如果是目录
        if (std::filesystem::is_directory(path)) {
            // 遍历指定目录
            auto filenames = TraversalAllFileNames(path);

            for (auto &filename : filenames) {
                if (doCancel) {
                    goto AddItemsAbort;
                }
                AddItemNoException(filename);
            }
            continue;
        }

        // 如果是文件
        if (doCancel) {
            goto AddItemsAbort;
        }
        AddItemNoException(path);
    }

AddItemsAbort:

    if (!failed.empty()) {
        string info = languageService->GetUtf8String(v0_2::StringId::FAILED_ADD_BELOW) + u8"\r\n";
        for (auto &pr : failed) {
            info += pr.first + u8" " + languageService->GetUtf8String(v0_2::StringId::REASON) + u8" " + pr.second +
                    u8"\r\n ";
        }

        MyMessage *msg = new MyMessage([this, info]() {
            MessageBox(utf8_to_wstring(info).c_str(), languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(),
                       MB_OK | MB_ICONERROR);
        });
        PostMessage(WM_MY_MESSAGE, 0, reinterpret_cast<LPARAM>(msg));
    }

    if (!ignored.empty()) {
        string s;

        std::string dest =
            MyPrintf(languageService->GetUtf8String(v0_2::StringId::NON_TEXT_OR_NO_DETECTED), 32LL, ignored.size());

        s += dest + u8"\r\n";

        int count = 0;
        for (auto &filename : ignored) {
            s += filename + u8"\r\n";
            count++;

            if (count >= 5) {
                s += languageService->GetUtf8String(v0_2::StringId::AND_SO_ON);
                break;
            }
        }

        s += u8"\r\n\r\n";
        s += languageService->GetUtf8String(v0_2::StringId::TIPS_USE_NO_FILTER);

        PostUIFunc([this, s]() {
            MessageBox(utf8_to_wstring(s).c_str(), languageService->GetWString(v0_2::StringId::PROMPT).c_str(),
                       MB_OK | MB_ICONINFORMATION);
        });
        return ignored;
    }
    return ignored;
}

void DialogMain::AddItemsAsync(const std::vector<std::string> &filenames) noexcept {
    auto restore = SetBusyState();

    doCancel = false;
    assert(thRunning == false);
    thRunning = true;
    fu = std::async(std::launch::async, [this, restore, filenames]() {
        // 使用RTTI的手法记下恢复事件
        unique_ptr<void, function<void(void *)>> deferRestore(reinterpret_cast<void *>(1), [this, restore](void *) {
            PostUIFunc([this, restore]() {
                RestoreReadyState(restore);

                thRunning = false;
            });
        });

        try {
            AddItems(filenames);
        } catch (const runtime_error &err) {
            PostUIFunc([this, err]() {
                MessageBoxW(utf8_to_wstring(err.what()).c_str(),
                            languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(), MB_OK | MB_ICONERROR);
            });
        }
    });
}

void DialogMain::StartConvert(const std::vector<std::pair<int, bool>> &restore, const std::vector<Item> &items) try {
    // 使用RTTI的手法记下恢复事件
    unique_ptr<void, function<void(void *)>> deferRestore(reinterpret_cast<void *>(1), [this, restore](void *) {
        PostUIFunc([this, restore]() {
            RestoreReadyState(restore);

#ifndef NDEBUG
            cout << "Exit: StartConvert thread" << endl;
#endif
            thRunning = false;
        });
    });

    // 如果没有内容
    if (listview.GetItemCount() == 0) {
        throw runtime_error(languageService->GetUtf8String(v0_2::StringId::NO_FILE_TO_CONVERT));
    }

    // 检查输出目录
    if (core->GetConfig().outputTarget != Configuration::OutputTarget::ORIGIN) {
        if (core->GetConfig().outputDir.empty()) {
            throw runtime_error(languageService->GetUtf8String(v0_2::StringId::INVALID_OUTPUT_DIR));
        }
    }

    vector<pair<string, string>> failed; // 失败文件/失败原因
    vector<string> succeed;              // 成功的文件

    // 目标编码
    auto targetCode = core->GetConfig().outputCharset;

    // 逐个转换
    auto count = items.size();
    for (int i = 0; i < count; ++i) {
        if (doCancel) {
            break;
        }

        auto &filename = items[i].filename;
        auto originCode = items[i].originCode;
        auto originLineBreak = items[i].originLineBreak;

        // 更新UI
        PostUIFunc([=]() {
            listview.SetItemText(i, static_cast<int>(ListViewColumn::INDEX), (TEXT("->") + to_tstring(i + 1)).c_str());

            // listview滚动
            listview.SelectItem(i);
        });

        auto convertResult = core->Convert(filename, originCode, originLineBreak);
        if (convertResult.errInfo.has_value()) {
            failed.push_back({filename, convertResult.errInfo.value()});
        } else {
            succeed.push_back(filename);
        }

        // 更新UI
        PostUIFunc([=]() {
            listview.SetItemText(i, static_cast<int>(ListViewColumn::INDEX), to_tstring(i + 1).c_str());
            if (convertResult.errInfo.has_value()) {
                return;
            }
            listview.SetItemText(i, static_cast<int>(ListViewColumn::FILENAME),
                                 utf8_to_wstring(convertResult.outputFileName).c_str());
            listview.SetItemText(i, static_cast<int>(ListViewColumn::FILESIZE),
                                 utf8_to_wstring(FileSizeToHumanString(convertResult.outputFileSize)).c_str());
            listview.SetItemText(i, static_cast<int>(ListViewColumn::ENCODING),
                                 utf8_to_wstring(ToViewCharsetName(targetCode)).c_str());
            listview.SetItemText(i, static_cast<int>(ListViewColumn::LINE_BREAK),
                                 utf8_to_wstring(LineBreaksToViewName(convertResult.targetLineBreaks)).c_str());
        });
    }

    // 已经完成处理

    // 如果有失败的
    if (failed.empty() == false) {
        string s;

        std::string dest =
            MyPrintf(languageService->GetUtf8String(v0_2::StringId::SUCCEED_SOME_FILES), 32LL, succeed.size());

        s += dest + u8"\r\n\r\n";
        s += languageService->GetUtf8String(v0_2::StringId::FAILED_CONVERT_BELOW) + u8"\r\n";
        for (auto &pr : failed) {
            s += pr.first + u8" " + languageService->GetUtf8String(v0_2::StringId::REASON) + pr.second + u8"\r\n";
        }
        if (doCancel) {
            s += u8"\r\n\r\n" + languageService->GetUtf8String(v0_2::StringId::NO_DEAL_DUE_TO_CANCEL);
        }

        PostUIFunc([this, s]() {
            MessageBox(utf8_to_wstring(s).c_str(), languageService->GetWString(v0_2::StringId::CONVERT_RESULT).c_str(),
                       MB_OK | MB_ICONERROR);
        });
    } else {
        // 全部成功之后
        stringstream ss;
        std::string dest =
            MyPrintf(languageService->GetUtf8String(v0_2::StringId::SUCCEED_SOME_FILES), 32LL, succeed.size());
        ss << dest << u8"\r\n\r\n";

        if (targetCode == CharsetCode::GB18030) {
            ss << u8"\r\n\r\n" << languageService->GetUtf8String(v0_2::StringId::NOTICE_SHOW_AS_UTF8);
        }
        if (doCancel) {
            ss << u8"\r\n\r\n" << languageService->GetUtf8String(v0_2::StringId::NO_DEAL_DUE_TO_CANCEL);
        }

        string s = ss.str();
        PostUIFunc([this, s]() {
            MessageBox(utf8_to_wstring(s).c_str(), languageService->GetWString(v0_2::StringId::PROMPT).c_str(),
                       MB_OK | MB_ICONINFORMATION);
        });
    }

    return;
} catch (const runtime_error &err) {
    PostUIFunc([this, err]() {
        MessageBox(utf8_to_wstring(err.what()).c_str(),
                   languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(), MB_OK | MB_ICONERROR);
    });
    return;
}

LRESULT DialogMain::OnBnClickedRadioStretegyNoFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                                     BOOL & /*bHandled*/) {
    SetFilterMode(Configuration::FilterMode::NO_FILTER);
    return 0;
}

LRESULT DialogMain::OnBnClickedRadioStretegySmart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                                  BOOL & /*bHandled*/) {
    SetFilterMode(Configuration::FilterMode::SMART);
    return 0;
}

LRESULT DialogMain::OnBnClickedRadioStretegyManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                                   BOOL & /*bHandled*/) {
    SetFilterMode(Configuration::FilterMode::ONLY_SOME_EXTANT);
    return 0;
}

LRESULT DialogMain::OnBnClickedRadioToOrigin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                             BOOL & /*bHandled*/) {
    SetOutputTarget(Configuration::OutputTarget::ORIGIN);
    return 0;
}

LRESULT DialogMain::OnBnClickedRadioToDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    SetOutputTarget(Configuration::OutputTarget::TO_DIR);
    return 0;
}

LRESULT DialogMain::OnBnClickedRadioUtf8(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    SetOutputCharset(CharsetCode::UTF8);
    return 0;
}

LRESULT DialogMain::OnBnClickedRadioUtf8bom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    SetOutputCharset(CharsetCode::UTF8BOM);
    return 0;
}

LRESULT DialogMain::OnBnClickedRadioGb18030(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    SetOutputCharset(CharsetCode::GB18030);
    return 0;
}

LRESULT DialogMain::OnBnClickedRadioOther(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    SetOutputCharset(static_cast<CharsetCode>(comboBoxOther.GetItemData(comboBoxOther.GetCurSel())));

    return 0;
}

void DialogMain::CheckAndTraversalIncludeRule(std::function<void(const std::string &dotExt)> fn) {
    // 后缀字符串
    auto &extsStr = core->GetConfig().includeRule;

    // 切分
    auto exts = Split(extsStr, u8" ,|");

    string filterExampleStr = languageService->GetUtf8String(v0_2::StringId::SUPPORT_FORMAT_BELOW) +
                              u8"\r\n *.h *.hpp *.c *.cpp *.txt\r\n h hpp c cpp txt\r\n h|hpp|c|cpp\r\n" +
                              languageService->GetUtf8String(v0_2::StringId::SEPERATOR_DESCRIPTION);

    // 如果为空
    if (exts.empty()) {
        throw runtime_error(languageService->GetUtf8String(v0_2::StringId::NO_SPECIFY_FILTER_EXTEND) + u8"\r\n\r\n" +
                            filterExampleStr);
    }

    // 逐个检查
    for (auto s : exts) {
        string extStr(s);
        string pattern = u8R"((\*\.|\.|)(\w+))"; // 匹配*.xxx/.xxx/xxx的正则
        regex r(pattern);
        smatch results;
        if (regex_match(extStr, results, r) == false) {
            throw runtime_error(languageService->GetUtf8String(v0_2::StringId::INVALID_EXTEND_FILTER) + extStr +
                                u8"\r\n\r\n" + filterExampleStr);
        }

        fn(tolower(u8"." + results.str(2)));
    }
}

LRESULT DialogMain::OnBnClickedButtonAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                              BOOL & /*bHandled*/) try {
    vector<pair<tstring, tstring>> dialogFilter;
    switch (core->GetConfig().filterMode) {
    case Configuration::FilterMode::NO_FILTER:
    case Configuration::FilterMode::SMART: // 智能识别文本
        dialogFilter = {{languageService->GetWString(v0_2::StringId::ALL_FILES) + L"*.*", L"*.*"}};
        break;
    case Configuration::FilterMode::ONLY_SOME_EXTANT: {
        // 只包括指定后缀
        tstring filterExtsStr; // dialog的过滤器要求;分割
        CheckAndTraversalIncludeRule([&](const string &dotExt) {
            filterExtsStr += utf8_to_wstring("*" + dotExt + ";");
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
    if (dialog.Open()) {
        auto filenames = to_utf8(dialog.GetResult());

        AddItemsAsync(filenames);
    }
    return 0;
} catch (runtime_error &err) {
    MessageBox(utf8_to_wstring(err.what()).c_str(), languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(),
               MB_OK | MB_ICONERROR);
    return 0;
}

LRESULT DialogMain::OnBnClickedButtonAddDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                            BOOL & /*bHandled*/) try {
    static wstring dir; // 可用于赋予TFolderBrowser初始路径

    TFolderBrowser folderBrowser(*this);
    if (folderBrowser.Open(dir)) {
        AddItemsAsync(to_utf8(std::vector<std::wstring>{dir}));
    }

    return 0;
} catch (runtime_error &err) {
    MessageBox(utf8_to_wstring(err.what()).c_str(), languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(),
               MB_OK | MB_ICONERROR);
    return 0;
}

LRESULT DialogMain::OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                           BOOL &bHandle /*bHandled*/) {
    if (thRunning) {
        doCancel = true;
        fu.get();
        return 0;
    }

    auto restore = SetBusyState();

    vector<Item> items;
    for (int i = 0; i < listview.GetItemCount(); ++i) {
        auto filename = to_utf8(listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME)));
        auto originCode = ToCharsetCode(to_utf8(listview.GetItemText(i, static_cast<int>(ListViewColumn::ENCODING))));
        auto originLineBreak =
            ViewNameToLineBreaks(to_utf8(listview.GetItemText(i, static_cast<int>(ListViewColumn::LINE_BREAK))));
        items.push_back({filename, originCode, originLineBreak});
    }

    cout << "OnBnClickedButtonStart set cancel false" << endl;
    doCancel = false;
    cout << "OnBnClickedButtonStart async StartConvert" << endl;
    thRunning = true;
    fu = std::async(std::launch::async, &DialogMain::StartConvert, this, restore, items);
    return 0;
}

LRESULT DialogMain::OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    listview.DeleteAllItems();
    core->Clear();
    return 0;
}

LRESULT DialogMain::OnBnClickedButtonSettings(WORD, WORD, HWND, BOOL &) {
    // 弹出右键菜单
    selectLanguageMenu->Popup(m_hWnd);

    return 0;
}

LRESULT DialogMain::OnBnClickedButtonSetOutputDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                                  BOOL & /*bHandled*/) {
    tstring dir = utf8_to_wstring(core->GetConfig().outputDir);

    TFolderBrowser folderBrowser(*this);
    if (folderBrowser.Open(dir)) {
        core->SetOutputDir(to_utf8(dir));
        GetDlgItem(IDC_EDIT_OUTPUT_DIR).SetWindowTextW(dir.c_str());
    }

    return 0;
}

LRESULT DialogMain::OnCbnSelchangeComboOtherCharset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                                    BOOL & /*bHandled*/) {
    SetOutputCharset(static_cast<CharsetCode>(comboBoxOther.GetItemData(comboBoxOther.GetCurSel())));

    return 0;
}

LRESULT DialogMain::OnNMRclickListview(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & /*bHandled*/) {
    auto selectedItems = listview.GetSelectedItems();
    if (selectedItems.empty()) {
        return 0;
    }

    // 弹出右键菜单
    rightMenu->Popup(m_hWnd);

    return 0;
}

LRESULT DialogMain::OnOpenWithNotepad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    auto selectedItems = listview.GetSelectedItems();
    for (auto i : selectedItems) {
        auto filename = listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME));

        wstring cmd = L"notepad " + filename;

        WinExec(to_string(cmd).c_str(), SW_SHOWNORMAL);
    }

    return 0;
}

LRESULT DialogMain::OnRemoveItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    auto selectedItems = listview.GetSelectedItems();
    // 选中的序号，倒序遍历
    for (auto itor = selectedItems.rbegin(); itor != selectedItems.rend(); ++itor) {
        int i = *itor;
        auto filename = to_utf8(listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME)));
        listview.DeleteItem(i);
        core->RemoveItem(filename);
    }

    // 剩下的重新编号
    for (int i = selectedItems.front(); i < listview.GetItemCount(); ++i) {
        listview.SetItemText(i, static_cast<int>(ListViewColumn::INDEX), to_tstring(i + 1).c_str());
    }

    return 0;
}

LRESULT DialogMain::OnSpecifyOriginCharset(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    CharsetCode code = CommandIdToCharsetCode(wID);

    auto selectedItems = listview.GetSelectedItems();

    vector<pair<string, string>> failed; // 失败文件/失败原因
    for (auto itor = selectedItems.begin(); itor != selectedItems.end(); ++itor) {
        int index = *itor;
        auto filename = to_utf8(listview.GetItemText(index, static_cast<int>(ListViewColumn::FILENAME)));
        try {
            core->SpecifyItemCharset(index, filename, code);
        } catch (const MyRuntimeError &err) {
            failed.push_back({filename, err.ToLocalString(languageService.get())});
        } catch (const std::runtime_error &err) {
            failed.push_back({filename, err.what()});
        }
    }

    if (!failed.empty()) {
        string info = languageService->GetUtf8String(v0_2::StringId::FAILED_TO_SET_CHARSET_MANUALLY) + u8"\r\n";
        for (auto &pr : failed) {
            info += pr.first + u8" " + languageService->GetUtf8String(v0_2::StringId::REASON) + pr.second + u8"\r\n";
        }

        MyMessage *msg = new MyMessage([this, info]() {
            MessageBox(utf8_to_wstring(info).c_str(), languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(),
                       MB_OK | MB_ICONERROR);
        });
        PostMessage(WM_MY_MESSAGE, 0, reinterpret_cast<LPARAM>(msg));
    }
    return 0;
}

LRESULT DialogMain::OnSelectLanguage(WORD, WORD wID, HWND, BOOL &) {
    std::string languageName = CommandIdToLanguageName(wID);

    try {
        languageService->SetCurrentLanguage(languageName);
    } catch (const std::runtime_error &err) {
        MessageBox(utf8_to_wstring(err.what()).c_str(),
                   languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(), MB_OK | MB_ICONERROR);
        return 0;
    }

    core->SetLanguage(languageName);

    RefreshInterfaceByCurrentLanguage();

    return 0;
}

LRESULT DialogMain::OnEnChangeEditIncludeText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl,
                                              BOOL & /*bHandled*/) try {
    // 取得字符串
    tstring filterStr;

    BSTR bstr = nullptr;
    CEdit edit(hWndCtl);
    if (edit.GetWindowTextLengthW() != 0) {
        bool ok = edit.GetWindowTextW(bstr);
        if (!ok)
            throw runtime_error(languageService->GetUtf8String(v0_2::StringId::NO_MEMORY));
        filterStr = bstr;
        SysFreeString(bstr);
    }

    // 保存到core
    core->SetFilterRule(to_utf8(filterStr));

    return 0;
} catch (runtime_error &err) {
    MessageBox(utf8_to_wstring(err.what()).c_str(), languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(),
               MB_OK | MB_ICONERROR);
    return 0;
}

LRESULT DialogMain::OnNMClickSyslink1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & /*bHandled*/) {
    HINSTANCE r = ShellExecute(NULL, L"open", L"https://github.com/tomwillow/SmartCharsetConverter/releases", NULL,
                               NULL, SW_SHOWNORMAL);

    return 0;
}

LRESULT DialogMain::OnBnClickedCheckConvertReturn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                                                  BOOL & /*bHandled*/) {
    bool enableLineBreaks = CButton(GetDlgItem(IDC_CHECK_CONVERT_RETURN)).GetCheck();
    core->SetEnableConvertLineBreak(enableLineBreaks);

    CButton(GetDlgItem(IDC_RADIO_CRLF)).EnableWindow(enableLineBreaks);
    CButton(GetDlgItem(IDC_RADIO_LF)).EnableWindow(enableLineBreaks);
    CButton(GetDlgItem(IDC_RADIO_CR)).EnableWindow(enableLineBreaks);

    return 0;
}

LRESULT DialogMain::OnBnClickedRadioCrlf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    core->SetLineBreaks(LineBreaks::CRLF);

    return 0;
}

LRESULT DialogMain::OnBnClickedRadioLf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    core->SetLineBreaks(LineBreaks::LF);

    return 0;
}

LRESULT DialogMain::OnBnClickedRadioCr(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    core->SetLineBreaks(LineBreaks::CR);

    return 0;
}

LRESULT DialogMain::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) try {
    HDROP hDrop = reinterpret_cast<HDROP>(wParam);

    vector<string> filenames;
    UINT nFileNum = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0); // 拖拽文件个数
    TCHAR strFileName[MAX_PATH];
    for (UINT i = 0; i < nFileNum; i++) {
        DragQueryFileW(hDrop, i, strFileName, MAX_PATH); // 获得拖曳的文件名
        filenames.push_back(to_utf8(strFileName));
    }
    DragFinish(hDrop); // 释放hDrop

    AddItemsAsync(filenames);

    return 0;
} catch (const std::runtime_error &err) {
    MessageBox(utf8_to_wstring(err.what()).c_str(), languageService->GetWString(v0_2::StringId::MSGBOX_ERROR).c_str(),
               MB_OK | MB_ICONERROR);
    return 0;
}

void DialogMain::PostUIFunc(std::function<void()> fn) {
    MyMessage *msg = new MyMessage(fn);
    PostMessage(WM_MY_MESSAGE, 0, reinterpret_cast<LPARAM>(msg));
}

LRESULT DialogMain::OnUser(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
    // cout << "OnUser Begin " << std::hex << lParam << endl;
    unique_ptr<MyMessage> msg(reinterpret_cast<MyMessage *>(lParam));
    msg->fn();
    // cout << "OnUser End " << std::hex << lParam << endl;
    return 0;
}

std::vector<pair<int, bool>> DialogMain::SetBusyState() noexcept {
    // 遍历控件，如果是启用状态，那么设置为disable，并在restore中记下，留待日后恢复
    vector<pair<int, bool>> restore;
    for (auto id = IDC_RADIO_STRETEGY_SMART; id <= IDC_RADIO_CR; ++id) {
        if (::GetDlgItem(m_hWnd, id) != NULL) {
            auto wnd = GetDlgItem(id);
            if (wnd.IsWindowEnabled()) {
                restore.push_back({id, true});
                wnd.EnableWindow(false);
            }
        }
    }

    // 开始按钮text变更为“取消”，并额外enable，用于让用户按“取消”
    GetDlgItem(IDC_BUTTON_START).SetWindowTextW(languageService->GetWString(v0_2::StringId::CANCEL).c_str());
    GetDlgItem(IDC_BUTTON_START).EnableWindow(true);
    return restore;
}

void DialogMain::RestoreReadyState(const std::vector<std::pair<int, bool>> &restore) noexcept {
    for (auto &pr : restore) {
        auto wnd = GetDlgItem(pr.first);
        wnd.EnableWindow(pr.second);
    }

    GetDlgItem(IDC_BUTTON_START).SetWindowTextW(languageService->GetWString(v0_2::StringId::START_CONVERT).c_str());
}

void DialogMain::AppendListViewItem(std::string filename, uint64_t fileSize, CharsetCode charset, LineBreaks lineBreak,
                                    std::u16string textPiece) noexcept {
    auto count = listview.GetItemCount();
    listview.AddItem(count, static_cast<int>(ListViewColumn::INDEX), to_tstring(count + 1).c_str());
    listview.AddItem(count, static_cast<int>(ListViewColumn::FILENAME), utf8_to_wstring(filename).c_str());
    listview.AddItem(count, static_cast<int>(ListViewColumn::FILESIZE),
                     utf8_to_wstring(FileSizeToHumanString(fileSize)).c_str());
    listview.AddItem(count, static_cast<int>(ListViewColumn::ENCODING),
                     utf8_to_wstring(ToViewCharsetName(charset)).c_str());
    listview.AddItem(count, static_cast<int>(ListViewColumn::LINE_BREAK),
                     utf8_to_wstring(LineBreaksToViewName(lineBreak)).c_str());
    listview.AddItem(count, static_cast<int>(ListViewColumn::TEXT_PIECE),
                     reinterpret_cast<const wchar_t *>(textPiece.c_str()));

    // listview滚动到最下面
    listview.SelectItem(count);
}
