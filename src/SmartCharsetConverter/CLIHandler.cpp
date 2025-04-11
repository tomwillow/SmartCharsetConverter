#include "CLIHandler.h"

#include "Core/Core.h"
#include "Common/tstring.h"
#include "Common/FileFunction.h"
#include "Common/ConsoleSettings.h"

#include <guicon/guicon.h>

#include <sstream>
#include <iostream>
#include <filesystem>
#include <memory>

using std::cerr;
using std::cout;

const std::string configFileName = "SmartCharsetConverter.json";

const char usage[] = u8R"(
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

int CLIMain(const std::vector<std::string> &args) noexcept {
    std::setlocale(LC_CTYPE, ".UTF-8");

    try {
        bool ok = AttachParentConsole(1024);
        if (!ok) {
            throw std::runtime_error("failed to AttachParentConsole");
        }

    } catch (const std::runtime_error &err) {
        MessageBoxW(NULL, utf8_to_wstring(err.what()).c_str(), L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    std::shared_ptr<void> defer(nullptr, [](auto) {
        try {
            bool ok = ReleaseConsole();
            if (!ok) {
                throw std::runtime_error("failed to ReleaseConsole");
            }

        } catch (const std::runtime_error &err) {
            MessageBoxW(NULL, utf8_to_wstring(err.what()).c_str(), L"Error", MB_OK | MB_ICONERROR);
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

    std::stringstream ssErr;
    std::stringstream ssOutput;

    std::vector<std::string> inputPathes;

    int state = 0;
    for (std::size_t i = 1; i < args.size(); ++i) {
        std::string arg = args[i];
        switch (state) {
        case 0:
            if (arg == u8"--help") {
                taskType = TaskType::PURE_PRINT;
                if (i == args.size() - 1) {
                    ssOutput << usage;
                    break;
                }

                state = 10;
                break;
            }
            if (arg == u8"--input") {
                if (setInput) {
                    ssErr << u8"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                state = 20;
                break;
            }
            if (arg == u8"--target_charset") {
                if (setTargetCharset) {
                    ssErr << u8"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                state = 30;
                break;
            }
            if (arg == u8"--target_linebreak") {
                if (setTargetLineBreak) {
                    ssErr << u8"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                state = 40;
                break;
            }
            if (arg == u8"--output_origin") {
                if (setOutput) {
                    ssErr << u8"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                setOutput = true;
                core.SetOutputTarget(Configuration::OutputTarget::ORIGIN);
                break;
            }
            if (arg == u8"--output_dir") {
                if (setOutput) {
                    ssErr << u8"错误：重复设置参数：" << arg << "\n";
                    break;
                }
                state = 50;
                break;
            }

            ssErr << u8"无效参数：" << arg;
            i = args.size(); // 让最外层循环退出
            break;
        case 10: // --help xxx
            if (arg == u8"charset") {
                ssOutput << u8"支持的字符集有：\n";
                for (int i = static_cast<int>(CharsetCode::UTF8); i < static_cast<int>(CharsetCode::CHARSET_CODE_END);
                     ++i) {

                    CharsetCode code = static_cast<CharsetCode>(i);
                    ssOutput << ToViewCharsetName(code) << u8"\n";
                }
                break;
            }

            ssErr << u8"错误：无效参数：" << arg;
            i = args.size(); // 让最外层循环退出
            break;
        case 20: // --input xxx
        {
            setInput = true;
            std::filesystem::path path = std::filesystem::u8path(arg);
            if (std::filesystem::is_regular_file(path) || std::filesystem::is_directory(path)) {
                inputPathes.push_back(arg);
                break;
            }
            if (arg.substr(0, 2) == u8"--") {
                state = 0;
                i--;
                break;
            }

            ssErr << u8"错误：无效路径：" << arg << u8"\n";
            break;
        }
        case 30:
            setTargetCharset = true;
            try {
                core.SetOutputCharset(ToCharsetCode(arg));
            } catch (const std::runtime_error &err) {
                (err);
                ssErr << u8"错误：未能识别的字符集名称：" << arg << u8"\n";
                ssErr << u8"提示：使用--help charset可以查看支持的字符集名称。\n";
            }
            state = 0;
            break;
        case 40:
            setTargetLineBreak = true;
            core.SetEnableConvertLineBreak(true);
            if (tolower(arg) == tolower(std::string(u8"LF")) || tolower(arg) == tolower(std::string(u8"Linux"))) {
                core.SetLineBreaks(LineBreaks::LF);
                break;
            }
            if (tolower(arg) == tolower(std::string(u8"CRLF")) || tolower(arg) == tolower(std::string(u8"Windows"))) {
                core.SetLineBreaks(LineBreaks::CRLF);
                break;
            }
            if (tolower(arg) == tolower(std::string(u8"CR")) || tolower(arg) == tolower(std::string(u8"Mac"))) {
                core.SetLineBreaks(LineBreaks::CR);
                break;
            }
            ssErr << u8"错误：未能识别的换行符名称：" << arg << u8"\n";
            ssErr << u8"提示：使用--help可以查看换行符名称。\n";
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
        if (inputPathes.empty()) {
            ssErr << u8"错误：没有设置输入文件（--input）。" << u8"\n";
        }

        if (!setOutput) {
            ssErr << u8"错误：没有设置输出方式（--output_origin或者--output_dir）。" << u8"\n";
        } else {
            if (core.GetConfig().outputTarget == Configuration::OutputTarget::ORIGIN) {
                ssOutput << u8"输出方式：原位输出\n";
            } else {
                ssOutput << u8"输出方式：输出到文件夹：" << core.GetConfig().outputDir << u8"\n";
            }
        }

        if (!setTargetCharset) {
            ssErr << u8"错误：没有设置目标字符集（--targetCharset）。" << u8"\n";
        } else {
            ssOutput << u8"目标字符集：" << ToViewCharsetName(core.GetConfig().outputCharset) << u8"\n";
        }

        if (setTargetLineBreak) {
            ssOutput << u8"目标换行符：" << LineBreaksToViewName(core.GetConfig().lineBreak) << u8"\n";
        }
    }

    // 开始输出
    cout << std::string(32, L'=') << u8"\n";

    int retCode = 0;
    if (!ssErr.str().empty()) {
        SetConsoleColor(ConsoleColor::RED);
        std::cerr << u8"输入参数：" << u8"\n";
        for (auto arg : args) {
            std::cerr << arg << u8" ";
        }
        std::cerr << u8"\n\n";
        std::cerr << ssErr.str() << u8"\n";
        retCode = -1;
        SetConsoleColor();
        return -1;
    }

    SetConsoleColor(ConsoleColor::GREEN);
    std::cout << ssOutput.str();
    SetConsoleColor();

    cout << std::string(32, L'=') << u8"\n";
    cout << u8"\n";

    ssErr.swap(std::stringstream{});
    ssOutput.swap(std::stringstream{});

    // 开始转换

    auto AddAndConvertOneFile = [&core, &ssOutput, setTargetLineBreak](
                                    int index, int total, const std::string &inputFilename, int &success, int &failed) {
        Core::AddItemResult addedItem;
        SetConsoleColor(ConsoleColor::YELLOW);
        std::cout << u8"[" << std::to_string(index) << u8"/" << std::to_string(total) << u8"] " << inputFilename
                  << u8"\n";
        SetConsoleColor();
        try {
            addedItem = core.AddItem(inputFilename, {});
            // std::cout << u8"  文本片段: " << addedItem.strPiece << u8"\n";
        } catch (const std::runtime_error &err) {
            SetConsoleColor(ConsoleColor::RED);
            cerr << u8"读入文件失败。原因: " << err.what() << u8"\n";
            cerr << u8"\n";
            SetConsoleColor();
            failed++;
            return;
        }

        Core::ConvertFileResult ret = core.Convert(inputFilename, addedItem.srcCharset, addedItem.srcLineBreak);
        if (ret.errInfo.has_value()) {
            cout << u8"  大小: " << FileSizeToHumanString(addedItem.filesize) << u8"\n";
            cout << u8"  字符集: " << ToViewCharsetName(addedItem.srcCharset) << u8"\n";
            cout << u8"  换行符: " << LineBreaksToViewName(addedItem.srcLineBreak) << u8"\n";
            SetConsoleColor(ConsoleColor::RED);
            cerr << u8"转换失败。原因: " << ret.errInfo.value() << u8"\n";
            cerr << u8"\n";
            SetConsoleColor();
            failed++;
            return;
        }

        cout << u8"  大小: " << FileSizeToHumanString(addedItem.filesize) << u8"\n";
        cout << u8"  字符集: " << ToViewCharsetName(addedItem.srcCharset) << u8" -> ";

        SetConsoleColor(ConsoleColor::GREEN);
        cout << ToViewCharsetName(core.GetConfig().outputCharset) << u8"\n";
        SetConsoleColor();

        cout << u8"  换行符: " << LineBreaksToViewName(addedItem.srcLineBreak);

        if (setTargetLineBreak) {
            cout << u8" -> ";
            SetConsoleColor(ConsoleColor::GREEN);
            cout << LineBreaksToViewName(core.GetConfig().lineBreak) << u8"\n";
            SetConsoleColor();
        } else {
            cout << u8"\n";
        }

        cout << u8"转换成功。\n\n";
        success++;
        return;
    };

    std::vector<std::string> inputFileNames;
    for (auto &inputPath : inputPathes) {
        if (std::filesystem::is_regular_file(std::filesystem::u8path(inputPath))) {
            inputFileNames.push_back(inputPath);
            continue;
        }

        for (auto &path : std::filesystem::recursive_directory_iterator(std::filesystem::u8path(inputPath))) {
            if (std::filesystem::is_regular_file(path)) {
                inputFileNames.push_back(path.path().u8string());
                continue;
            }
        }
    }

    int success = 0, failed = 0;
    int total = static_cast<int>(inputFileNames.size());
    for (int i = 0; i < total; i++) {
        AddAndConvertOneFile(i + 1, total, inputFileNames[i], success, failed);
    }

    cout << std::string(32, L'=') << u8"\n";
    cout << u8"总计：" << std::to_string(total) << u8"\n";
    SetConsoleColor(ConsoleColor::GREEN);
    cout << u8"成功：" << std::to_string(success) << u8"\n";
    if (failed > 0) {
        SetConsoleColor(ConsoleColor::RED);
    }
    cout << u8"失败：" << std::to_string(failed) << u8"\n";
    SetConsoleColor();

    return retCode;
}
