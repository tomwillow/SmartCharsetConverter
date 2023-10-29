#include "CLIHandler.h"

#include "Core.h"
#include "tstring.h"

#include <sstream>
#include <iostream>
#include <filesystem>

using std::wcout;

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
    enum class TaskType { PURE_PRINT, CONVERT };
    TaskType taskType = TaskType::CONVERT;
    bool setInput = false;
    bool setTargetCharset = false;
    bool setTargetLineBreak = false;
    bool setOutput = false;

    CoreInitOption coreInitOpt;
    coreInitOpt.fnUIAddItem = [](std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                                 std::wstring lineBreakStr, std::wstring textPiece) {
        std::wcout << L"已读取：\n";
        std::wcout << L"  文件名: " << filename << L"\n";
        std::wcout << L"  大小: " << fileSizeStr << L"\n";
        std::wcout << L"  字符集: " << charsetStr << L"\n";
        std::wcout << L"  换行符: " << lineBreakStr << L"\n";
        std::wcout << L"  文本片段: " << textPiece << L"\n";
    };
    Core core(TEXT("SmartCharsetConverter.ini"), coreInitOpt);

    core.SetFilterMode(Configuration::FilterMode::NO_FILTER);

    std::wcout.imbue(std::locale(""));
    std::wcerr.imbue(std::locale(""));

    std::wstringstream ssErr;
    std::wstringstream ssOutput;

    std::vector<std::wstring> inputFilenames;

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
                inputFilenames.push_back(arg);
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
                ssErr << L"错误：未能识别的字符集名称：" << arg << "。\n";
                ssErr << L"提示：使用--help charset可以查看支持的字符集名称。\n";
            }
            state = 0;
            break;
        case 40:
            setTargetLineBreak = true;
            core.SetEnableConvertLineBreak(true);
            if (tolower(arg) == tolower(std::wstring(L"LF")) && tolower(arg) == tolower(std::wstring(L"Linux"))) {
                core.SetLineBreaks(Configuration::LineBreaks::LF);
                break;
            }
            if (tolower(arg) == tolower(std::wstring(L"CRLF")) && tolower(arg) == tolower(std::wstring(L"Windows"))) {
                core.SetLineBreaks(Configuration::LineBreaks::CRLF);
                break;
            }
            if (tolower(arg) == tolower(std::wstring(L"CR")) && tolower(arg) == tolower(std::wstring(L"Mac"))) {
                core.SetLineBreaks(Configuration::LineBreaks::CR);
                break;
            }
            ssErr << L"错误：未能识别的换行符名称：" << arg << "。\n";
            ssErr << L"提示：使用--help可以查看换行符名称。\n";
            state = 0;
            break;
        case 50:
            setOutput = true;
            core.SetOutputTarget(Configuration::OutputTarget::TO_DIR);
            core.SetOutputDir(arg);
            state = 0;
            break;
        } // end of switch
    }

    // 校验输入参数
    if (taskType == TaskType::CONVERT) {
        if (inputFilenames.empty()) {
            ssErr << L"错误：没有输入文件（--input）。" << L"\n";
        }
        if (!setOutput) {
            ssErr << L"错误：没有设置输出方式（--output_origin或者--output_dir）。" << L"\n";
        }
        if (!setTargetCharset) {
            ssErr << L"错误：没有设置目标字符集（--targetCharset）。" << L"\n";
        }
    }

    // 开始转换
    for (auto &inputFilename : inputFilenames) {
        try {
            core.AddItem(inputFilename, {});
        } catch (const std::runtime_error &err) {
            wcout << "读入文件失败：\n";
            wcout << L"  文件名: " << inputFilename << L"\n";
            wcout << L"  原因: " << to_wstring(err.what()) << L"\n";
        }
    }

    int retCode = 0;
    if (!ssErr.str().empty()) {
        std::wcerr << L"输入参数：" << L"\n";
        for (auto arg : args) {
            std::wcerr << arg << L" ";
        }
        std::wcerr << L"\n\n";
        std::wcerr << ssErr.str() << L"\n";
        retCode = -1;
    }

    std::wcout << ssOutput.str() << L"\n";

    return retCode;
}
