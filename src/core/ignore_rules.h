/**
 * @file ignore_rules.h
 * @brief 忽略规则应用接口定义
 */
#pragma once

#include "core/history_item.h"
#include "core/settings.h"

namespace maccy {

/**
 * @brief 忽略原因枚举
 * @details 定义了内容被忽略的具体原因
 */
enum class IgnoreReason {
  kNone,                  /**< 没有被忽略 */
  kIgnoreAll,             /**< 全部被忽略 */
  kIgnoredApplication,     /**< 来源应用被忽略 */
  kNotAllowedApplication, /**< 来源应用不在允许列表 */
  kIgnoredFormat,         /**< 格式被忽略 */
  kIgnoredPattern,        /**< 内容模式被忽略 */
  kEmptyItem,             /**< 空内容项 */
};

/**
 * @brief 忽略决策结构体
 * @details 包含忽略判断的结果和相关信息
 */
struct IgnoreDecision {
  bool should_store = false;  /**< 是否应该存储 */
  IgnoreReason reason = IgnoreReason::kNone; /**< 忽略原因 */
  HistoryItem item;            /**< 历史记录项 */
};

/**
 * @brief 应用忽略规则
 * @param settings 应用程序设置
 * @param item 历史记录项
 * @return IgnoreDecision 忽略决策结果
 */
[[nodiscard]] IgnoreDecision ApplyIgnoreRules(
    const AppSettings& settings,
    HistoryItem item);

}  // namespace maccy
