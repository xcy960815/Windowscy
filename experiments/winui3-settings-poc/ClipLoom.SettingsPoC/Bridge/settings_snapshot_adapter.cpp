#include "Bridge/settings_snapshot_adapter.h"

#include "../../../../src/core/settings.h"

namespace maccy {

SettingsSnapshot BuildDemoSettingsSnapshot() {
  AppSettings settings;
  settings.double_click_popup_enabled = true;
  settings.double_click_modifier_key = DoubleClickModifierKey::kControl;
  settings.show_startup_guide = false;
  settings.capture_enabled = true;
  settings.auto_paste = false;
  settings.paste_plain_text = false;
  settings.start_on_login = false;
  settings.popup.show_search = true;
  settings.popup.show_preview = true;
  settings.popup.remember_last_position = true;
  settings.popup.preview_width = 320;
  settings.pin_position = PinPosition::kTop;
  settings.search_mode = SearchMode::kMixed;
  return MakeSettingsSnapshot(settings);
}

}  // namespace maccy
