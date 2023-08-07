#define _CRT_SECURE_NO_WARNINGS
#include "FileFunction.h"

#include <algorithm>
#include <tchar.h>
#include <memory>
#include <assert.h>

#include <commdlg.h>

#include <Shlobj.h>   //选择文件夹对话框
#include <shellapi.h> //CommandLineToArgvW
#pragma comment(lib, "Shell32.lib")

#undef max
#undef min

using namespace std;

// 文件对话框 存储文件列表字符串的缓冲长度。MAX_PATH是肯定不够的，选几个就超出了。
// 据说nMaxFile虽然是DWORD类型，但只用了前2个字节，所以这里设成int16_t最大值
const int TFileDialog_BUF_SIZE = 32767;

std::unique_ptr<TCHAR[]> ToTCHARArray(const std::tstring &s) {
    auto size = s.size();
    unique_ptr<TCHAR[]> ret(new TCHAR[size + 1]);
    memcpy(ret.get(), s.data(), size * sizeof(TCHAR));
    ret[size] = TEXT('\0');
    return ret;
}

TFileDialog::TFileDialog(HWND hwndOwner) {
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwndOwner;

    result = unique_ptr<TCHAR[]>(new TCHAR[TFileDialog_BUF_SIZE]);
    result[0] = TEXT('\0');
    ofn.lpstrFile = result.get();
    ofn.nMaxFile = TFileDialog_BUF_SIZE;

    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = NULL;
}

TFileDialog::TFileDialog(HWND hwndOwner, std::vector<std::pair<std::tstring, std::tstring>> vecFilter, bool multiSelect)
    : TFileDialog(hwndOwner) {
    SetFilter(vecFilter);

    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (multiSelect) {
        ofn.Flags |= OFN_ALLOWMULTISELECT;
    }
}

void TFileDialog::SetFilter(std::vector<std::pair<std::tstring, std::tstring>> vecFilter) {
    // lpstrFilter格式：TEXT("机构设计文件(*.lml)\0*.lml\0\0")
    std::tstring ret;
    for (auto &pr : vecFilter) {
        auto &name = pr.first;
        auto &reg = pr.second;
        ret += name + TEXT('\0') + reg + TEXT('\0');
    }
    ret += TEXT('\0');

    filter = ToTCHARArray(ret);

    ofn.lpstrFilter = filter.get(); //两个\0表示结束
}

void TFileDialog::SetTitle(const std::tstring &title) {
    this->title = ToTCHARArray(title);
    ofn.lpstrTitle = this->title.get();
}

bool TFileDialog::Open() {
    TCHAR originDir[1024];
    GetCurrentDirectory(1024, originDir);
    bool ok = ::GetOpenFileName(&ofn);
    if (!ok) {
        auto err = CommDlgExtendedError();

        if (err == 0x3003) // FNERR_BUFFERTOOSMALL
        {
            throw runtime_error("文件选太多。少选一点试试。");
        }
    }

    SetCurrentDirectory(originDir);
    return ok;
}

std::vector<std::tstring> TFileDialog::GetResult() const {
    // 如果没结果，返回空
    if (ofn.lpstrFile[0] == TEXT('\0')) {
        return {};
    }

    // 如果是单选
    if ((ofn.Flags & OFN_ALLOWMULTISELECT) == 0) {
        return {ofn.lpstrFile};
    }

    // 如果是多选
    std::vector<std::tstring> ans;
    tstring dir, temp;
    auto p = ofn.lpstrFile;
    int state = 0;
    for (DWORD i = 0; i < ofn.nMaxFile; ++i) {
        auto c = p[i];
        switch (state) {
        case 0: // 状态0：第一次遇到\0之前
            if (c == TEXT('\0')) {
                ans.push_back(temp);
                temp.clear();
                state = 1;
            } else {
                temp += c;
            }
            break;
        case 1: // 状态1：中途的字符
            if (c == TEXT('\0')) {
                goto end;
            } else {
                temp += c;
                state = 2;

                dir = ans[0];
                ans.pop_back();
            }
            break;
        case 2:                  // 状态2：中途已遇到\0
            if (c == TEXT('\0')) // 再次遇到\0，结束
            {
                ans.push_back(dir + TEXT('\\') + temp);
                temp.clear();
                state = 3;
            } else {
                temp += c;
            }
            break;
        case 3:
            if (c == TEXT('\0')) {
                goto end;
            } else {
                temp += c;
                state = 2;
            }
            break;
        }
    }
end:

    return ans;
}

void TFileDialog::SetResult(const std::tstring &s) {
    result = ToTCHARArray(s);
    ofn.lpstrFile = result.get();
}

bool TFileDialog::Save() {
    ofn.Flags = OFN_PATHMUSTEXIST;

    //设为空可以自动加上选择的后缀名，否则无论选什么后缀，
    //只要没有输入.txt这种，都是无后缀
    ofn.lpstrDefExt = TEXT("");

    return ::GetSaveFileName(&ofn);
}

TFolderBrowser::TFolderBrowser(HWND hwndOwner, std::tstring title) : hWndOwner(hwndOwner), title(title) {}

bool TFolderBrowser::Open(std::tstring &fileName) {
    assert(hWndOwner);
    auto BrowserCallbackProc = [](HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) -> int {
        if (uMsg == BFFM_INITIALIZED) {
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        }
        return 0;
    };

    TCHAR szBuffer[MAX_PATH] = {0};

    BROWSEINFO bi = {0};
    bi.hwndOwner = hWndOwner;
    bi.pszDisplayName = szBuffer; //接收文件夹的缓冲区
    bi.lpszTitle = title.c_str(); //标题
    bi.ulFlags = BIF_NEWDIALOGSTYLE;

    //使用回调传入初始路径
    bi.lParam = (LPARAM)fileName.c_str();
    bi.lpfn = (BFFCALLBACK)BrowserCallbackProc;

    LPITEMIDLIST idl = SHBrowseForFolder(&bi);
    bool ret = SHGetPathFromIDList(idl, szBuffer);
    if (ret)
        fileName = szBuffer;
    return ret;
}

//传入index=1则得到传入文件名
//失败返回空串
#ifdef UNICODE
std::wstring GetCommandLineByIndex(int index) {
    int nArgs;
    LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (szArglist && nArgs >= index + 1) {
        return szArglist[index];
    }
    return L"";
}
#else
std::string GetCommandLineByIndex(int index) {
    int nArgs;
    LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (szArglist && nArgs >= index + 1) {
        return szArglist[index];
    }
    return L"";
}

#endif

bool GetFileExists(const std::tstring filename) {
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    hFind = FindFirstFile(filename.c_str(), &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    } else {
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // is folder
            FindClose(hFind);
            return false;
        } else {
            FindClose(hFind);
            return true;
        }
    }
}

vector<tstring> SplitPath(const std::tstring &s) {
    vector<tstring> ret;
    auto slash = s.find_last_of(TEXT("/\\")); //找最后一个正斜杠或者反斜杠位置

    if (slash != tstring::npos) //存在
    {
        ret.push_back(s.substr(0, slash));             //得到纯路径
        auto vec = SplitFileName(s.substr(slash + 1)); //分割文件名
        ret.insert(ret.end(), vec.begin(), vec.end());
    } else {
        ret.push_back(TEXT(""));
        auto vec = SplitFileName(s);
        ret.insert(ret.end(), vec.begin(), vec.end());
    }
    return ret;
}

vector<tstring> SplitFileName(const std::tstring &s) {
    auto dot = s.find_last_of(TEXT('.')); // 从后寻找.
    if (dot != tstring::npos)
        return {s.substr(0, dot), s.substr(dot)};
    else
        return {s, TEXT("")};
}

std::tstring GetNameAndExt(std::tstring s) noexcept {
    tstring ret;
    auto slash = s.find_last_of(TEXT("/\\")); //从后寻找正反斜杠
    if (slash != tstring::npos) {
        ret = s.substr(slash + 1);
    } else {
        ret = s;
    }
    return ret;
}

// e.g. ext="txt"
tstring ChangeExtend(tstring fileName, tstring ext) {
    auto slash_pos = fileName.find_last_of('\\');
    auto dot_pos = fileName.find_last_of('.');
    if (dot_pos == tstring::npos ||
        (slash_pos != tstring::npos && dot_pos < slash_pos)) // e.g. "Name" "dir.dir\file.ext"
    {
        return fileName + TEXT('.') + ext;
    }
    return fileName.substr(0, dot_pos) + TEXT('.') + ext;
}

std::tstring GetExtend(std::tstring fileName) {
    auto slash_pos = fileName.find_last_of('\\');
    auto dot_pos = fileName.find_last_of('.');
    if (dot_pos == tstring::npos || (slash_pos != tstring::npos && dot_pos < slash_pos)) // e.g. "Name" "dir.dir\file"
    {
        return TEXT("");
    }
    return fileName.substr(dot_pos + 1);
}

//取得文件大小，不改变读写位置
uint64_t GetFileSize(FILE *fp) {
    long long origin_pos = _ftelli64(fp);
    _fseeki64(fp, 0LL, SEEK_END);
    long long ret = _ftelli64(fp);
    _fseeki64(fp, origin_pos, SEEK_SET);
    return ret;
}

uint64_t GetFileSize(std::tstring fileName) {
    FILE *fp = _tfopen(fileName.c_str(), TEXT("rb"));
    if (fp == nullptr)
        throw file_io_error(to_string(TEXT("\"") + fileName + TEXT("\"") + TEXT(" can't open.")).c_str(), fileName);
    uint64_t sz = GetFileSize(fp);
    fclose(fp);
    return sz;
}

std::tstring FileSizeToTString(uint64_t fileSize) {
    double d;
    TCHAR buf[64];
    if (fileSize >= GB) {
        d = fileSize / (double)GB;
        _stprintf(buf, TEXT("%.2f GB"), d);
    } else {
        if (fileSize >= MB) {
            d = fileSize / (double)MB;
            _stprintf(buf, TEXT("%.2f MB"), d);
        } else if (fileSize >= KB) {
            d = fileSize / (double)KB;
            _stprintf(buf, TEXT("%.2f KB"), d);
        } else {
            d = static_cast<double>(fileSize);
            _stprintf(buf, TEXT("%.0f B"), d);
        }
    }

    return buf;
}

std::tuple<std::unique_ptr<char[]>, uint64_t> ReadFileToBuffer(std::tstring fileName, uint64_t limitSize) {
    uint64_t bufSize = GetFileSize(fileName);

    if (limitSize != 0) {
        bufSize = std::min(bufSize, limitSize);
    }

    unique_ptr<char[]> buf = make_unique<char[]>(bufSize);

    // 如果是空文件，就不用读了
    if (bufSize == 0) {
        return make_tuple(std::move(buf), bufSize);
    }

    FILE *fp = _tfopen(fileName.c_str(), TEXT("rb"));
    if (fp == nullptr)
        throw file_io_error(to_string(TEXT("\"") + fileName + TEXT("\"") + TEXT(" can't open.")).c_str(), fileName);
    auto ret = fread(buf.get(), bufSize, 1, fp);
    if (ret < 1) {
        fclose(fp);
        throw file_io_error(to_string(TEXT("\"") + fileName + TEXT("\"") + TEXT(" can't open.")).c_str(), fileName);
    }
    fclose(fp);

    return make_tuple(std::move(buf), bufSize);
}

void WriteFileFromBuffer(std::tstring fileName, const char buf[], uint64_t bufSize) {
    FILE *fp = _tfopen(fileName.c_str(), TEXT("wb"));
    if (fp == nullptr)
        throw file_io_error(to_string(TEXT("\"") + fileName + TEXT("\"") + TEXT(" can't open.")).c_str(), fileName);
    auto ret = fwrite(buf, bufSize, 1, fp);
    if (ret < 1) {
        fclose(fp);
        throw file_io_error(to_string(TEXT("\"") + fileName + TEXT("\"") + TEXT(" can't write.")).c_str(), fileName);
    }
    fclose(fp);
}

void WriteDetailFile(HWND hWnd, std::tstring filename, std::function<void(std::tofstream &ofs)> fnWrite) {
    TFileDialog fileDialog(hWnd, {{TEXT("txt"), TEXT("*.txt")}});
    fileDialog.SetResult(filename);
    if (fileDialog.Save()) {
        tofstream ofs(filename);

        if (ofs.fail()) {
            tstring errInfo = TEXT("写入") + filename + TEXT("失败。可能是文件正在被占用或者没有权限。");
            ::MessageBox(hWnd, errInfo.c_str(), TEXT("出错"), MB_OK | MB_ICONERROR);
            return;
        }

        fnWrite(ofs);
        ofs.close();

        tstring info = filename + TEXT("已写入。");
        ::MessageBox(hWnd, info.c_str(), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
    }
}

//失败抛出file_io_error类型自定义异常
void findFiles(std::vector<std::tstring> &ret, std::tstring lpPath, std::vector<std::tstring> dotextNames,
               bool enterSubFolder) {
    tstring szFile;
    WIN32_FIND_DATA FindFileData;

    tstring szFind = lpPath + TEXT("\\*");

    HANDLE hFind = ::FindFirstFile(szFind.c_str(), &FindFileData);

    if (INVALID_HANDLE_VALUE == hFind) {
        throw file_io_error(to_string(tstring(TEXT("Invalid folder:")) + szFind).c_str(), szFind);
        return;
    }

    do {
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            //是目录
            if (FindFileData.cFileName[0] != TEXT('.')) //不是以.开头的目录
            {
                if (enterSubFolder) {
                    //进入子目录
                    szFile = lpPath + TEXT("\\") + FindFileData.cFileName;
                    findFiles(ret, szFile, dotextNames, enterSubFolder);
                }
            }
        } else {
            //是文件
            std::tstring filePath = lpPath + TEXT("\\") + FindFileData.cFileName;

            if (dotextNames.empty()) {
                ret.push_back(filePath);
            } else {
                auto tstolower = [](tstring s) -> tstring {
                    tstring temp(s);
                    for_each(temp.begin(), temp.end(), [](TCHAR &c) {
                        c = tolower(c);
                    });
                    return temp;
                };

                tstring filename(tstolower(FindFileData.cFileName));
                for (auto &dotext : dotextNames) {
                    if (filename.substr(filename.length() - dotext.length(), dotext.length()) ==
                        dotext) //文件名后n位和扩展名相同
                    {
                        ret.push_back(filePath);
                    }
                }
            }
            //        if ( szFile.empty() )
            //        {
            ////szFile为空，为当前目录
            //            std::string filePath = szFile+FindFileData.cFileName;
            //            file_lists.push_back(filePath);
            //        }
            //        else
            //        {
            ////非当前目录，输出全路径
            //            std::string filePath = lpPath+"\\"+FindFileData.cFileName;
            //            file_lists.push_back(filePath);
            //        }
        }

    } while (::FindNextFile(hFind, &FindFileData));

    ::FindClose(hFind);
}

//失败抛出file_io_error类型自定义异常
std::vector<std::tstring> TraversalAllFileNames(std::tstring lpPath, std::vector<std::tstring> dotextNames,
                                                bool enterSubFolder) {
    std::vector<std::tstring> file_lists;
    findFiles(file_lists, lpPath, dotextNames, enterSubFolder);
    return file_lists;
}