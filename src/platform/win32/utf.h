/**
 * @file utf.h
 * @brief UTF-8 和宽字符转换接口定义
 */
#pragma once

#ifdef _WIN32

#include <string>
#include <string_view>

namespace maccy::win32 {

/**
 * @brief UTF-8 转宽字符
 * @param text UTF-8 编码的文本
 * @return std::wstring 宽字符编码的文本
 */
[[nodiscard]] std::wstring Utf8ToWide(std::string_view text);

/**
 * @brief 宽字符转 UTF-8
 * @param text 宽字符编码的文本
 * @return std::string UTF-8 编码的文本
 */
[[nodiscard]] std::string WideToUtf8(std::wstring_view text);

}  // namespace maccy::win32

#endif  // _WIN32
