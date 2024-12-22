#pragma once

// self
#include "CharsetCode.h"

// third-party lib
#include <uchardet.h>

// standard lib
#include <string>

std::tuple<std::string, int> DetectByUCharDet(uchardet *det, const char *buf, std::size_t bufSize);

/**
 * @param bufSize 受ucsdet限制只能接受int32_t
 */
std::tuple<std::string, int> DetectByUCSDet(const char *buf, int32_t bufSize);

/**
 * @exception runtime_error 如果CED中定义的名称在CharsetCode中没有定义，将抛出异常
 */
std::tuple<CharsetCode, bool> DetectByCED(const char *buf, int len);

CharsetCode ToCharsetCodeFinal(std::string charsetStr, const char *buf, std::size_t bufSize);

/**
 * 探测编码集。
 * return 探测出的编码集，根据探测出的编码集解码出的Unicode文本片段(最大64bytes)，文本片段长度
 * @exception file_io_error 读文件失败
 * @exception runtime_error ucnv出错。code
 */
CharsetCode DetectEncodingPlain(uchardet *det, const char *buf, std::size_t bufSize, int times);

/**
 * 探测编码集。
 * return 探测出的编码集，根据探测出的编码集解码出的Unicode文本片段(最大64bytes)，文本片段长度
 * @exception file_io_error 读文件失败
 * @exception runtime_error ucnv出错。code
 */
CharsetCode DetectEncoding(uchardet *det, const char *buf, std::size_t bufSize);
