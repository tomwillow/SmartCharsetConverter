#pragma once

#include <string>

enum class MessageId;
class TranslatorBase {
public:
    virtual std::string MessageIdToString(MessageId mid) const noexcept = 0;
};