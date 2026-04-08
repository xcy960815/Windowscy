#pragma once

#include <string>

#include "../Bridge/settings_snapshot_adapter.h"

namespace ClipLoom::SettingsPoC::ViewModels {

class SettingsViewModel {
 public:
  SettingsViewModel();
  explicit SettingsViewModel(maccy::SettingsSnapshot snapshot);

  static SettingsViewModel CreateDemo();

  const maccy::SettingsSnapshot& Snapshot() const;
  const std::wstring& WindowTitle() const;
  const std::wstring& WindowSubtitle() const;

 private:
  maccy::SettingsSnapshot snapshot_;
  std::wstring window_title_;
  std::wstring window_subtitle_;
};

}  // namespace ClipLoom::SettingsPoC::ViewModels
