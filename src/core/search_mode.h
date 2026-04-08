/**
 * @file search_mode.h
 * @brief 搜索模式轻量类型定义
 */
#pragma once

#include <string_view>

namespace maccy {

/**
 * @brief 搜索模式枚举
 */
enum class SearchMode {
  kExact,  /**< 精确匹配模式 */
  kFuzzy,  /**< 模糊匹配模式 */
  kRegexp, /**< 正则表达式匹配模式 */
  kMixed,  /**< 混合模式（精确+模糊） */
};

/**
 * @brief 将搜索模式转换为字符串
 * @param mode 搜索模式
 * @return std::string_view 模式对应的字符串表示
 */
[[nodiscard]] std::string_view ToString(SearchMode mode);

}  // namespace maccy
