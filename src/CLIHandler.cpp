#include "CLIHandler.h"

#include "Core.h"
#include "tstring.h"
#include "FileFunction.h"
#include "ConsoleSettings.h"

#include <guicon/guicon.h>

#include <sstream>
#include <iostream>
#include <filesystem>
#include <memory>

using std::wcerr;
using std::wcout;

const std::tstring configFileName = TEXT("SmartCharsetConverter.json");

const wchar_t usage[] = LR"(
SmartCharsetConverter --help [<options>]
SmartCharsetConverter --input <path>... --target_charset <charset> [--target_linebreak <linebreak>] [--output_origin | --output_dir <dir>]

--help [<options>]
  打印帮助信息。
  options:
  * charset
      打印出支持的字符集名称。
      例如：--help charset

--input <path>...
  指定输入文件或者输入文件夹
  例如：--input D:\a.txt D:\input

--target_charset <charset>
  指定目标字符集
  例如：--target_charset UTF-8

--target_linebreak <linebreak>
  指定目标换行符
  * linebreak: 
      LF 或者 Linux
      CRLF 或者 Windows
      CR 或者 Mac

--output_origin
  转换后直接覆盖输入文件

--output_dir <dir>
  指定输出的文件夹

)";

int CLIMain(const std::vector<std::wstring> &args) noexcept {

    try {
        bool ok = AttachParentConsole(1024);
        if (!ok) {
            throw std::runtime_error("failed to AttachParentConsole");
        }

    } catch (const std::runtime_error &err) {
        MessageBoxA(NULL, err.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    std::shared_ptr<void> defer(nullptr, [](auto) {
        try {
            bool ok = ReleaseConsole();
            if (!ok) {
                throw std::runtime_error("failed to ReleaseConsole");
            }

        } catch (const std::runtime_error &err) {
            MessageBoxA(NULL, err.what(), "Error", MB_OK | MB_ICONERROR);
            return -1;
        }
    });

    // ==================== 命令行已挂载 =====================

    enum class TaskType { PURE_PRINT, CONVERT };
    TaskType taskType = TaskType::CONVERT;
    bool setInput = false;
    bool setTargetCharset = false;
    bool setTargetLineBreak = false;
    bool setOutput = false;

    CoreInitOption coreInitOpt;
    Core core(configFileName, coreInitOpt);

    core.SetFilterMode(Configuration::FilterMode::NO_FILTER);

    std::wcout.imbue(std::locale(""));
    std::wcerr.imbue(std::locale(""));

    std::wstringstream ssErr;
    std::wstringstream ssOutput;

    std::vector<std::wstring> inputPathes;

    int state = 0;
    for (int i = 1; i < args.size(); ++i) {
        std::wstring arg = args[i];
        switch (state) {
        case 0:
            if (arg == L"--help") {
                taskType = TaskType::PURE_PRINT;
                if (i == args.size() - 1) {
                    ssOutput << usage;
                    break;
                }

                state = 10;
                break;
            }
            if (arg == L"--input") {
                if (setInput) {
                    ssErr << L"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                state = 20;
                break;
            }
            if (arg == L"--target_charset") {
                if (setTargetCharset) {
                    ssErr << L"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                state = 30;
                break;
            }
            if (arg == L"--target_linebreak") {
                if (setTargetLineBreak) {
                    ssErr << L"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                state = 40;
                break;
            }
            if (arg == L"--output_origin") {
                if (setOutput) {
                    ssErr << L"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                setOutput = true;
                core.SetOutputTarget(Configuration::OutputTarget::ORIGIN);
                break;
            }
            if (arg == L"--output_dir") {
                if (setOutput) {
                    ssErr << L"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                state = 50;
                break;
            }

            ssErr << L"无效参数：" << arg;
            i = args.size(); // 让最外层循环退出
            break;
        case 10: // --help xxx
            if (arg == L"charset") {
                ssOutput << L"支持的字符集有：\n";
                for (int i = static_cast<int>(CharsetCode::UTF8); i < static_cast<int>(CharsetCode::CHARSET_CODE_END);
                     ++i) {

                    CharsetCode code = static_cast<CharsetCode>(i);
                    ssOutput << ToViewCharsetName(code) << L"\n";
                }
                break;
            }

            ssErr << L"错误：无效参数：" << arg;
            i = args.size(); // 让最外层循环退出
            break;
        case 20: // --input xxx
            setInput = true;
            if (std::filesystem::is_regular_file(arg) || std::filesystem::is_directory(arg)) {
                inputPathes.push_back(arg);
                break;
            }
            if (arg.substr(0, 2) == L"--") {
                state = 0;
                i--;
                break;
            }

            ssErr << L"错误：无效路径：" << arg << L"\n";
            break;
        case 30:
            setTargetCharset = true;
            try {
                core.SetOutputCharset(ToCharsetCode(arg));
            } catch (const std::runtime_error &err) {
                ssErr << L"错误：未能识别的字符集名称：" << arg << L"\n";
                ssErr << L"提示：使用--help charset可以查看支持的字符集名称。\n";
            }
            state = 0;
            break;
        case 40:
            setTargetLineBreak = true;
            core.SetEnableConvertLineBreak(true);
            if (tolower(arg) == tolower(std::wstring(L"LF")) || tolower(arg) == tolower(std::wstring(L"Linux"))) {
                core.SetLineBreaks(LineBreaks::LF);
                break;
            }
            if (tolower(arg) == tolower(std::wstring(L"CRLF")) || tolower(arg) == tolower(std::wstring(L"Windows"))) {
                core.SetLineBreaks(LineBreaks::CRLF);
                break;
            }
            if (tolower(arg) == tolower(std::wstring(L"CR")) || tolower(arg) == tolower(std::wstring(L"Mac"))) {
                core.SetLineBreaks(LineBreaks::CR);
                break;
            }
            ssErr << L"错误：未能识别的换行符名称：" << arg << L"\n";
            ssErr << L"提示：使用--help可以查看换行符名称。\n";
            state = 0;
            break;
        case 50:
            setOutput = true;
            core.SetOutputTarget(Configuration::OutputTarget::TO_DIR);
            core.SetOutputDir(to_utf8(arg));
            state = 0;
            break;
        } // end of switch
    }

    // 校验输入参数
    if (taskType == TaskType::CONVERT) {
        if (inputPathes.empty()) {
            ssErr << L"错误：没有设置输入文件（--input）。" << L"\n";
        }

        if (!setOutput) {
            ssErr << L"错误：没有设置输出方式（--output_origin或者--output_dir）。" << L"\n";
        } else {
            if (core.GetConfig().outputTarget == Configuration::OutputTarget::ORIGIN) {
                ssOutput << L"输出方式：原位输出\n";
            } else {
                ssOutput << L"输出方式：输出到文件夹：" << utf8_to_wstring(core.GetConfig().outputDir) << L"\n";
            }
        }

        if (!setTargetCharset) {
            ssErr << L"错误：没有设置目标字符集（--targetCharset）。" << L"\n";
        } else {
            ssOutput << L"目标字符集：" << ToViewCharsetName(core.GetConfig().outputCharset) << L"\n";
        }

        if (setTargetLineBreak) {
            ssOutput << L"目标换行符：" << lineBreaksMap.at(core.GetConfig().lineBreak) << L"\n";
        }
    }

    // 开始输出
    wcout << std::wstring(32, L'=') << L"\n";

    int retCode = 0;
    if (!ssErr.str().empty()) {
        SetConsoleColor(ConsoleColor::RED);
        std::wcerr << L"输入参数：" << L"\n";
        for (auto arg : args) {
            std::wcerr << arg << L" ";
        }
        std::wcerr << L"\n\n";
        std::wcerr << ssErr.str() << L"\n";
        retCode = -1;
        SetConsoleColor();
        return -1;
    }

    SetConsoleColor(ConsoleColor::GREEN);
    std::wcout << ssOutput.str();
    SetConsoleColor();

    wcout << std::wstring(32, L'=') << L"\n";
    wcout << L"\n";

    ssErr.swap(std::wstringstream{});
    ssOutput.swap(std::wstringstream{});

    // 开始转换

    auto AddAndConvertOneFile = [&core, &ssOutput, setTargetLineBreak](int index, int total,
                                                                       const std::wstring &inputFilename, int &success,
                                                                       int &failed) {
        Core::AddItemResult addedItem;
        SetConsoleColor(ConsoleColor::YELLOW);
        std::wcout << L"[" << to_wstring(std::to_string(index)) << L"/" << to_wstring(std::to_string(total)) << L"] "
                   << inputFilename << L"\n";
        SetConsoleColor();
        try {
            addedItem = core.AddItem(inputFilename, {});
            // std::wcout << L"  文本片段: " << addedItem.strPiece << L"\n";
        } catch (const std::runtime_error &err) {
            SetConsoleColor(ConsoleColor::RED);
            wcerr << L"读入文件失败。原因: " << to_wstring(err.what()) << L"\n";
            wcerr << L"\n";
            SetConsoleColor();
            failed++;
            return;
        }

        Core::ConvertResult ret = core.Convert(inputFilename, addedItem.srcCharset, addedItem.srcLineBreak);
        if (ret.errInfo.has_value()) {
            wcout << L"  大小: " << FileSizeToTString(addedItem.filesize) << L"\n";
            wcout << L"  字符集: " << ToViewCharsetName(addedItem.srcCharset) << L"\n";
            wcout << L"  换行符: " << lineBreaksMap.at(addedItem.srcLineBreak) << L"\n";
            SetConsoleColor(ConsoleColor::RED);
            wcerr << L"转换失败。原因: " << utf8_to_wstring(ret.errInfo.value()) << L"\n";
            wcerr << L"\n";
            SetConsoleColor();
            failed++;
            return;
        }

        wcout << L"  大小: " << FileSizeToTString(addedItem.filesize) << L"\n";
        wcout << L"  字符集: " << ToViewCharsetName(addedItem.srcCharset) << L" -> ";

        SetConsoleColor(ConsoleColor::GREEN);
        wcout << ToViewCharsetName(core.GetConfig().outputCharset) << L"\n";
        SetConsoleColor();

        wcout << L"  换行符: " << lineBreaksMap.at(addedItem.srcLineBreak);

        if (setTargetLineBreak) {
            wcout << L" -> ";
            SetConsoleColor(ConsoleColor::GREEN);
            wcout << lineBreaksMap.at(core.GetConfig().lineBreak) << L"\n";
            SetConsoleColor();
        } else {
            wcout << L"\n";
        }

        wcout << L"转换成功。\n\n";
        success++;
        return;
    };

    std::vector<std::wstring> inputFileNames;
    for (auto &inputPath : inputPathes) {
        if (std::filesystem::is_regular_file(inputPath)) {
            inputFileNames.push_back(inputPath);
            continue;
        }

        for (auto &path : std::filesystem::recursive_directory_iterator(inputPath)) {
            if (std::filesystem::is_regular_file(path)) {
                inputFileNames.push_back(path.path());
                continue;
            }
        }
    }

    int success = 0, failed = 0;
    int total = inputFileNames.size();
    for (int i = 0; i < total; i++) {
        AddAndConvertOneFile(i + 1, total, inputFileNames[i], success, failed);
    }

    wcout << std::wstring(32, L'=') << L"\n";
    wcout << L"总计：" << to_wstring(std::to_string(total)) << L"\n";
    SetConsoleColor(ConsoleColor::GREEN);
    wcout << L"成功：" << to_wstring(std::to_string(success)) << L"\n";
    if (failed > 0) {
        SetConsoleColor(ConsoleColor::RED);
    }
    wcout << L"失败：" << to_wstring(std::to_string(failed)) << L"\n";
    SetConsoleColor();

    return retCode;
}
