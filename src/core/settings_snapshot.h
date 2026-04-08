/**
 * @file settings_snapshot.h
 * @brief UI 设置快照桥接模型
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "core/double_click_modifier.h"
#include "core/history_types.h"
#include "core/search_mode.h"
#include "core/settings.h"

namespace maccy {

/**
 * @brief UI 可编辑的忽略规则快照
 */
struct IgnoreRulesSnapshot {
  bool ignore_all = false;
  bool only_listed_applications = false;
  bool capture_text = true;
  bool capture_html = true;
  bool capture_rtf = true;
  bool capture_images = true;
  bool capture_files = true;
  std::vector<std::string> ignored_applications;
  std::vector<std::string> allowed_applications;
  std::vector<std::string> ignored_patterns;
  std::vector<std::string> ignored_formats;
};

/**
 * @brief UI 可编辑的弹窗设置快照
 */
struct PopupSettingsSnapshot {
  bool show_search = true;
  bool show_preview = true;
  bool remember_last_position = true;
  bool has_last_position = false;
  int x = 0;
  int y = 0;
  int width = 760;
  int height = 420;
  int preview_width = 280;
};

/**
 * @brief UI 可编辑的完整设置快照
 * @details 该结构用于桥接 Win32 / WinUI / 其他 UI 技术栈和 AppSettings。
 */
struct SettingsSnapshot {
  std::size_t max_history_items = 200;
  PinPosition pin_position = PinPosition::kTop;
  HistorySortOrder sort_order = HistorySortOrder::kLastCopied;
  SearchMode search_mode = SearchMode::kMixed;
  std::uint32_t popup_hotkey_modifiers = 0x0006;
  std::uint32_t popup_hotkey_virtual_key = 'C';
  bool double_click_popup_enabled = false;
  DoubleClickModifierKey double_click_modifier_key = DoubleClickModifierKey::kNone;
  bool show_startup_guide = true;
  bool capture_enabled = true;
  bool auto_paste = true;
  bool paste_plain_text = false;
  bool start_on_login = false;
  bool clear_history_on_exit = false;
  bool clear_system_clipboard_on_exit = false;
  PopupSettingsSnapshot popup;
  IgnoreRulesSnapshot ignore;
};

/**
 * @brief 从应用设置生成 UI 快照
 * @param settings 应用设置
 * @return SettingsSnapshot 对应的 UI 快照
 */
[[nodiscard]] SettingsSnapshot MakeSettingsSnapshot(const AppSettings& settings);

/**
 * @brief 将 UI 快照映射回应用设置
 * @param snapshot UI 快照
 * @return AppSettings 应用设置
 */
[[nodiscard]] AppSettings ToAppSettings(const SettingsSnapshot& snapshot);

/**
 * @brief 用 UI 快照覆盖现有应用设置
 * @param snapshot UI 快照
 * @param settings 需要更新的应用设置
 */
void ApplySettingsSnapshot(const SettingsSnapshot& snapshot, AppSettings& settings);

}  // namespace maccy
