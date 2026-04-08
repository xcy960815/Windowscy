#include "Views/AppearancePage.xaml.h"

namespace winrt::ClipLoom::SettingsPoC::Views::implementation {

AppearancePage::AppearancePage() {
  InitializeComponent();
  PinPositionCombo().SelectedIndex(0);
  SearchModeCombo().SelectedIndex(0);
}

}  // namespace winrt::ClipLoom::SettingsPoC::Views::implementation
