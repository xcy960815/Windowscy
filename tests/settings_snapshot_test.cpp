#include <cassert>
#include <string>
#include <vector>

#include "core/settings_snapshot.h"

int main() {
  maccy::AppSettings original;
  original.max_history_items = 512;
  original.pin_position = maccy::PinPosition::kBottom;
  original.sort_order = maccy::HistorySortOrder::kCopyCount;
  original.search_mode = maccy::SearchMode::kRegexp;
  original.popup_hotkey_modifiers = 0x0003;
  original.popup_hotkey_virtual_key = 0x77;
  original.double_click_popup_enabled = true;
  original.double_click_modifier_key = maccy::DoubleClickModifierKey::kShift;
  original.show_startup_guide = false;
  original.capture_enabled = false;
  original.auto_paste = false;
  original.paste_plain_text = true;
  original.start_on_login = true;
  original.clear_history_on_exit = true;
  original.clear_system_clipboard_on_exit = true;
  original.popup.show_search = false;
  original.popup.show_preview = true;
  original.popup.remember_last_position = false;
  original.popup.has_last_position = true;
  original.popup.x = 77;
  original.popup.y = 88;
  original.popup.width = 900;
  original.popup.height = 540;
  original.popup.preview_width = 320;
  original.ignore.ignore_all = false;
  original.ignore.only_listed_applications = true;
  original.ignore.capture_html = false;
  original.ignore.capture_images = false;
  original.ignore.ignored_applications = {"Slack.exe", "KeePassXC.exe"};
  original.ignore.allowed_applications = {"Code.exe"};
  original.ignore.ignored_patterns = {"password=.*", "token=.*"};
  original.ignore.ignored_formats = {"html", "CF_PRIVATE"};

  const auto snapshot = maccy::MakeSettingsSnapshot(original);
  const auto roundtrip = maccy::ToAppSettings(snapshot);

  assert(roundtrip.max_history_items == original.max_history_items);
  assert(roundtrip.pin_position == original.pin_position);
  assert(roundtrip.sort_order == original.sort_order);
  assert(roundtrip.search_mode == original.search_mode);
  assert(roundtrip.popup_hotkey_modifiers == original.popup_hotkey_modifiers);
  assert(roundtrip.popup_hotkey_virtual_key == original.popup_hotkey_virtual_key);
  assert(roundtrip.double_click_popup_enabled == original.double_click_popup_enabled);
  assert(roundtrip.double_click_modifier_key == original.double_click_modifier_key);
  assert(roundtrip.show_startup_guide == original.show_startup_guide);
  assert(roundtrip.capture_enabled == original.capture_enabled);
  assert(roundtrip.auto_paste == original.auto_paste);
  assert(roundtrip.paste_plain_text == original.paste_plain_text);
  assert(roundtrip.start_on_login == original.start_on_login);
  assert(roundtrip.clear_history_on_exit == original.clear_history_on_exit);
  assert(roundtrip.clear_system_clipboard_on_exit == original.clear_system_clipboard_on_exit);
  assert(roundtrip.popup.show_search == original.popup.show_search);
  assert(roundtrip.popup.show_preview == original.popup.show_preview);
  assert(roundtrip.popup.remember_last_position == original.popup.remember_last_position);
  assert(roundtrip.popup.has_last_position == original.popup.has_last_position);
  assert(roundtrip.popup.x == original.popup.x);
  assert(roundtrip.popup.y == original.popup.y);
  assert(roundtrip.popup.width == original.popup.width);
  assert(roundtrip.popup.height == original.popup.height);
  assert(roundtrip.popup.preview_width == original.popup.preview_width);
  assert(roundtrip.ignore.only_listed_applications == original.ignore.only_listed_applications);
  assert(roundtrip.ignore.capture_html == original.ignore.capture_html);
  assert(roundtrip.ignore.capture_images == original.ignore.capture_images);
  assert(roundtrip.ignore.ignored_applications == original.ignore.ignored_applications);
  assert(roundtrip.ignore.allowed_applications == original.ignore.allowed_applications);
  assert(roundtrip.ignore.ignored_patterns == original.ignore.ignored_patterns);
  assert(roundtrip.ignore.ignored_formats == original.ignore.ignored_formats);

  maccy::SettingsSnapshot edited = snapshot;
  edited.max_history_items = 0;
  edited.popup.width = 100;
  edited.popup.height = 120;
  edited.popup.preview_width = 999;

  maccy::AppSettings clamped = original;
  maccy::ApplySettingsSnapshot(edited, clamped);

  assert(clamped.max_history_items == 1);
  assert(clamped.popup.width == 420);
  assert(clamped.popup.height == 280);
  assert(clamped.popup.preview_width == 480);
  return 0;
}
