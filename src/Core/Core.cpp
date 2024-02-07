#include "Core.h"

#include "Language.h"

#include <FileFunction.h>

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <compact_enc_det/compact_enc_det.h>

#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

constexpr uint64_t tryReadSize = 100Ui64 * KB;

/*
 * @exception runtime_error ucnv出错。code
 */
void DealWithUCNVError(UErrorCode err) {
    switch (err) {
    case U_ZERO_ERROR:
        break;
    case U_AMBIGUOUS_ALIAS_WARNING: // windows-1252 时会出这个，暂时忽略
        break;
    case U_INVALID_CHAR_FOUND:
        throw runtime_error(GetLanguageService().GetUtf8String(StringId::INVALID_CHARACTERS));
    default:
        throw runtime_error("ucnv error. code=" + to_string(err));
        break;
    }
}

tuple<unique_ptr<UChar[]>, int> Decode(const char *str, int len, CharsetCode code) {
    if (code == CharsetCode::EMPTY) {
        return {unique_ptr<UChar[]>(new UChar[0]), 0};
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

    int32_t cap = len + 1;
    unique_ptr<UChar[]> target(new UChar[cap]);

    // 解码
    int retLen = ucnv_toUChars(conv.get(), target.get(), cap, str, len, &err);
    DealWithUCNVError(err);

    return make_tuple<unique_ptr<UChar[]>, int32_t>(std::move(target), std::move(retLen));
}

// below copied from https://github.com/unicode-org/icu/blob/main/icu4c/source/samples/ucnv/flagcb.c

/* The structure of a FromU Flag context.
   (conceivably there could be a ToU Flag Context) */
struct FromUFLAGContext {
    UConverterFromUCallback subCallback;
    const void *subContext;
    std::vector<UChar32> unassigned; // 是否出现了不能转换的字符
    FromUFLAGContext() : subCallback(nullptr), subContext(nullptr), unassigned(false) {}
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

std::tuple<std::unique_ptr<char[]>, int> Encode(const std::unique_ptr<UChar[]> &buf, int bufSize,
                                                CharsetCode targetCode) {
    // 从code转换到icu的字符集名称
    auto icuCharsetName = ToICUCharsetName(targetCode);

    UErrorCode err = U_ZERO_ERROR;

    // 打开转换器
    unique_ptr<UConverter, function<void(UConverter *)>> conv(ucnv_open(to_string(icuCharsetName).c_str(), &err),
                                                              [](UConverter *p) {
                                                                  ucnv_close(p);
                                                              });
    DealWithUCNVError(err);

    int32_t destCap = bufSize * sizeof(UChar) + 2;
    unique_ptr<char[]> target(new char[destCap]);

    FromUFLAGContext *context = new FromUFLAGContext; // 由回调函数管理生命期

    /* Set our special callback */
    ucnv_setFromUCallBack(conv.get(), flagCB_fromU, context, &(context->subCallback), &(context->subContext), &err);
    DealWithUCNVError(err);

    // 编码
    int retLen;
    while (1) {
        err = U_ZERO_ERROR;
        retLen = ucnv_fromUChars(conv.get(), target.get(), destCap, buf.get(), bufSize, &err);
        if (err == U_BUFFER_OVERFLOW_ERROR || err == U_STRING_NOT_TERMINATED_WARNING) {
            destCap = retLen + 6; // 增加一个尾后0的大小：utf-8 单个字符最大占用字节数
            target.reset(new char[destCap]);
            continue;
        }
        DealWithUCNVError(err);
        if (err == U_ZERO_ERROR) {
            break;
        }
    }

    if (!context->unassigned.empty()) {
        context->unassigned.push_back(0);
        auto s = context->unassigned.data();

        auto [buf, bufSize] = Decode(reinterpret_cast<char *>(s), context->unassigned.size() * 4, CharsetCode::UTF32LE);

        auto [ret, retSize] = Encode(buf, bufSize, CharsetCode::UTF8);

        throw runtime_error(GetLanguageService().GetUtf8String(StringId::WILL_LOST_CHARACTERS) + ret.get());
    }

    return make_tuple(std::move(target), retLen);
}

Core::Core(std::tstring configFileName, CoreInitOption opt) : configFileName(configFileName), opt(opt) {
    // 读ini
    ReadConfigFromFile();

    // 初始化uchardet
    det = unique_ptr<uchardet, std::function<void(uchardet *)>>(uchardet_new(), [](uchardet *det) {
        uchardet_delete(det);
    });

#ifndef NDEBUG
    UErrorCode err;
    auto allNames = ucnv_openAllNames(&err);
    while (1) {
        auto name = uenum_next(allNames, nullptr, &err);
        if (name == nullptr) {
            break;
        }
    }
#endif
}

const Configuration &Core::GetConfig() const {
    return config;
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

std::tuple<std::string, int> DetectByUCharDet(uchardet *det, const char *buf, int bufSize) {
    // 用uchardet判定字符集
    uchardet_reset(det);
    int ret = uchardet_handle_data(det, buf, bufSize);
    switch (ret) {
    case HANDLE_DATA_RESULT_NEED_MORE_DATA:
    case HANDLE_DATA_RESULT_DETECTED:
        break;
    case HANDLE_DATA_RESULT_ERROR:
        throw runtime_error("uchardet fail");
    }

    uchardet_data_end(det);

    // 得到uchardet的识别结果
    string charset = uchardet_get_charset(det);

    float confidence = uchardet_get_confidence(det);

    return {charset, static_cast<int>(confidence * 100)};
}

std::tuple<std::string, int> DetectByUCSDet(const char *buf, int bufSize) {
    UErrorCode status = U_ZERO_ERROR;
    UCharsetDetector *csd = ucsdet_open(&status);
    DealWithUCNVError(status);

    ucsdet_setText(csd, buf, bufSize, &status);
    DealWithUCNVError(status);

    const UCharsetMatch *ucm = ucsdet_detect(csd, &status);
    DealWithUCNVError(status);

    int32_t confidence = ucsdet_getConfidence(ucm, &status);
    DealWithUCNVError(status);

    const char *name = ucsdet_getName(ucm, &status);
    DealWithUCNVError(status);

    return {name, confidence};
}

std::unordered_set<CharsetCode> DetectByMine(const char *buf, int bufSize) {
    std::unordered_set<CharsetCode> ret;
    for (int i = static_cast<int>(CharsetCode::UTF8); i <= static_cast<int>(CharsetCode::ISO_8859_1); ++i) {
        CharsetCode tryCode = static_cast<CharsetCode>(i);

        try {
            auto [decStr, decLen] = Decode(buf, bufSize, tryCode);

            Encode(decStr, decLen, tryCode);

            ret.insert(tryCode);
        } catch (...) {}
    }
    return ret;
}

/**
 * @exception runtime_error 如果CED中定义的名称在CharsetCode中没有定义，将抛出异常
 */
std::tuple<CharsetCode, bool> DetectByCED(const char *buf, int len) {
    int bytes_consumed;
    bool is_reliable;
    Encoding encoding = CompactEncDet::DetectEncoding(buf, len, nullptr, // URL hint
                                                      nullptr,           // HTTP hint
                                                      nullptr,           // Meta hint
                                                      UNKNOWN_ENCODING, UNKNOWN_LANGUAGE, CompactEncDet::WEB_CORPUS,
                                                      false, // Include 7-bit encodings?
                                                      &bytes_consumed, &is_reliable);

    // 如果认为是二进制文件，那么取信它
    if (encoding == Encoding::BINARYENC) {
        return {CharsetCode::UNKNOWN, true};
    }

    // 这里如果CED识别出的名字在CharsetCode中没有定义，将抛出异常
    CharsetCode code;
    try {
        code = ToCharsetCode(string_to_wstring(EncodingName(encoding)));

    } catch (const std::runtime_error &err) {
        if (is_reliable) {
            throw;
        }
        return {CharsetCode::UNKNOWN, true};
    }
    return {code, is_reliable};
}

void RemoveASCII(std::vector<char> &data) noexcept {
    auto itor = std::stable_partition(data.begin(), data.end(), [](char c) {
        return (c & 0b10000000);
    });

    data.erase(itor, data.end());
}

CharsetCode ToCharsetCodeFinal(std::string charsetStr, const char *buf, int bufSize) {

    // filter
    CharsetCode code;
    if (charsetStr == "UTF-8") {
        // 区分有无BOM
        if (bufSize >= sizeof(UTF8BOM_DATA) && memcmp(buf, UTF8BOM_DATA, sizeof(UTF8BOM_DATA)) == 0) {
            code = CharsetCode::UTF8BOM;
        } else {
            code = CharsetCode::UTF8;
        }
    } else if (charsetStr == "UTF-16LE") {
        // 区分有无BOM
        if (bufSize >= sizeof(UTF16LEBOM_DATA) && memcmp(buf, UTF16LEBOM_DATA, sizeof(UTF16LEBOM_DATA)) == 0) {
            code = CharsetCode::UTF16LEBOM;
        } else {
            code = CharsetCode::UTF16LE;
        }
    } else if (charsetStr == "UTF-16BE") {
        // 区分有无BOM
        if (bufSize >= sizeof(UTF16BEBOM_DATA) && memcmp(buf, UTF16BEBOM_DATA, sizeof(UTF16BEBOM_DATA)) == 0) {
            code = CharsetCode::UTF16BEBOM;
        } else {
            code = CharsetCode::UTF16BE;
        }
    } else if (charsetStr == "") // 没识别出来
    {
        code = CharsetCode::UNKNOWN;
    } else {
        try {
            code = ToCharsetCode(to_wstring(charsetStr));
        } catch (...) {
            string info = MyPrintf(GetLanguageService().GetUtf8String(StringId::INVALID_CHARACTERS),
                                   charsetStr.size() + 1LL, charsetStr.c_str());
            throw runtime_error(info);
        }
    }
    return code;
}

CharsetCode Core::DetectEncodingPlain(const char *buf, int bufSize, int times) const {
    if (bufSize == 0) {
        return CharsetCode::EMPTY;
    }

    auto [ucsdetResult, ucsdetConfidence] = DetectByUCSDet(buf, bufSize);

    if (ucsdetConfidence >= 95 && ucsdetResult.find("UTF") != string::npos) {
        // ucsdet如果判定为UTF-8/UTF-16BE|LE等，那么相信它
        return ToCharsetCodeFinal(ucsdetResult, buf, bufSize);
    }

    auto [uchardetResult, uchardetConfidence] = DetectByUCharDet(det.get(), buf, bufSize);
    if (uchardetConfidence >= 95) {
        // uchardet如果有95及以上的信心，那么直接相信它
        return ToCharsetCodeFinal(uchardetResult, buf, bufSize);
    }

    // ucsdet和uchardet都没把握
    return CharsetCode::UNKNOWN;

    /*
    // update: CED的探测结果不稳定，而且可能出现识别出错但reliable==true的情况，
    //          暂时先不用CED了。

    auto [cedResult, reliable] = DetectByCED(buf, bufSize);
    if (reliable) {
        return cedResult;
    }

    if (times > 0) {
        // 第2次尝试失败，认命了
        return CharsetCode::UNKNOWN;
    }

    // 裁掉ASCII，再战！
    std::vector<char> data(bufSize);
    std::memcpy(data.data(), buf, bufSize);

    RemoveASCII(data);
    return DetectEncodingPlain(data.data(), data.size(), 1);

    */
}

CharsetCode Core::DetectEncoding(const char *buf, int bufSize) const {
    return DetectEncodingPlain(buf, bufSize, 0);
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
    auto charsetCode = DetectEncoding(buf.get(), bufSize);

    std::unique_ptr<UChar[]> content;
    int64_t contentSize;

    switch (charsetCode) {
    case CharsetCode::EMPTY:
        std::tie(content, contentSize) = std::make_tuple(std::unique_ptr<UChar[]>(new UChar[1]{L'\0'}), 0);
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

            return AddItemResult{false, fileSize, charsetCode, LineBreaks::UNKNOWN, L""};
        }
        default:
            assert(0);
        }
        break;
    }
    default:
        // 根据uchardet得出的字符集解码
        std::tie(content, contentSize) = Decode(buf.get(), std::min(64, static_cast<int>(bufSize)), charsetCode);
    }

    auto fileSize = GetFileSize(filename);

    auto charsetName = ToViewCharsetName(charsetCode);

    // 重新读入整个文件，因为之前只读入了部分，换行符可能判断不彻底
    if (bufSize < fileSize) {
        std::tie(buf, bufSize) = ReadFileToBuffer(filename);
    }
    auto [wholeUtfStr, wholeUtfStrSize] = Decode(buf.get(), bufSize, charsetCode);

    // 检查换行符
    auto lineBreak = GetLineBreaks(wholeUtfStr.get(), wholeUtfStrSize);

    // 到达这里不会再抛异常了

    // 成功添加
    listFileNames.insert(filename);

    return AddItemResult{false, fileSize, charsetCode, lineBreak, reinterpret_cast<wchar_t *>(content.get())};
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
    auto [wholeUtfStr, wholeUtfStrSize] = Decode(buf.get(), bufSize, charsetCode);
    auto lineBreak = GetLineBreaks(wholeUtfStr.get(), wholeUtfStrSize);

    auto lineBreakStr = lineBreaksMap.at(lineBreak);

    // 到达这里不会再抛异常了

    // 通知UI新增条目
    auto partContentAndSize = Decode(buf.get(), std::min(64, static_cast<int>(bufSize)), charsetCode);
    opt.fnUIUpdateItem(index, filename, fileSizeStr, charsetName, lineBreakStr,
                       reinterpret_cast<wchar_t *>(std::get<0>(partContentAndSize).get()));
}

void Core::RemoveItem(const std::tstring &filename) {
    listFileNames.erase(filename);
}

void Core::Clear() {
    listFileNames.clear();
}

Core::ConvertResult Core::Convert(const std::tstring &inputFilename, CharsetCode originCode,
                                  LineBreaks originLineBreak) noexcept {
    CharsetCode targetCode = config.outputCharset;

    ConvertResult ret;
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

                // 根据原编码得到Unicode字符串
                auto [buf, bufLen] = Decode(rawStart, static_cast<int>(rawSize), originCode);

                // 如果需要转换换行符
                if (GetConfig().enableConvertLineBreaks && GetConfig().lineBreak != originLineBreak) {
                    ChangeLineBreaks(buf, bufLen, GetConfig().lineBreak);
                    ret.targetLineBreaks = GetConfig().lineBreak;
                }

                // 转到目标编码
                auto [outputBuf, outputBufSize] = Encode(buf, bufLen, targetCode);
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
                size_t wrote = fwrite(outputBuf.get(), outputBufSize, 1, fp);
                ret.outputFileSize += outputBufSize;
                if (outputBufSize != 0 && wrote != 1) {
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
