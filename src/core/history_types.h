/**
 * @file history_types.h
 * @brief 历史记录存储相关轻量类型定义
 */
#pragma once

#include <cstddef>

namespace maccy {

/**
 * @brief 固定位置枚举
 */
enum class PinPosition {
  kTop,    /**< 固定在顶部 */
  kBottom, /**< 固定在底部 */
};

/**
 * @brief 历史记录排序方式枚举
 */
enum class HistorySortOrder {
  kLastCopied,  /**< 按最后复制时间排序 */
  kFirstCopied, /**< 按首次复制时间排序 */
  kCopyCount,   /**< 按复制次数排序 */
};

/**
 * @brief 历史记录存储选项结构体
 */
struct HistoryStoreOptions {
  std::size_t max_unpinned_items = 200;                         /**< 最大未固定项数量 */
  PinPosition pin_position = PinPosition::kTop;                /**< 固定项位置 */
  HistorySortOrder sort_order = HistorySortOrder::kLastCopied; /**< 排序方式 */
};

}  // namespace maccy
