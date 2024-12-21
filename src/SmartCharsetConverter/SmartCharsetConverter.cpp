#include <Control/ControlStyle.h>

#include "CLIHandler.h"
#include "DialogMain.h"
#include "Common/CommandLineParser.h"

#include <Common/tstring.h>
#include <Common/ErrorFunction.h>

#include <stdexcept>
#include <filesystem>

using namespace std;

#ifndef NDEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR szCmdLine, int nCmdShow) {
#endif

    // 得到命令行参数
    const vector<wstring> args = GetCommandLineArgs();

    // 是否使用命令模式
    // 进入命令行模式的前提条件：有传入参数，且这些参数不全为文件或文件夹路径
    bool useCli = false;

    // GUI模式下的初始输入文件。目的是文件拖到程序图标上时能够自动加载
    vector<wstring> inputFilenames;

    // 第1个参数是程序路径。多于1个参数说明传入更多参数了
    if (args.size() > 1) {
        for (int i = 1; i < args.size(); ++i) {
            if (!filesystem::is_regular_file(to_string(args[i])) && !filesystem::is_directory(to_string(args[i]))) {
                useCli = true;
                break;
            }
            inputFilenames.push_back(args[i]);
        }

        if (useCli) {
            // 进入命令行模式
            return CLIMain(args);
        }
    }

    // GUI模式
    try {
        InitCommonControls();
        SupportHighDPI();

        {
            DialogMain dialogMain(inputFilenames);
            auto ret = dialogMain.DoModal();
            if (ret == -1) {
                auto code = GetLastError();
                throw runtime_error(string("DoModal fail. \ncode = ") + to_string(code) + string("\ninfo = ") +
                                    to_utf8(GetLastErrorString(code)));
            }
        }

        return 0;
    } catch (const std::exception &err) {
        MessageBox(0, utf8_to_wstring(err.what()).c_str(), L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}