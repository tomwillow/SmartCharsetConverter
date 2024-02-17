#include "Vietnamese.h"

#include <unordered_map>

const std::unordered_map<std::string, std::string> tcvn3ToUtf8 = {{"\xa5\xee", ""}};

std::string viet::ConvertToUtf8(const std::string &data, Encoding srcEncoding) {
    return std::string();
}

std::string viet::ConvertFromUtf8(const std::string &data, Encoding destEncoding) {
    return std::string();
}
