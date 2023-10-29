#pragma once

#include "Core.h"
#include "ThreadPool/ThreadPool.h"
#include "Control/TMenu.h"

#include "resource.h"

#include <tstring.h>
#include <TListView.h>

#include <atlbase.h> // 基本的ATL类
#include <atlwin.h>  // ATL窗口类
#include <atlapp.h>  // WTL 主框架窗口类
#include <atlctrls.h>
#include <atlcrack.h> // WTL 增强的消息宏

#include <vector>
#include <chrono>
#include <string>
#include <unordered_set>
#include <thread>
#include <atomic>
#include <future>
#include <iostream>

const unsigned int WM_MY_MESSAGE = WM_USER + 1;
struct MyMessage {
    std::function<void()> fn;
    MyMessage(std::function<void()> fn) : fn(fn) {
        // std::cout << "MyMessage ctor: " << this << std::endl;
    }
};

/*
    为了动态地在listview的右键菜单"指定原编码"项目里面添加字符集菜单选项，需要为每个字符集指定一个id。
    但手动指定太麻烦，根据观察，菜单项目的起始编号为40000，所以这里选定了一个30000为起始编号，目的是不和其他id重合。
    然后这个30000+字符集的index则得到菜单项的id。
*/
const int SPECIFY_ORIGIN_CHARSET_ID_CONST = 30000;
const int SPECIFY_ORIGIN_CHARSET_ID_START = SPECIFY_ORIGIN_CHARSET_ID_CONST + static_cast<int>(CharsetCode::UTF8);
const int SPECIFY_ORIGIN_CHARSET_ID_END =
    SPECIFY_ORIGIN_CHARSET_ID_CONST + static_cast<int>(CharsetCode::CHARSET_CODE_END);
inline int CharsetCodeToCommandId(CharsetCode code) noexcept {
    return SPECIFY_ORIGIN_CHARSET_ID_CONST + static_cast<int>(code);
}

inline CharsetCode CommandIdToCharsetCode(int id) noexcept {
    return static_cast<CharsetCode>(id - SPECIFY_ORIGIN_CHARSET_ID_CONST);
}

class DialogMain : public CDialogImpl<DialogMain> {
public:
    enum { IDD = IDD_DIALOG_MAIN };

    /*
     * 如果需要初始时就添加文件/文件夹，则传入filenames参数
     */
    DialogMain(const std::vector<std::tstring> &filenames = {});

    ~DialogMain();

private:
    const std::string caption;

    std::vector<std::tstring> inputFilenames;

    std::unique_ptr<Core> core;

    CComboBox comboBoxOther;
    TListView listview;
    std::unique_ptr<TPopupMenu> rightMenu;

    enum class ListViewColumn { INDEX = 0, FILENAME, FILESIZE, ENCODING, LINE_BREAK, TEXT_PIECE };

    std::future<void> fu;
    std::atomic<bool> thRunning;
    std::atomic<bool> doCancel;

    ThreadPool thPool;

    BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);

    void SetFilterMode(Configuration::FilterMode mode);

    void SetOutputTarget(Configuration::OutputTarget outputTarget);

    void SetOutputCharset(CharsetCode charset);

    /*
     * 加入多个文件/文件夹到列表。
     * 如果有添加失败的文件，会在事件队列中Post一个弹窗事件
     * 线程安全。
     */
    std::vector<std::tstring> AddItems(const std::vector<std::tstring> &filenames) noexcept;

    /*
     * 加入多个文件/文件夹到列表。
     * 如果有添加失败的文件，会在事件队列中Post一个弹窗事件
     */
    void AddItemsAsync(const std::vector<std::tstring> &filenames) noexcept;

    struct Item {
        std::tstring filename;
        CharsetCode originCode;
        Configuration::LineBreaks originLineBreak;
    };

    void StartConvert(const std::vector<std::pair<int, bool>> &restore, const std::vector<Item> &items);

    void OnClose();

    BEGIN_MSG_MAP(DialogMain)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_CLOSE(OnClose)

    COMMAND_HANDLER(IDC_RADIO_STRETEGY_SMART, BN_CLICKED, OnBnClickedRadioStretegySmart)
    COMMAND_HANDLER(IDC_RADIO_STRETEGY_MANUAL, BN_CLICKED, OnBnClickedRadioStretegyManual)
    COMMAND_HANDLER(IDC_RADIO_TO_ORIGIN, BN_CLICKED, OnBnClickedRadioToOrigin)
    COMMAND_HANDLER(IDC_RADIO_TO_DIR, BN_CLICKED, OnBnClickedRadioToDir)
    COMMAND_HANDLER(IDC_RADIO_UTF8, BN_CLICKED, OnBnClickedRadioUtf8)
    COMMAND_HANDLER(IDC_RADIO_UTF8BOM, BN_CLICKED, OnBnClickedRadioUtf8bom)
    COMMAND_HANDLER(IDC_RADIO_GB18030, BN_CLICKED, OnBnClickedRadioGb18030)
    COMMAND_HANDLER(IDC_RADIO_OTHER, BN_CLICKED, OnBnClickedRadioOther)
    COMMAND_HANDLER(IDC_BUTTON_ADD_FILES, BN_CLICKED, OnBnClickedButtonAddFiles)
    COMMAND_HANDLER(IDC_BUTTON_ADD_DIR, BN_CLICKED, OnBnClickedButtonAddDir)
    COMMAND_HANDLER(IDC_BUTTON_START, BN_CLICKED, OnBnClickedButtonStart)
    COMMAND_HANDLER(IDC_BUTTON_CLEAR, BN_CLICKED, OnBnClickedButtonClear)
    COMMAND_HANDLER(IDC_BUTTON_SET_OUTPUT_DIR, BN_CLICKED, OnBnClickedButtonSetOutputDir)
    COMMAND_HANDLER(IDC_COMBO_OTHER_CHARSET, CBN_SELCHANGE, OnCbnSelchangeComboOtherCharset)

    NOTIFY_HANDLER(IDC_LISTVIEW, NM_RCLICK, OnNMRclickListview)
    COMMAND_ID_HANDLER(ID_OPEN_WITH_NOTEPAD, OnOpenWithNotepad)
    COMMAND_ID_HANDLER(ID_REMOVE_ITEM, OnRemoveItem)
    COMMAND_RANGE_HANDLER(SPECIFY_ORIGIN_CHARSET_ID_START, SPECIFY_ORIGIN_CHARSET_ID_END, OnSpecifyOriginCharset)

    COMMAND_HANDLER(IDC_EDIT_INCLUDE_TEXT, EN_CHANGE, OnEnChangeEditIncludeText)
    COMMAND_HANDLER(IDC_RADIO_STRETEGY_NO_FILTER, BN_CLICKED, OnBnClickedRadioStretegyNoFilter)
    NOTIFY_HANDLER(IDC_SYSLINK1, NM_CLICK, OnNMClickSyslink1)
    COMMAND_HANDLER(IDC_CHECK_CONVERT_RETURN, BN_CLICKED, OnBnClickedCheckConvertReturn)
    COMMAND_HANDLER(IDC_RADIO_CRLF, BN_CLICKED, OnBnClickedRadioCrlf)
    COMMAND_HANDLER(IDC_RADIO_LF, BN_CLICKED, OnBnClickedRadioLf)
    COMMAND_HANDLER(IDC_RADIO_CR, BN_CLICKED, OnBnClickedRadioCr)

    MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
    MESSAGE_HANDLER(WM_MY_MESSAGE, OnUser)
    END_MSG_MAP()
    LRESULT OnBnClickedRadioStretegySmart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioStretegyManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioToOrigin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioToDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioUtf8(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioUtf8bom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioGb18030(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    void CheckAndTraversalIncludeRule(std::function<void(const std::tstring &dotExt)> fn);

    LRESULT OnBnClickedRadioOther(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedButtonAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedButtonAddDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedButtonSetOutputDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnCbnSelchangeComboOtherCharset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    // 右键点击listview
    LRESULT OnNMRclickListview(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & /*bHandled*/);

    LRESULT OnOpenWithNotepad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnRemoveItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnSpecifyOriginCharset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnEnChangeEditIncludeText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioStretegyNoFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnNMClickSyslink1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & /*bHandled*/);
    LRESULT OnBnClickedCheckConvertReturn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioCrlf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioLf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnBnClickedRadioCr(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

    void PostUIFunc(std::function<void()> fn);
    LRESULT OnUser(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

    /**
     * @brief 禁用控件，只保留一个取消按钮。
     * @return vector<被禁用控件的id, 恢复时应该设置的enable值>
     */
    std::vector<std::pair<int, bool>> SetBusyState() noexcept;

    /**
     * @brief 恢复控件。
     * @return vector<被禁用控件的id, 恢复时应该设置的enable值>
     */
    void RestoreReadyState(const std::vector<std::pair<int, bool>> &restore) noexcept;

    void AppendListViewItem(std::wstring filename, uint64_t fileSize, CharsetCode charset,
                            Configuration::LineBreaks lineBreak, std::wstring textPiece) noexcept;
};