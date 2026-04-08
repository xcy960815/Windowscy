#include "Views/GeneralPage.xaml.h"

namespace winrt::ClipLoom::SettingsPoC::Views::implementation {

GeneralPage::GeneralPage() {
  InitializeComponent();
  ModifierKeyCombo().SelectedIndex(0);
  HotKeyCombo().SelectedIndex(0);
}

}  // namespace winrt::ClipLoom::SettingsPoC::Views::implementation
