#include "Core.h"

#include "Language.h"
#include "UCNVHelper.h"

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <compact_enc_det/compact_enc_det.h>

#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

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
        auto tryCode = static_cast<CharsetCode>(i);

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
    Encoding encoding = DetectEncoding(buf, len, nullptr, // URL hint
                                       nullptr, // HTTP hint
                                       nullptr, // Meta hint
                                       UNKNOWN_ENCODING, UNKNOWN_LANGUAGE, CompactEncDet::WEB_CORPUS,
                                       false, // Include 7-bit encodings?
                                       &bytes_consumed, &is_reliable);

    // 如果认为是二进制文件，那么取信它
    if (encoding == BINARYENC) {
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

CharsetCode ToCharsetCodeFinal(const std::string& charsetStr, const char *buf, int bufSize) {

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

CharsetCode DetectEncodingPlain(uchardet *det, const char *buf, int bufSize, int times) {
    if (bufSize == 0) {
        return CharsetCode::EMPTY;
    }

    std::string error = "";

    std::string ucsdetResult{};
    int ucsdetConfidence = 0;

    try {
        auto result = DetectByUCSDet(buf, bufSize);
        ucsdetResult = std::get<0>(result);
        ucsdetConfidence = std::get<1>(result);
    } catch (const ucnv_error& u_error) {
        error = u_error.what();
    }

    int ucsdetWeight = 100;
    if (ucsdetResult.find("UTF") != string::npos) {
        ucsdetWeight = 120;
    }

    std::string uchardetResult{};
    int uchardetConfidence = 0;

    try {
        const auto result = DetectByUCharDet(det, buf, bufSize);
        uchardetResult = std::get<0>(result);
        uchardetConfidence = std::get<1>(result);
    }catch (const std::runtime_error& u_error) {
        if(!error.empty()) {
            error += " & ";
        }
        error += u_error.what();
    }
    
    int uchardetWeight = 100;

    //int cedConfidence = 0;
    //auto [cedResult, reliable] = DetectByCED(buf, bufSize);
    //int ceddetWeight = 40;
    //if (reliable) {
    //    cedConfidence = 100;
    //}

    ucsdetConfidence *= ucsdetWeight;
    uchardetConfidence *= uchardetWeight;
    //cedConfidence *= ceddetWeight;

    std::array<int, 2> confidences = {ucsdetConfidence, uchardetConfidence};
    std::array<std::string, 2> results = { ucsdetResult, uchardetResult };

    size_t maxIndex = std::max_element(confidences.begin(), confidences.end()) - confidences.begin();
    auto maxValue = confidences[maxIndex];

    if(!maxValue) {
        throw std::runtime_error(error);
    }

    if(maxValue >= 900) {
        return ToCharsetCodeFinal(results[maxIndex], buf, bufSize);
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

CharsetCode DetectEncoding(uchardet *det, const char *buf, int bufSize) {
    return DetectEncodingPlain(det, buf, bufSize, 0);
}