#include "Core.h"

#include "UCNVHelper.h"

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <compact_enc_det/compact_enc_det.h>

#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

std::tuple<std::string, int> DetectByUCharDet(uchardet *det, const char *buf, std::size_t bufSize) {
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

std::tuple<std::string, int> DetectByUCSDet(const char *buf, int32_t bufSize) {
    UErrorCode status = U_ZERO_ERROR;
    auto csd =
        std::unique_ptr<UCharsetDetector, void (*)(UCharsetDetector *)>(ucsdet_open(&status), [](UCharsetDetector *p) {
            ucsdet_close(p);
        });
    DealWithUCNVError(status);

    ucsdet_setText(csd.get(), buf, bufSize, &status);
    DealWithUCNVError(status);

    const UCharsetMatch *ucm = ucsdet_detect(csd.get(), &status);
    DealWithUCNVError(status);

    int32_t confidence = ucsdet_getConfidence(ucm, &status);
    DealWithUCNVError(status);

    const char *name = ucsdet_getName(ucm, &status);
    DealWithUCNVError(status);

    return {name, confidence};
}

std::unordered_set<CharsetCode> DetectByMine(std::string_view src) {
    std::unordered_set<CharsetCode> ret;
    for (int i = static_cast<int>(CharsetCode::UTF8); i <= static_cast<int>(CharsetCode::ISO_8859_1); ++i) {
        CharsetCode tryCode = static_cast<CharsetCode>(i);

        try {
            auto temp = Decode(src, tryCode);

            Encode(temp, tryCode);

            ret.insert(tryCode);
        } catch (...) {}
    }
    return ret;
}

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
        code = ToCharsetCode(EncodingName(encoding));

    } catch (const std::runtime_error &err) {
        (err);
        if (is_reliable) {
            throw;
        }
        return {CharsetCode::UNKNOWN, true};
    }
    return {code, is_reliable};
}

CharsetCode ToCharsetCodeFinal(CharsetCode inputCode, const char *buf, std::size_t bufSize) {
    switch (inputCode) {
    case CharsetCode::UTF8:
        // 区分有无BOM
        if (bufSize >= sizeof(UTF8BOM_DATA) && memcmp(buf, UTF8BOM_DATA, sizeof(UTF8BOM_DATA)) == 0) {
            return CharsetCode::UTF8BOM;
        }
        return inputCode;
    case CharsetCode::UTF16BE:
        // 区分有无BOM
        if (bufSize >= sizeof(UTF16BEBOM_DATA) && memcmp(buf, UTF16BEBOM_DATA, sizeof(UTF16BEBOM_DATA)) == 0) {
            return CharsetCode::UTF16BEBOM;
        }
        return inputCode;
    case CharsetCode::UTF16LE:
        // 区分有无BOM
        if (bufSize >= sizeof(UTF16LEBOM_DATA) && memcmp(buf, UTF16LEBOM_DATA, sizeof(UTF16LEBOM_DATA)) == 0) {
            return CharsetCode::UTF16LEBOM;
        }
        return inputCode;
    case CharsetCode::UTF32BE:
        // 区分有无BOM
        if (bufSize >= sizeof(UTF32BEBOM_DATA) && memcmp(buf, UTF32BEBOM_DATA, sizeof(UTF32BEBOM_DATA)) == 0) {
            return CharsetCode::UTF32BEBOM;
        }
        return inputCode;
    case CharsetCode::UTF32LE:
        // 区分有无BOM
        if (bufSize >= sizeof(UTF32LEBOM_DATA) && memcmp(buf, UTF32LEBOM_DATA, sizeof(UTF32LEBOM_DATA)) == 0) {
            return CharsetCode::UTF32LEBOM;
        }
        return inputCode;
    }
    return inputCode;
}

CharsetCode DetectEncodingPlain(uchardet *det, const char *buf, std::size_t bufSize, int times) {
    if (bufSize == 0) {
        return CharsetCode::EMPTY;
    }

    if (bufSize > std::numeric_limits<int32_t>::max()) {
        throw MyRuntimeError(MessageId::STRING_LENGTH_OUT_OF_LIMIT);
    }

    auto [ucsdetResult, ucsdetConfidence] = DetectByUCSDet(buf, static_cast<int32_t>(bufSize));

    if (ucsdetConfidence >= 95 && ucsdetResult.find("UTF") != string::npos) {
        // ucsdet如果判定为UTF-8/UTF-16BE|LE等，那么相信它
        return ToCharsetCodeFinal(ToCharsetCode(ucsdetResult), buf, bufSize);
    }

    auto [uchardetResult, uchardetConfidence] = DetectByUCharDet(det, buf, bufSize);
    if (uchardetConfidence >= 95) {
        // uchardet如果有95及以上的信心，那么直接相信它
        return ToCharsetCodeFinal(ToCharsetCode(uchardetResult), buf, bufSize);
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

CharsetCode DetectEncoding(uchardet *det, const char *buf, std::size_t bufSize) {
    return DetectEncodingPlain(det, buf, bufSize, 0);
}