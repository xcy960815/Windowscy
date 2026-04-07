/**
 * @file history_persistence.h
 * @brief 历史记录持久化接口定义
 */
#pragma once

#include <filesystem>
#include <vector>

#include "core/history_item.h"

namespace maccy {

/**
 * @brief 保存历史记录到文件
 * @param path 历史记录文件路径
 * @param items 要保存的历史记录项列表
 * @return bool 保存是否成功
 */
[[nodiscard]] bool SaveHistoryFile(
    const std::filesystem::path& path,
    const std::vector<HistoryItem>& items);

/**
 * @brief 从文件加载历史记录
 * @param path 历史记录文件路径
 * @return std::vector<HistoryItem> 加载的历史记录项列表
 */
[[nodiscard]] std::vector<HistoryItem> LoadHistoryFile(
    const std::filesystem::path& path);

}  // namespace maccy
