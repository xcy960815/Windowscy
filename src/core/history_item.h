/**
 * @file history_item.h
 * @brief 历史记录项的数据结构定义
 */
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace maccy {

/**
 * @brief 内容格式枚举，定义了剪贴板内容的不同类型
 */
enum class ContentFormat {
  kPlainText,    /**< 纯文本格式 */
  kHtml,         /**< HTML 格式 */
  kRtf,          /**< RTF 富文本格式 */
  kImage,        /**< 图片格式 */
  kFileList,     /**< 文件列表格式 */
  kCustom,       /**< 自定义格式 */
};

/**
 * @brief 内容数据块结构体，存储单个内容项的格式和文本数据
 */
struct ContentBlob {
  ContentFormat format = ContentFormat::kPlainText; /**< 内容格式 */
  std::string format_name;                          /**< 格式名称 */
  std::string text_payload;                         /**< 文本载荷 */
};

/**
 * @brief 历史记录元数据结构体，存储与历史记录相关的统计信息
 */
struct HistoryMetadata {
  std::string source_application;      /**< 来源应用程序名称 */
  std::uint64_t copy_count = 1;        /**< 复制次数 */
  std::uint64_t first_copied_at = 0;   /**< 首次复制时间戳 */
  std::uint64_t last_copied_at = 0;    /**< 最后复制时间戳 */
  bool from_app = false;              /**< 是否来自应用程序 */
  bool modified_after_copy = false;    /**< 复制后是否被修改 */
};

/**
 * @brief 历史记录项结构体，表示一个剪贴板历史记录
 */
struct HistoryItem {
  std::uint64_t id = 0;                          /**< 唯一标识符 */
  std::string title;                             /**< 显示标题 */
  bool title_overridden = false;                 /**< 标题是否被手动覆盖 */
  std::vector<ContentBlob> contents;             /**< 内容块列表 */
  HistoryMetadata metadata;                      /**< 元数据信息 */
  bool pinned = false;                           /**< 是否被固定 */
  std::optional<char> pin_key;                   /**< 固定键（如果有） */

  /**
   * @brief 获取首选的文本内容
   * @return std::string 首选的文本内容
   */
  [[nodiscard]] std::string PreferredContentText() const;

  /**
   * @brief 获取首选的显示文本
   * @return std::string 首选的显示文本
   */
  [[nodiscard]] std::string PreferredDisplayText() const;

  /**
   * @brief 获取首选的搜索文本
   * @return std::string 首选的搜索文本
   */
  [[nodiscard]] std::string PreferredSearchText() const;

  /**
   * @brief 获取稳定的去重键
   * @return std::string 稳定的去重键
   */
  [[nodiscard]] std::string StableDedupeKey() const;
};

/**
 * @brief 标准化搜索文本
 * @param value 输入的文本
 * @return std::string 标准化后的文本
 */
[[nodiscard]] std::string NormalizeForSearch(std::string_view value);

/**
 * @brief 从文本内容自动生成标题
 * @param value 文本内容
 * @param max_length 最大标题长度，默认为 80
 * @return std::string 自动生成的标题
 */
[[nodiscard]] std::string BuildAutomaticTitle(std::string_view value, std::size_t max_length = 80);

/**
 * @brief 获取内容格式的名称
 * @param format 内容格式
 * @return std::string 格式名称
 */
[[nodiscard]] std::string ContentFormatName(ContentFormat format);

}  // namespace maccy
