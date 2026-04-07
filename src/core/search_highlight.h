/**
 * @file search_highlight.h
 * @brief 搜索高亮功能接口定义
 */
#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "core/search.h"

namespace maccy {

/**
 * @brief 高亮区间结构体
 * @details 定义了文本中需要高亮的区间
 */
struct HighlightSpan {
  std::size_t start = 0;   /**< 起始位置 */
  std::size_t length = 0; /**< 长度 */
};

/**
 * @brief 构建高亮区间
 * @param mode 搜索模式
 * @param query 搜索查询词
 * @param candidate 候选文本
 * @return std::vector<HighlightSpan> 高亮区间列表
 */
[[nodiscard]] std::vector<HighlightSpan> BuildHighlightSpans(
    SearchMode mode,
    std::string_view query,
    std::string_view candidate);

}  // namespace maccy
