/**
 * @file win32_app.cpp
 * @brief Windows 应用程序主类实现
 */

#ifdef _WIN32

#include "app/win32_app.h"
#include "app/resources/resource.h"

#include <commctrl.h>
#include <shellapi.h>
#include <windows.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "core/history_item.h"
#include "core/history_persistence.h"
#include "core/ignore_rules.h"
#include "core/search.h"
#include "core/search_highlight.h"
#include "platform/win32/clipboard.h"
#include "platform/win32/input.h"
#include "platform/win32/source_app.h"
#include "platform/win32/startup.h"
#include "platform/win32/utf.h"

namespace maccy {

namespace {

#include "app/win32_app_anon.inc"

}  // namespace

int Win32App::Run(HINSTANCE instance, int show_command) {
  (void)show_command;

  if (!AcquireSingleInstance()) {
    return 0;
  }

  if (!Initialize(instance)) {
    ReleaseSingleInstance();
    return 1;
  }

  MSG message{};
  while (GetMessageW(&message, nullptr, 0, 0) > 0) {
    if ((settings_window_ != nullptr && IsDialogMessageW(settings_window_, &message) != FALSE) ||
        (pin_editor_window_ != nullptr && IsDialogMessageW(pin_editor_window_, &message) != FALSE)) {
      continue;
    }

    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  Shutdown();
  return static_cast<int>(message.wParam);
}

#include "app/win32_app_lifecycle_tray.inc"

#include "app/win32_app_settings_core.inc"

void Win32App::LoadSettings() {
  settings_ = LoadSettingsFile(settings_path_);
  if (!IsValidPopupHotKey(settings_.popup_hotkey_modifiers, settings_.popup_hotkey_virtual_key)) {
    settings_.popup_hotkey_modifiers = kHotKeyModControl | kHotKeyModShift;
    settings_.popup_hotkey_virtual_key = 'C';
  }
  settings_.start_on_login = win32::IsStartOnLoginEnabled();
  capture_enabled_ = settings_.capture_enabled;
  ApplyStoreOptions();
}

void Win32App::PersistSettings() {
  settings_.capture_enabled = capture_enabled_;
  SavePopupPlacement();
  if (!settings_path_.empty()) {
    (void)SaveSettingsFile(settings_path_, settings_);
  }
}

void Win32App::ApplyStoreOptions() {
  store_.SetOptions(HistoryStoreOptions{
      .max_unpinned_items = settings_.max_history_items,
      .pin_position = settings_.pin_position,
      .sort_order = settings_.sort_order,
  });
}

void Win32App::LoadHistory() {
  store_.ReplaceAll(LoadHistoryFile(history_path_));
  UpdateTrayIcon();
}

void Win32App::PersistHistory() const {
  if (!history_path_.empty()) {
    (void)SaveHistoryFile(history_path_, store_.items());
  }
}

void Win32App::ShowStartupGuide() {
  if (!settings_.show_startup_guide) {
    return;
  }

  const PopupOpenTriggerConfiguration open_trigger_configuration = OpenTriggerConfigurationForSettings(settings_);
  if ((open_trigger_configuration == PopupOpenTriggerConfiguration::kRegularShortcut && !toggle_hotkey_registered_) ||
      (open_trigger_configuration == PopupOpenTriggerConfiguration::kDoubleClick && double_click_hook_ == nullptr)) {
    return;
  }

  std::wstring message = UiText(
      use_chinese_ui_,
      L"ClipLoom is running in the notification area.\n\n",
      L"ClipLoom 正在通知区域运行。\n\n");
  if (open_trigger_configuration == PopupOpenTriggerConfiguration::kDoubleClick) {
    message += UiText(use_chinese_ui_, L"Double-click ", L"双击 ");
    message += DoubleClickModifierLabel(use_chinese_ui_, settings_.double_click_modifier_key);
    message += UiText(
        use_chinese_ui_,
        L" to open clipboard history, or click the tray icon.",
        L" 可打开剪贴板历史记录，或点击托盘图标。");
  } else {
    message += UiText(use_chinese_ui_, L"Press ", L"按下 ");
    message += FormatPopupHotKey(use_chinese_ui_, settings_.popup_hotkey_modifiers, settings_.popup_hotkey_virtual_key);
    message += UiText(
        use_chinese_ui_,
        L" or click the tray icon to open clipboard history.",
        L"，或点击托盘图标打开剪贴板历史记录。");
  }
  ShowDialog(
      controller_window_,
      message,
      MB_ICONINFORMATION);

  settings_.show_startup_guide = false;
  PersistSettings();
}

#include "app/win32_app_popup_input.inc"

std::filesystem::path Win32App::ResolveHistoryPath() const {
  return ResolveAppDataDirectory() / "history.dat";
}

std::filesystem::path Win32App::ResolveSettingsPath() const {
  return ResolveAppDataDirectory() / "settings.dat";
}

#include "app/win32_app_message_handlers.inc"

#include "app/win32_app_settings_message.inc"

#include "app/win32_app_window_procs.inc"

}  // namespace maccy

#endif  // _WIN32
