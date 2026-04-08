/**
 * @file settings_snapshot.cpp
 * @brief UI 设置快照桥接实现
 */

#include "core/settings_snapshot.h"

#include <algorithm>

namespace maccy {

namespace {

void ClampPopupSettings(PopupSettingsSnapshot& popup) {
  popup.width = std::max(420, popup.width);
  popup.height = std::max(280, popup.height);
  popup.preview_width = std::clamp(popup.preview_width, 180, 480);
}

void ClampSettingsSnapshot(SettingsSnapshot& snapshot) {
  snapshot.max_history_items = std::max<std::size_t>(1, snapshot.max_history_items);
  ClampPopupSettings(snapshot.popup);
}

}  // namespace

SettingsSnapshot MakeSettingsSnapshot(const AppSettings& settings) {
  SettingsSnapshot snapshot;
  snapshot.max_history_items = settings.max_history_items;
  snapshot.pin_position = settings.pin_position;
  snapshot.sort_order = settings.sort_order;
  snapshot.search_mode = settings.search_mode;
  snapshot.popup_hotkey_modifiers = settings.popup_hotkey_modifiers;
  snapshot.popup_hotkey_virtual_key = settings.popup_hotkey_virtual_key;
  snapshot.double_click_popup_enabled = settings.double_click_popup_enabled;
  snapshot.double_click_modifier_key = settings.double_click_modifier_key;
  snapshot.show_startup_guide = settings.show_startup_guide;
  snapshot.capture_enabled = settings.capture_enabled;
  snapshot.auto_paste = settings.auto_paste;
  snapshot.paste_plain_text = settings.paste_plain_text;
  snapshot.start_on_login = settings.start_on_login;
  snapshot.clear_history_on_exit = settings.clear_history_on_exit;
  snapshot.clear_system_clipboard_on_exit = settings.clear_system_clipboard_on_exit;
  snapshot.popup.show_search = settings.popup.show_search;
  snapshot.popup.show_preview = settings.popup.show_preview;
  snapshot.popup.remember_last_position = settings.popup.remember_last_position;
  snapshot.popup.has_last_position = settings.popup.has_last_position;
  snapshot.popup.x = settings.popup.x;
  snapshot.popup.y = settings.popup.y;
  snapshot.popup.width = settings.popup.width;
  snapshot.popup.height = settings.popup.height;
  snapshot.popup.preview_width = settings.popup.preview_width;
  snapshot.ignore.ignore_all = settings.ignore.ignore_all;
  snapshot.ignore.only_listed_applications = settings.ignore.only_listed_applications;
  snapshot.ignore.capture_text = settings.ignore.capture_text;
  snapshot.ignore.capture_html = settings.ignore.capture_html;
  snapshot.ignore.capture_rtf = settings.ignore.capture_rtf;
  snapshot.ignore.capture_images = settings.ignore.capture_images;
  snapshot.ignore.capture_files = settings.ignore.capture_files;
  snapshot.ignore.ignored_applications = settings.ignore.ignored_applications;
  snapshot.ignore.allowed_applications = settings.ignore.allowed_applications;
  snapshot.ignore.ignored_patterns = settings.ignore.ignored_patterns;
  snapshot.ignore.ignored_formats = settings.ignore.ignored_formats;
  ClampSettingsSnapshot(snapshot);
  return snapshot;
}

AppSettings ToAppSettings(const SettingsSnapshot& snapshot) {
  SettingsSnapshot normalized = snapshot;
  ClampSettingsSnapshot(normalized);

  AppSettings settings;
  settings.max_history_items = normalized.max_history_items;
  settings.pin_position = normalized.pin_position;
  settings.sort_order = normalized.sort_order;
  settings.search_mode = normalized.search_mode;
  settings.popup_hotkey_modifiers = normalized.popup_hotkey_modifiers;
  settings.popup_hotkey_virtual_key = normalized.popup_hotkey_virtual_key;
  settings.double_click_popup_enabled = normalized.double_click_popup_enabled;
  settings.double_click_modifier_key = normalized.double_click_modifier_key;
  settings.show_startup_guide = normalized.show_startup_guide;
  settings.capture_enabled = normalized.capture_enabled;
  settings.auto_paste = normalized.auto_paste;
  settings.paste_plain_text = normalized.paste_plain_text;
  settings.start_on_login = normalized.start_on_login;
  settings.clear_history_on_exit = normalized.clear_history_on_exit;
  settings.clear_system_clipboard_on_exit = normalized.clear_system_clipboard_on_exit;
  settings.popup.show_search = normalized.popup.show_search;
  settings.popup.show_preview = normalized.popup.show_preview;
  settings.popup.remember_last_position = normalized.popup.remember_last_position;
  settings.popup.has_last_position = normalized.popup.has_last_position;
  settings.popup.x = normalized.popup.x;
  settings.popup.y = normalized.popup.y;
  settings.popup.width = normalized.popup.width;
  settings.popup.height = normalized.popup.height;
  settings.popup.preview_width = normalized.popup.preview_width;
  settings.ignore.ignore_all = normalized.ignore.ignore_all;
  settings.ignore.only_listed_applications = normalized.ignore.only_listed_applications;
  settings.ignore.capture_text = normalized.ignore.capture_text;
  settings.ignore.capture_html = normalized.ignore.capture_html;
  settings.ignore.capture_rtf = normalized.ignore.capture_rtf;
  settings.ignore.capture_images = normalized.ignore.capture_images;
  settings.ignore.capture_files = normalized.ignore.capture_files;
  settings.ignore.ignored_applications = normalized.ignore.ignored_applications;
  settings.ignore.allowed_applications = normalized.ignore.allowed_applications;
  settings.ignore.ignored_patterns = normalized.ignore.ignored_patterns;
  settings.ignore.ignored_formats = normalized.ignore.ignored_formats;
  return settings;
}

void ApplySettingsSnapshot(const SettingsSnapshot& snapshot, AppSettings& settings) {
  settings = ToAppSettings(snapshot);
}

}  // namespace maccy
