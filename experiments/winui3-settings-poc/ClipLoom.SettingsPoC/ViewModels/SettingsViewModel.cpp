#include "ViewModels/SettingsViewModel.h"

namespace ClipLoom::SettingsPoC::ViewModels {

SettingsViewModel::SettingsViewModel()
    : SettingsViewModel(maccy::BuildDemoSettingsSnapshot()) {}

SettingsViewModel::SettingsViewModel(maccy::SettingsSnapshot snapshot)
    : snapshot_(std::move(snapshot)),
      window_title_(L"ClipLoom Settings PoC"),
      window_subtitle_(
          L"A WinUI 3 shell for replacing the current owner-draw Win32 settings UI in stages.") {}

SettingsViewModel SettingsViewModel::CreateDemo() {
  return SettingsViewModel(maccy::BuildDemoSettingsSnapshot());
}

const maccy::SettingsSnapshot& SettingsViewModel::Snapshot() const {
  return snapshot_;
}

const std::wstring& SettingsViewModel::WindowTitle() const {
  return window_title_;
}

const std::wstring& SettingsViewModel::WindowSubtitle() const {
  return window_subtitle_;
}

}  // namespace ClipLoom::SettingsPoC::ViewModels
