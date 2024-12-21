#include "Exceptions.h"
#include <Common\tstring.h>

ConvertError::ConvertError(std::string content, int position, viet::Encoding srcEncoding,
                           viet::Encoding destEncoding) noexcept
    : std::runtime_error("parse error"), content(content), position(position), srcEncoding(srcEncoding),
      destEncoding(destEncoding) {
    errMsg = std::string("[") + to_string(srcEncoding).data() + "->" + to_string(destEncoding).data() +
             "] convert error at position " + std::to_string(position);
    errMsg += "\n";
    errMsg += "with content:\n";
    for (auto c : content) {
        errMsg += "\\x";
        errMsg += to_hex(c);
    }
}