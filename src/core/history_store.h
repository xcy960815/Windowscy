/**
 * @file history_store.h
 * @brief 历史记录存储管理类定义
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/history_types.h"
#include "core/history_item.h"

namespace maccy {

/**
 * @brief 历史记录存储管理类
 * @details 负责管理剪贴板历史记录的添加、删除、排序和去重
 */
class HistoryStore {
 public:
  /**
   * @brief 构造函数
   * @param options 存储选项，默认为空
   */
  explicit HistoryStore(HistoryStoreOptions options = {});

  /**
   * @brief 获取所有历史记录项
   * @return const std::vector<HistoryItem>& 历史记录项列表的常量引用
   */
  [[nodiscard]] const std::vector<HistoryItem>& items() const;

  /**
   * @brief 获取当前存储选项
   * @return const HistoryStoreOptions& 存储选项的常量引用
   */
  [[nodiscard]] const HistoryStoreOptions& options() const;

  /**
   * @brief 添加历史记录项
   * @param item 要添加的历史记录项
   * @return std::uint64_t 添加项的ID
   */
  std::uint64_t Add(HistoryItem item);

  /**
   * @brief 设置存储选项
   * @param options 新的存储选项
   */
  void SetOptions(HistoryStoreOptions options);

  /**
   * @brief 根据ID删除历史记录项
   * @param id 要删除的项的ID
   * @return bool 删除是否成功
   */
  bool RemoveById(std::uint64_t id);

  /**
   * @brief 切换指定项的固定状态
   * @param id 要切换的项的ID
   * @return bool 操作是否成功
   */
  bool TogglePin(std::uint64_t id);

  /**
   * @brief 重命名固定项
   * @param id 要重命名的项的ID
   * @param title 新的标题
   * @return bool 操作是否成功
   */
  bool RenamePinnedItem(std::uint64_t id, std::string title);

  /**
   * @brief 更新固定项的文本
   * @param id 要更新的项的ID
   * @param text 新的文本内容
   * @return bool 操作是否成功
   */
  bool UpdatePinnedText(std::uint64_t id, std::string text);

  /**
   * @brief 清除所有未固定的项
   */
  void ClearUnpinned();

  /**
   * @brief 清除所有项
   */
  void ClearAll();

  /**
   * @brief 替换所有项
   * @param items 新的历史记录项列表
   */
  void ReplaceAll(std::vector<HistoryItem> items);

  /**
   * @brief 根据ID查找历史记录项
   * @param id 要查找的项的ID
   * @return HistoryItem* 找到的项的指针，未找到则返回nullptr
   */
  [[nodiscard]] HistoryItem* FindById(std::uint64_t id);

  /**
   * @brief 根据ID查找历史记录项（常量版本）
   * @param id 要查找的项的ID
   * @return const HistoryItem* 找到的项的常量指针，未找到则返回nullptr
   */
  [[nodiscard]] const HistoryItem* FindById(std::uint64_t id) const;

  /**
   * @brief 获取可用的固定键列表
   * @return std::vector<char> 可用的固定键列表
   */
  [[nodiscard]] std::vector<char> AvailablePinKeys() const;

 private:
  /**
   * @brief 获取支持的固定键列表
   * @return std::vector<char> 支持的固定键列表
   */
  static std::vector<char> SupportedPinKeys();

  /**
   * @brief 对项进行排序
   */
  void SortItems();

  /**
   * @brief 强制执行数量限制
   */
  void EnforceLimit();

  /**
   * @brief 将传入项合并到目标项
   * @param target 目标项
   * @param incoming 传入的项
   */
  void MergeInto(HistoryItem& target, const HistoryItem& incoming);

  /**
   * @brief 为新项添加时间戳
   * @param item 要添加时间戳的项
   */
  void StampNewItem(HistoryItem& item);

  HistoryStoreOptions options_;    /**< 存储选项 */
  std::vector<HistoryItem> items_;  /**< 历史记录项列表 */
  std::uint64_t next_id_ = 1;       /**< 下一个可用的ID */
  std::uint64_t next_tick_ = 1;     /**< 下一个时间刻度 */
};

}  // namespace maccy
