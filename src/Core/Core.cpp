#include "Core.h"

#include "Language.h"
#include "Detect.h"
#include "UCNVHelper.h"

#include <FileFunction.h>

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <compact_enc_det/compact_enc_det.h>

#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

constexpr uint64_t tryReadSize = 100Ui64 * KB;

std::u16string Decode(std::string_view src, CharsetCode code) {
    if (code == CharsetCode::EMPTY) {
        return {};
    }

    if (charsetCodeMap.at(code).isVietnameseLocalCharset) {
        viet::Init();
        return viet::ConvertToUtf16LE(src, CharsetCodeToVietEncoding(code));
    }

    // 从code转换到icu的字符集名称
    auto icuCharsetName = ToICUCharsetName(code);

    UErrorCode err = U_ZERO_ERROR;

    // 打开转换器
    unique_ptr<UConverter, function<void(UConverter *)>> conv(ucnv_open(to_string(icuCharsetName).c_str(), &err),
                                                              [](UConverter *p) {
                                                                  ucnv_close(p);
                                                              });
    DealWithUCNVError(err);

    std::size_t cap = src.size() + 1;
    std::u16string target(cap, u'\u0000');

    ucnv_setToUCallBack(conv.get(), UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
    DealWithUCNVError(err);

    // 解码
    int retLen = ucnv_toUChars(conv.get(), target.data(), cap, src.data(), src.size(), &err);
    target.resize(retLen);
    DealWithUCNVError(err);

    return target;
}

// below copied from https://github.com/unicode-org/icu/blob/main/icu4c/source/samples/ucnv/flagcb.c

/* The structure of a FromU Flag context.
   (conceivably there could be a ToU Flag Context) */
struct FromUFLAGContext {
    UConverterFromUCallback subCallback;
    const void *subContext;
    std::vector<UChar32> unassigned; // 是否出现了不能转换的字符
    FromUFLAGContext() : subCallback(nullptr), subContext(nullptr) {}
};

/**
 * the actual callback
 */
U_CAPI void U_EXPORT2 flagCB_fromU(const void *context, UConverterFromUnicodeArgs *fromUArgs, const UChar *codeUnits,
                                   int32_t length, UChar32 codePoint, UConverterCallbackReason reason,
                                   UErrorCode *err) {
    /* First step - based on the reason code, take action */
    FromUFLAGContext *ctx = reinterpret_cast<FromUFLAGContext *>(const_cast<void *>(context));

    if (reason == UCNV_UNASSIGNED) { /* whatever set should be trapped here */
        if (ctx->unassigned.size() < 32)
            ctx->unassigned.push_back(codePoint);
    }

    if (reason == UCNV_CLONE) {
        /* The following is the recommended way to implement UCNV_CLONE
           in a callback. */
        UConverterFromUCallback saveCallback;
        const void *saveContext;
        UErrorCode subErr = U_ZERO_ERROR;

        FromUFLAGContext *cloned;
        *cloned = *ctx;

        /* We need to get the sub CB to handle cloning,
         * so we have to set up the following, temporarily:
         *
         *   - Set the callback+context to the sub of this (flag) cb
         *   - preserve the current cb+context, it could be anything
         *
         *   Before:
         *      CNV  ->   FLAG ->  subcb -> ...
         *
         *   After:
         *      CNV  ->   subcb -> ...
         *
         *    The chain from 'something' on is saved, and will be restored
         *   at the end of this block.
         *
         */

        ucnv_setFromUCallBack(fromUArgs->converter, cloned->subCallback, cloned->subContext, &saveCallback,
                              &saveContext, &subErr);

        if (cloned->subCallback != NULL) {
            /* Now, call the sub callback if present */
            cloned->subCallback(cloned->subContext, fromUArgs, codeUnits, length, codePoint, reason, err);
        }

        ucnv_setFromUCallBack(fromUArgs->converter, saveCallback, /* Us */
                              cloned,                             /* new context */
                              &cloned->subCallback,               /* IMPORTANT! Accept any change in CB or context */
                              &cloned->subContext, &subErr);

        if (U_FAILURE(subErr)) {
            *err = subErr;
        }
    }

    /* process other reasons here if need be */

    /* Always call the subCallback if present */
    if (ctx->subCallback != NULL && reason != UCNV_CLONE) {
        ctx->subCallback(ctx->subContext, fromUArgs, codeUnits, length, codePoint, reason, err);
    }

    /* cleanup - free the memory AFTER calling the sub CB */
    if (reason == UCNV_CLOSE) {
        delete context;
    }
}

std::string Encode(std::u16string_view src, CharsetCode targetCode) {
    if (charsetCodeMap.at(targetCode).isVietnameseLocalCharset) {
        viet::Init();
        return viet::ConvertFromUtf16LE(src, CharsetCodeToVietEncoding(targetCode));
    }

    // 从code转换到icu的字符集名称
    auto icuCharsetName = ToICUCharsetName(targetCode);

    UErrorCode err = U_ZERO_ERROR;

    // 打开转换器
    unique_ptr<UConverter, function<void(UConverter *)>> conv(ucnv_open(to_string(icuCharsetName).c_str(), &err),
                                                              [](UConverter *p) {
                                                                  ucnv_close(p);
                                                              });
    DealWithUCNVError(err);

    int32_t destCap = src.size() * sizeof(UChar) + 2;
    std::string target(destCap, '\0');

    FromUFLAGContext *context = new FromUFLAGContext; // 由回调函数管理生命期

    /* Set our special callback */
    ucnv_setFromUCallBack(conv.get(), flagCB_fromU, context, &(context->subCallback), &(context->subContext), &err);
    DealWithUCNVError(err);

    // 编码
    int retLen;
    while (1) {
        err = U_ZERO_ERROR;
        retLen = ucnv_fromUChars(conv.get(), target.data(), destCap, src.data(), src.size(), &err);
        if (err == U_BUFFER_OVERFLOW_ERROR || err == U_STRING_NOT_TERMINATED_WARNING) {
            destCap = retLen + 6; // 增加一个尾后0的大小：utf-8 单个字符最大占用字节数
            target.resize(destCap);
            continue;
        }
        DealWithUCNVError(err);
        if (err == U_ZERO_ERROR) {
            target.resize(retLen);
            break;
        }
    }

    // 如果存在不能转换的字符，那么抛出异常
    if (!context->unassigned.empty()) {
        context->unassigned.push_back(0);
        UChar32 *s = context->unassigned.data();

        // UTF32LE -> UTF16LE
        std::u16string temp =
            Decode(std::string_view(reinterpret_cast<char *>(s), context->unassigned.size() * 4), CharsetCode::UTF32LE);

        // UTF16LE -> UTF8
        std::string ret = Encode(temp, CharsetCode::UTF8);

        throw runtime_error(GetLanguageService().GetUtf8String(StringId::WILL_LOST_CHARACTERS) + ret);
    }

    return target;
}

std::string Convert(std::string_view src, ConvertParam inputParam) {
    // 根据原编码得到Unicode字符串
    std::u16string buf = Decode(src, inputParam.originCode);

    // 如果需要转换换行符
    if (inputParam.doConvertLineBreaks) {
        ChangeLineBreaks(buf, inputParam.targetLineBreak);
    }

    // 转到目标编码
    return Encode(buf, inputParam.targetCode);
}

viet::Encoding CharsetCodeToVietEncoding(CharsetCode code) noexcept {
    return viet::to_encoding(to_utf8(ToViewCharsetName(code)));
}

Core::Core(std::tstring configFileName, CoreInitOption opt) : configFileName(configFileName), opt(opt) {
    // 读ini
    ReadConfigFromFile();

    // 初始化uchardet
    det = unique_ptr<uchardet, std::function<void(uchardet *)>>(uchardet_new(), [](uchardet *det) {
        uchardet_delete(det);
    });

#ifndef NDEBUG
    // =================================
    // ==== will detect memory leak ====
    // UErrorCode err;
    // UEnumeration *allNames = ucnv_openAllNames(&err);
    // while (1) {
    //    auto name = uenum_next(allNames, nullptr, &err);
    //    if (name == nullptr) {
    //        break;
    //    }
    //}
    // ================================
#endif
}

const Configuration &Core::GetConfig() const {
    return config;
}

const std::unique_ptr<uchardet, std::function<void(uchardet *)>> &Core::GetUCharDet() const {
    return det;
}

void Core::SetFilterMode(Configuration::FilterMode mode) {
    config.filterMode = mode;
    WriteConfigToFile();
}

void Core::SetFilterRule(const std::string &rule) {
    config.includeRule = rule;
    WriteConfigToFile();
}

void Core::SetOutputTarget(Configuration::OutputTarget outputTarget) {
    config.outputTarget = outputTarget;
    WriteConfigToFile();
}

void Core::SetOutputDir(const std::string &outputDir) {
    config.outputDir = outputDir;
    WriteConfigToFile();
}

void Core::SetOutputCharset(CharsetCode outputCharset) {
    config.outputCharset = outputCharset;
    WriteConfigToFile();
}

void Core::SetLineBreaks(LineBreaks lineBreak) {
    config.lineBreak = lineBreak;
    WriteConfigToFile();
}

void Core::SetLanguage(const std::string &language) noexcept {
    config.language = language;
    WriteConfigToFile();
}

void Core::SetEnableConvertLineBreak(bool enableLineBreaks) {
    config.enableConvertLineBreaks = enableLineBreaks;
    WriteConfigToFile();
}

void RemoveASCII(std::vector<char> &data) noexcept {
    auto itor = std::stable_partition(data.begin(), data.end(), [](char c) {
        return (c & 0b10000000);
    });

    data.erase(itor, data.end());
}

Core::AddItemResult Core::AddItem(const std::tstring &filename, const std::unordered_set<std::tstring> &filterDotExts) {
    // 如果是只包括指定后缀的模式，且文件后缀不符合，则忽略掉，且不提示
    if (GetConfig().filterMode == Configuration::FilterMode::ONLY_SOME_EXTANT) {
        auto ext = GetExtend(filename);
        auto dotExt = TEXT(".") + tolower(ext);

        if (filterDotExts.find(dotExt) == filterDotExts.end()) {
            return {};
        }
    }

    // 如果重复了
    if (listFileNames.find(filename) != listFileNames.end()) {
        throw runtime_error(GetLanguageService().GetUtf8String(StringId::ADD_REDUNDANTLY));
    }

    // 读入文件，只读入部分。因为读入大文件会占用太长时间。
    auto [buf, bufSize] = ReadFileToBuffer(filename, tryReadSize);

    // 识别字符集
    auto charsetCode = DetectEncoding(det.get(), buf.get(), bufSize);

    std::u16string content;

    switch (charsetCode) {
    case CharsetCode::EMPTY:
        break;

    // 如果没有识别出编码集
    case (CharsetCode::UNKNOWN): {
        switch (GetConfig().filterMode) {
        // 如果是智能模式或者后缀模式，不添加这个文件，但要抛出异常，让UI弹出提示
        case Configuration::FilterMode::SMART:
        case Configuration::FilterMode::ONLY_SOME_EXTANT:
            throw io_error_ignore();
        // 如果是不过滤模式
        case Configuration::FilterMode::NO_FILTER: {
            // 强行添加

            auto fileSize = GetFileSize(filename);

            // 成功添加
            listFileNames.insert(filename);

            return AddItemResult{false, fileSize, charsetCode, LineBreaks::UNKNOWN, {}};
        }
        default:
            assert(0);
        }
        break;
    }
    default:
        // 根据uchardet得出的字符集解码
        content = Decode(std::string_view(buf.get(), std::min(64, static_cast<int>(bufSize))), charsetCode);
    }

    auto fileSize = GetFileSize(filename);

    auto charsetName = ToViewCharsetName(charsetCode);

    // 重新读入整个文件，因为之前只读入了部分，换行符可能判断不彻底
    if (bufSize < fileSize) {
        std::tie(buf, bufSize) = ReadFileToBuffer(filename);
    }
    auto wholeUtfStr = Decode(std::string_view(buf.get(), bufSize), charsetCode);

    // 检查换行符
    auto lineBreak = GetLineBreaks(wholeUtfStr.data(), wholeUtfStr.size());

    // 到达这里不会再抛异常了

    // 成功添加
    listFileNames.insert(filename);

    return AddItemResult{false, fileSize, charsetCode, lineBreak, content};
}

void Core::SpecifyItemCharset(int index, const std::tstring &filename, CharsetCode charsetCode) {
    assert(listFileNames.find(filename) != listFileNames.end());

    // 读入文件，只读入部分。因为读入大文件会占用太长时间。
    auto [buf, bufSize] = ReadFileToBuffer(filename, tryReadSize);

    auto fileSize = GetFileSize(filename);
    auto fileSizeStr = FileSizeToTString(fileSize);

    auto charsetName = ToViewCharsetName(charsetCode);

    // 重新读入整个文件，因为之前只读入了部分，换行符可能判断不彻底
    if (bufSize < fileSize) {
        std::tie(buf, bufSize) = ReadFileToBuffer(filename);
    }
    auto wholeUtfStr = Decode(std::string_view(buf.get(), bufSize), charsetCode);
    auto lineBreak = GetLineBreaks(wholeUtfStr.data(), wholeUtfStr.size());

    auto lineBreakStr = lineBreaksMap.at(lineBreak);

    // 到达这里不会再抛异常了

    // 通知UI新增条目
    auto stringPiece = Decode(std::string_view(buf.get(), std::min(64, static_cast<int>(bufSize))), charsetCode);
    opt.fnUIUpdateItem(index, filename, fileSizeStr, charsetName, lineBreakStr, stringPiece);
}

void Core::RemoveItem(const std::tstring &filename) {
    listFileNames.erase(filename);
}

void Core::Clear() {
    listFileNames.clear();
}

Core::ConvertFileResult Core::Convert(const std::tstring &inputFilename, CharsetCode originCode,
                                      LineBreaks originLineBreak) noexcept {
    CharsetCode targetCode = config.outputCharset;

    ConvertFileResult ret;
    try {
        ret.outputFileName = inputFilename;
        ret.targetLineBreaks = originLineBreak;
        ret.outputFileSize = GetFileSize(inputFilename);

        // 计算目标文件名
        if (GetConfig().outputTarget != Configuration::OutputTarget::ORIGIN) {
            // 纯文件名
            auto pureFileName = GetNameAndExt(ret.outputFileName);

            ret.outputFileName = utf8_to_wstring(GetConfig().outputDir) + TEXT("\\") + pureFileName;
        }

        // 原编码集
        if (originCode == CharsetCode::UNKNOWN) {
            throw runtime_error(GetLanguageService().GetUtf8String(StringId::NO_DETECTED_ENCODING));
        }

        // 返回原字符集和目标字符集的条件为不需要转换的情形
        auto CharsetNeedNotConvert = [&]() -> bool {
            // 原编码和目标编码一样
            if (originCode == targetCode)
                return true;

            // 原来是空文件，且目标编码不需要写入BOM
            if (originCode == CharsetCode::EMPTY && !HasBom(targetCode))
                return true;
            return false;
        };

        // 判断不需要转换的条件，或者是需要复制的情形，直接不转换或者复制
        // 返回true则不需要实际转换了
        auto CheckNothingOrCopy = [&]() -> bool {
            if (CharsetNeedNotConvert() &&
                // 不转换换行符，或者新换行符和原来的换行符一样
                (GetConfig().enableConvertLineBreaks == false || GetConfig().lineBreak == originLineBreak)) {
                // 那么只需要考虑是否原位转换，原位转换的话什么也不做，否则复制过去

                // 如果不是原位置转换，复制过去
                if (GetConfig().outputTarget == Configuration::OutputTarget::TO_DIR) {
                    bool ok = CopyFile(inputFilename.c_str(), ret.outputFileName.c_str(), false);
                    if (!ok) {
                        throw runtime_error(GetLanguageService().GetUtf8String(StringId::FAILED_TO_WRITE_FILE) +
                                            to_utf8(ret.outputFileName));
                    }
                }

                // 原位转换，什么也不做
                return true;
            }

            return false;
        };

        do {
            if (CheckNothingOrCopy())
                break;

            // 前后编码不一样
            auto filesize = GetFileSize(inputFilename);

            // 暂时不做分块转换 TODO

            {
                // 读二进制
                auto [raw, rawSize] = ReadFileToBuffer(inputFilename);

                if (rawSize >= std::numeric_limits<int>::max()) {
                    throw runtime_error(GetLanguageService().GetUtf8String(StringId::FILE_SIZE_OUT_OF_LIMIT));
                }

                // 根据BOM偏移
                const char *rawStart = raw.get();

                // 如果需要抹掉BOM，则把起始位置设置到BOM之后，确保UChar[]不带BOM
                if (HasBom(originCode) && !HasBom(targetCode)) {
                    auto bomSize = BomSize(originCode);
                    rawStart += bomSize;
                    rawSize -= bomSize;
                }

                ConvertParam param;
                param.originCode = originCode;
                param.targetCode = targetCode;
                param.doConvertLineBreaks =
                    GetConfig().enableConvertLineBreaks && GetConfig().lineBreak != originLineBreak;
                param.targetLineBreak = GetConfig().lineBreak;

                // 转到目标编码
                auto outputBuf = ::Convert(std::string_view(rawStart, rawSize), param);

                if (param.doConvertLineBreaks) {
                    ret.targetLineBreaks = param.targetLineBreak;
                }

                ret.outputFileSize = 0;

                // 写入文件
                FILE *fp = nullptr;
                errno_t err = _tfopen_s(&fp, ret.outputFileName.c_str(), TEXT("wb"));
                if (fp == nullptr) {
                    throw runtime_error(GetLanguageService().GetUtf8String(StringId::FAILED_TO_OPEN_FILE) +
                                        to_utf8(ret.outputFileName));
                }
                unique_ptr<FILE, function<void(FILE *)>> upFile(fp, [](FILE *fp) {
                    fclose(fp);
                });

                // 如果需要额外加上BOM，先写入BOM
                if (!HasBom(originCode) && HasBom(targetCode)) {
                    auto bomData = GetBomData(targetCode);

                    // 写入BOM
                    size_t wrote = fwrite(bomData, BomSize(targetCode), 1, fp);
                    ret.outputFileSize += BomSize(targetCode);
                    if (wrote != 1) {
                        throw runtime_error(GetLanguageService().GetUtf8String(StringId::FAILED_TO_WRITE_FILE) +
                                            to_utf8(ret.outputFileName));
                    }
                }

                // 写入正文
                size_t wrote = fwrite(outputBuf.data(), outputBuf.size(), 1, fp);
                ret.outputFileSize += outputBuf.size();
                if (outputBuf.size() != 0 && wrote != 1) {
                    throw runtime_error(GetLanguageService().GetUtf8String(StringId::FAILED_TO_WRITE_FILE) +
                                        to_utf8(ret.outputFileName));
                }
            }

        } while (0);

    } catch (const std::runtime_error &err) {
        // 这个文件失败了
        ret.errInfo = err.what();
    }

    // 这个文件成功了
    return ret;
}

void Core::ReadConfigFromFile() {
    if (!GetFileExists(configFileName)) {
        return;
    }

    std::ifstream ifs(to_string(configFileName));
    if (!ifs) {
        throw std::runtime_error("open file fail: " + to_string(configFileName));
    }

    nlohmann::json j = nlohmann::json::parse(ifs);
    from_json(j, config);

    ifs.close();
}

void Core::WriteConfigToFile() {
    std::ofstream ofs(to_string(configFileName));
    if (!ofs) {
        throw std::runtime_error("write file fail: " + to_string(configFileName));
    }

    nlohmann::json j;
    to_json(j, config);

    ofs << std::setw(4) << j;
    ofs.close();
}

// UINT Configuration::ToWinCodePage(OutputCharset charset)
//{
//	switch (charset)
//	{
//	case OutputCharset::UTF8:
//		return CP_UTF8;
//	case OutputCharset::GB18030:
//		return CP_GB18030;
//	}
//}
