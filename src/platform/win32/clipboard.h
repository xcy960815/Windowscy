/**
 * @file clipboard.h
 * @brief Windows 剪贴板操作接口定义
 */
#pragma once

#ifdef _WIN32

#include <windows.h>

#include <optional>
#include <string>
#include <string_view>

#include "core/history_item.h"

namespace maccy::win32 {

/**
 * @brief 读取历史记录项
 * @param owner 所有者窗口句柄
 * @return std::optional<HistoryItem> 读取的历史记录项
 */
[[nodiscard]] std::optional<HistoryItem> ReadHistoryItem(HWND owner);

/**
 * @brief 读取纯文本
 * @param owner 所有者窗口句柄
 * @return std::optional<std::string> 读取的纯文本
 */
[[nodiscard]] std::optional<std::string> ReadPlainText(HWND owner);

/**
 * @brief 写入历史记录项
 * @param owner 所有者窗口句柄
 * @param item 要写入的历史记录项
 * @param plain_text_only 是否只写入纯文本，默认为 false
 * @return bool 写入是否成功
 */
[[nodiscard]] bool WriteHistoryItem(HWND owner, const HistoryItem& item, bool plain_text_only = false);

/**
 * @brief 写入纯文本
 * @param owner 所有者窗口句柄
 * @param text 要写入的纯文本
 * @return bool 写入是否成功
 */
[[nodiscard]] bool WritePlainText(HWND owner, std::string_view text);

/**
 * @brief 清空剪贴板
 * @param owner 所有者窗口句柄
 * @return bool 清空是否成功
 */
[[nodiscard]] bool ClearClipboard(HWND owner);

/**
 * @brief 从文本内容构建历史标题
 * @param text 文本内容
 * @param max_length 最大标题长度，默认为 80
 * @return std::string 生成的标题
 */
[[nodiscard]] std::string BuildHistoryTitleFromText(std::string_view text, std::size_t max_length = 80);

}  // namespace maccy::win32

#endif  // _WIN32
