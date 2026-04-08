/**
 * @file search.h
 * @brief 搜索功能接口定义
 */
#pragma once

#include <vector>

#include "core/history_item.h"
#include "core/search_mode.h"

namespace maccy {

/**
 * @brief 精确搜索
 * @param query 搜索查询词
 * @param items 历史记录项列表
 * @return std::vector<const HistoryItem*> 匹配的历史记录项指针列表
 */
[[nodiscard]] std::vector<const HistoryItem*> ExactSearch(
    std::string_view query,
    const std::vector<HistoryItem>& items);

/**
 * @brief 模糊搜索
 * @param query 搜索查询词
 * @param items 历史记录项列表
 * @return std::vector<const HistoryItem*> 匹配的历史记录项指针列表
 */
[[nodiscard]] std::vector<const HistoryItem*> FuzzySearch(
    std::string_view query,
    const std::vector<HistoryItem>& items);

/**
 * @brief 正则表达式搜索
 * @param query 搜索查询词（正则表达式）
 * @param items 历史记录项列表
 * @return std::vector<const HistoryItem*> 匹配的历史记录项指针列表
 */
[[nodiscard]] std::vector<const HistoryItem*> RegexpSearch(
    std::string_view query,
    const std::vector<HistoryItem>& items);

/**
 * @brief 混合搜索（同时使用精确和模糊搜索）
 * @param query 搜索查询词
 * @param items 历史记录项列表
 * @return std::vector<const HistoryItem*> 匹配的历史记录项指针列表
 */
[[nodiscard]] std::vector<const HistoryItem*> MixedSearch(
    std::string_view query,
    const std::vector<HistoryItem>& items);

/**
 * @brief 根据指定模式执行搜索
 * @param mode 搜索模式
 * @param query 搜索查询词
 * @param items 历史记录项列表
 * @return std::vector<const HistoryItem*> 匹配的历史记录项指针列表
 */
[[nodiscard]] std::vector<const HistoryItem*> Search(
    SearchMode mode,
    std::string_view query,
    const std::vector<HistoryItem>& items);

}  // namespace maccy
