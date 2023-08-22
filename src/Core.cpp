#include "Core.h"

#include <FileFunction.h>

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>

#include <stdexcept>
#include <algorithm>

#ifdef _DEBUG
#include <iostream>
#endif

#undef min
#undef max

using namespace std;

std::unordered_set<CharsetCode> Configuration::normalCharset = {CharsetCode::UTF8, CharsetCode::UTF8BOM,
                                                                CharsetCode::GB18030};

std::tstring ToViewCharsetName(CharsetCode code) {
    return charsetCodeMap.at(code).viewName;
}

CharsetCode ToCharsetCode(const std::tstring &name) {
    auto it =
        std::find_if(charsetCodeMap.begin(), charsetCodeMap.end(), [&](const std::pair<CharsetCode, MyCharset> &pr) {
            return pr.second.viewName == name;
        });
    if (it != charsetCodeMap.end()) {
        return it->first;
    }
    it = std::find_if(charsetCodeMap.begin(), charsetCodeMap.end(), [&](const std::pair<CharsetCode, MyCharset> &pr) {
        return pr.second.icuName == to_string(name);
    });
    if (it != charsetCodeMap.end()) {
        return it->first;
    }
    it = std::find_if(charsetCodeMap.begin(), charsetCodeMap.end(), [&](const std::pair<CharsetCode, MyCharset> &pr) {
        auto ititEnd = pr.second.icuNames.end();
        auto itit = std::find(pr.second.icuNames.begin(), pr.second.icuNames.end(), to_string(name));
        return itit != ititEnd;
    });
    if (it != charsetCodeMap.end()) {
        return it->first;
    }
    throw runtime_error("unsupported: " + to_string(name));
}

bool HasBom(CharsetCode code) {
    switch (code) {
    case CharsetCode::UTF8BOM:
    case CharsetCode::UTF16LEBOM:
    case CharsetCode::UTF16BEBOM:
        return true;
    }
    return false;
}

const char *GetBomData(CharsetCode code) {
    switch (code) {
    case CharsetCode::UTF8BOM:
        return UTF8BOM_DATA;
    case CharsetCode::UTF16LEBOM:
        return UTF16LEBOM_DATA;
    case CharsetCode::UTF16BEBOM:
        return UTF16BEBOM_DATA;
    }
    return nullptr;
}

int BomSize(CharsetCode code) {
    switch (code) {
    case CharsetCode::UTF8BOM:
        return sizeof(UTF8BOM_DATA);
    case CharsetCode::UTF16LEBOM:
        return sizeof(UTF16LEBOM_DATA);
    case CharsetCode::UTF16BEBOM:
        return sizeof(UTF16BEBOM_DATA);
    }
    return 0;
}

CharsetCode CheckBom(char *buf, int bufSize) {
    if (bufSize >= sizeof(UTF8BOM_DATA) && memcmp(buf, UTF8BOM_DATA, sizeof(UTF8BOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    if (bufSize >= sizeof(UTF16LEBOM_DATA) && memcmp(buf, UTF16LEBOM_DATA, sizeof(UTF16LEBOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    if (bufSize >= sizeof(UTF16BEBOM_DATA) && memcmp(buf, UTF16BEBOM_DATA, sizeof(UTF16BEBOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    if (bufSize >= sizeof(UTF32LEBOM_DATA) && memcmp(buf, UTF32LEBOM_DATA, sizeof(UTF32LEBOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    if (bufSize >= sizeof(UTF32BEBOM_DATA) && memcmp(buf, UTF32BEBOM_DATA, sizeof(UTF32BEBOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    return CharsetCode::UNKNOWN;
}

std::string ToICUCharsetName(CharsetCode code) {
    return to_string(charsetCodeMap.at(code).icuName);
}

/*
 * @exception runtime_error ucnv出错。code
 */
void DealWithUCNVError(UErrorCode err) {
    switch (err) {
    case U_ZERO_ERROR:
        break;
    case U_AMBIGUOUS_ALIAS_WARNING: // windows-1252 时会出这个，暂时忽略
        break;
    default:
        throw runtime_error("ucnv出错。code=" + to_string(err));
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
    UBool unassigned; // 是否出现了不能转换的字符
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
        ctx->unassigned = true;
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
        if (err == U_BUFFER_OVERFLOW_ERROR) {
            destCap = retLen + 6; // 增加一个尾后0的大小：utf-8 单个字符最大占用字节数
            target.reset(new char[destCap]);
            continue;
        }
        DealWithUCNVError(err);
        if (err == U_ZERO_ERROR) {
            break;
        }
    }

    if (context->unassigned) {
        throw runtime_error("转换到目标编码会丢失字符。");
    }

    return make_tuple(std::move(target), retLen);
}

Configuration::LineBreaks GetLineBreaks(const UChar *buf, int len) {
    Configuration::LineBreaks ans = Configuration::LineBreaks::EMPTY;
    for (int i = 0; i < len;) {
        const UChar &c = buf[i];
        if (c == UChar(u'\r')) {
            // \r\n
            if (i < len && buf[i + 1] == UChar(u'\n')) {
                if (ans == Configuration::LineBreaks::EMPTY) {
                    ans = Configuration::LineBreaks::CRLF;
                } else {
                    if (ans != Configuration::LineBreaks::CRLF) {
                        ans = Configuration::LineBreaks::MIX;
                        return ans;
                    }
                }
                i += 2;
                continue;
            }

            // \r
            if (ans == Configuration::LineBreaks::EMPTY) {
                ans = Configuration::LineBreaks::CR;
            } else {
                if (ans != Configuration::LineBreaks::CR) {
                    ans = Configuration::LineBreaks::MIX;
                    return ans;
                }
            }
            i++;
            continue;
        }

        // \n
        if (c == UChar(u'\n')) {
            if (ans == Configuration::LineBreaks::EMPTY) {
                ans = Configuration::LineBreaks::LF;
            } else {
                if (ans != Configuration::LineBreaks::LF) {
                    ans = Configuration::LineBreaks::MIX;
                    return ans;
                }
            }
            i++;
            continue;
        }

        i++;
    }
    return ans;
}

void Test_GetLineBreaks() {
    u16string ws;

    ws = u"\r";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::CR);
    ws = u"\r\r";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::CR);
    ws = u"\r00\r";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::CR);
    ws = u"\r\r\n";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::MIX);

    ws = u"\n";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::LF);
    ws = u"\n\r";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::MIX);
    ws = u"\n\n";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::LF);
    ws = u"\n\n\r";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::MIX);

    ws = u"\r\n";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::CRLF);
    ws = u"\r\n\n";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::MIX);
    ws = u"\r\n\r";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::MIX);

    ws = u"\n\r";
    assert(GetLineBreaks(ws.c_str(), ws.length()) == Configuration::LineBreaks::MIX);
}

void ChangeLineBreaks(std::unique_ptr<UChar[]> &buf, int &len, Configuration::LineBreaks targetLineBreak) {
    vector<UChar> out;
    out.reserve(len);

    vector<UChar> lineBreak;
    switch (targetLineBreak) {
    case Configuration::LineBreaks::CRLF:
        lineBreak = {u'\r', u'\n'};
        break;
    case Configuration::LineBreaks::LF:
        lineBreak = {u'\n'};
        break;
    case Configuration::LineBreaks::CR:
        lineBreak = {u'\r'};
        break;
    }

    for (int i = 0; i < len;) {
        UChar &c = buf.get()[i];
        if (c == UChar(u'\r')) {
            // \r\n
            if (i < len && buf.get()[i + 1] == UChar(u'\n')) {
                out.insert(out.end(), lineBreak.begin(), lineBreak.end());
                i += 2;
                continue;
            }

            // \r
            out.insert(out.end(), lineBreak.begin(), lineBreak.end());
            i++;
            continue;
        }

        if (c == UChar(u'\n')) {
            out.insert(out.end(), lineBreak.begin(), lineBreak.end());
            i++;
            continue;
        }

        out.push_back(c);
        i++;
    }

    if (out.size() >= std::numeric_limits<int>::max()) {
        throw runtime_error("生成文件大小超出限制");
    }

    int outLen = static_cast<int>(out.size());
    buf.reset(new UChar[outLen]);
    memcpy(buf.get(), out.data(), out.size() * sizeof(UChar));
    len = outLen;

    return;
}

Core::Core(std::tstring iniFileName, CoreInitOption opt) : iniFileName(iniFileName), opt(opt) {
    // 读ini
    ReadFromIni();

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
        cout << name << endl;
    }
#endif
}

const Configuration &Core::GetConfig() const {
    return config;
}

void Core::SetFilterMode(Configuration::FilterMode mode) {
    config.filterMode = mode;
    WriteToIni();
}

void Core::SetFilterRule(std::tstring rule) {
    config.includeRule = rule;
    WriteToIni();
}

void Core::SetOutputTarget(Configuration::OutputTarget outputTarget) {
    config.outputTarget = outputTarget;
    WriteToIni();
}

void Core::SetOutputDir(std::tstring outputDir) {
    config.outputDir = outputDir;
    WriteToIni();
}

void Core::SetOutputCharset(CharsetCode outputCharset) {
    config.outputCharset = outputCharset;
    WriteToIni();
}

void Core::SetLineBreaks(Configuration::LineBreaks lineBreak) {
    config.lineBreak = lineBreak;
    WriteToIni();
}

void Core::SetEnableConvertLineBreak(bool enableLineBreaks) {
    config.enableConvertLineBreaks = enableLineBreaks;
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
            string info = "暂不支持：";
            info += charsetStr;
            info += "，请联系作者。";
            throw runtime_error(info);
        }
    }
    return code;
}

std::tuple<CharsetCode, std::unique_ptr<UChar[]>, int32_t> Core::GetEncoding(const char *buf, int bufSize) const {
    if (bufSize == 0) {
        return {CharsetCode::EMPTY, unique_ptr<UChar[]>(new UChar[1]{L'\0'}), 0};
    }

    bufSize = std::min(bufSize, static_cast<int>(100 * KB));

    auto [ucsdetResult, ucsdetConfidence] = DetectByUCSDet(buf, bufSize);
    { tcout << TEXT("ucsdet: ") << to_tstring(ucsdetResult) << TEXT(": ") << ucsdetConfidence << endl; }
    auto [uchardetResult, uchardetConfidence] = DetectByUCharDet(det.get(), buf, bufSize);
    { std::tcout << TEXT("uchardet: ") << to_tstring(uchardetResult) << TEXT(": ") << uchardetConfidence << endl; }

    CharsetCode code = CharsetCode::UNKNOWN;

    if (ucsdetConfidence >= 95 && ucsdetResult.find("UTF") != string::npos) {
        // ucsdet如果判定为UTF-8/UTF-16BE|LE等，那么相信它
        code = ToCharsetCodeFinal(ucsdetResult, buf, bufSize);
    } else if (uchardetConfidence >= 95) {
        // uchardet如果有95及以上的信心，那么直接相信它
        code = ToCharsetCodeFinal(uchardetResult, buf, bufSize);
    } else {
        auto codes = DetectByMine(buf, bufSize);
        int n = 10;
    }

    if (code == CharsetCode::UNKNOWN) {
        return make_tuple(code, nullptr, 0);
    }

    // 根据uchardet得出的字符集解码
    auto partContentAndSize = Decode(buf, std::min(64, static_cast<int>(bufSize)), code);

    return make_tuple(code, std::move(get<0>(partContentAndSize)), get<1>(partContentAndSize));
}

void Core::AddItem(const std::tstring &filename, const std::unordered_set<std::tstring> &filterDotExts) {
    // 如果是只包括指定后缀的模式，且文件后缀不符合，则忽略掉，且不提示
    if (GetConfig().filterMode == Configuration::FilterMode::ONLY_SOME_EXTANT &&
        filterDotExts.find(TEXT(".") + GetExtend(filename)) == filterDotExts.end()) {
        return;
    }

    // 如果重复了
    if (listFileNames.find(filename) != listFileNames.end()) {
        throw runtime_error("重复添加");
        return; // 不重复添加了
    }

    std::tcout << filename << TEXT(": ") << endl;

    auto [buf, bufSize] = ReadFileToBuffer(filename);

    // 识别字符集
    auto [charsetCode, content, contentSize] = GetEncoding(buf.get(), bufSize);

    // 如果是智能模式，且没有识别出编码集，则忽略掉，但要提示
    if (GetConfig().filterMode == Configuration::FilterMode::SMART && charsetCode == CharsetCode::UNKNOWN) {
        throw io_error_ignore();
        return;
    }

    auto fileSizeStr = FileSizeToTString(GetFileSize(filename));

    auto charsetName = ToViewCharsetName(charsetCode);

    auto [wholeUtfStr, wholeUtfStrSize] = Decode(buf.get(), bufSize, charsetCode);
    auto lineBreak = GetLineBreaks(wholeUtfStr.get(), wholeUtfStrSize);

    auto lineBreakStr = lineBreaksMap.at(lineBreak);

    // 到达这里不会再抛异常了

    // 通知UI新增条目
    opt.fnUIAddItem(filename, fileSizeStr, charsetName, lineBreakStr, reinterpret_cast<wchar_t *>(content.get()));

    // 成功添加
    listFileNames.insert(filename);
}

void Core::RemoveItem(const std::tstring &filename) {
    listFileNames.erase(filename);
}

void Core::Clear() {
    listFileNames.clear();
}

Core::ConvertResult Core::Convert(const std::tstring &inputFilename, CharsetCode originCode, CharsetCode targetCode,
                                  Configuration::LineBreaks originLineBreak) noexcept {

    ConvertResult ret;
    try {
        ret.outputFileName = inputFilename;
        ret.targetLineBreaks = originLineBreak;
        ret.outputFileSize = GetFileSize(inputFilename);

        // 计算目标文件名
        if (GetConfig().outputTarget != Configuration::OutputTarget::ORIGIN) {
            // 纯文件名
            auto pureFileName = GetNameAndExt(ret.outputFileName);

            ret.outputFileName = GetConfig().outputDir + TEXT("\\") + pureFileName;
        }

        // 原编码集
        if (originCode == CharsetCode::UNKNOWN) {
            throw runtime_error("未探测出编码集");
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
                        throw runtime_error("写入失败：" + to_string(ret.outputFileName));
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
                    throw runtime_error("文件大小超出限制");
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
                    throw runtime_error("打开文件失败：" + to_string(ret.outputFileName));
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
                        throw runtime_error("写入失败：" + to_string(ret.outputFileName));
                    }
                }

                // 写入正文
                size_t wrote = fwrite(outputBuf.get(), outputBufSize, 1, fp);
                ret.outputFileSize += outputBufSize;
                if (outputBufSize != 0 && wrote != 1) {
                    throw runtime_error("写入失败：" + to_string(ret.outputFileName));
                }
            }

        } while (0);

    } catch (runtime_error &e) {
        // 这个文件失败了
        ret.errInfo = to_tstring(e.what());
    }

    // 这个文件成功了
    return ret;
}

void Core::ReadFromIni() {}

void Core::WriteToIni() {}

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
