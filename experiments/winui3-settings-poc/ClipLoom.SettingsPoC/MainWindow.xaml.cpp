#include "MainWindow.xaml.h"

#include "Views/AppearancePage.xaml.h"
#include "Views/GeneralPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::ClipLoom::SettingsPoC::implementation {

MainWindow::MainWindow()
    : m_view_model(ClipLoom::SettingsPoC::ViewModels::SettingsViewModel::CreateDemo()) {
  InitializeComponent();
  Title(L"ClipLoom Settings PoC");

  WindowTitleText().Text(hstring{m_view_model.WindowTitle()});
  WindowSubtitleText().Text(hstring{m_view_model.WindowSubtitle()});

  auto first_item = RootNavigationView().MenuItems().GetAt(0).try_as<NavigationViewItem>();
  RootNavigationView().SelectedItem(first_item);
  NavigateToTag(L"general");
}

void MainWindow::NavigationViewSelectionChanged(
    NavigationView const&,
    NavigationViewSelectionChangedEventArgs const& args) {
  const auto item = args.SelectedItemContainer();
  if (item == nullptr) {
    return;
  }

  const auto tag = unbox_value_or<hstring>(item.Tag(), L"general");
  NavigateToTag(tag);
}

void MainWindow::SaveButtonClick(
    Windows::Foundation::IInspectable const&,
    RoutedEventArgs const&) {
  SetStatus(L"Save clicked. Next step is wiring SettingsSnapshot back to PersistSettings().");
}

void MainWindow::ApplyButtonClick(
    Windows::Foundation::IInspectable const&,
    RoutedEventArgs const&) {
  SetStatus(L"Apply clicked. Hook this to ApplySettingsSnapshot(...) in the host app.");
}

void MainWindow::CloseButtonClick(
    Windows::Foundation::IInspectable const&,
    RoutedEventArgs const&) {
  Close();
}

void MainWindow::NavigateToTag(hstring const& tag) {
  if (tag == L"appearance") {
    ContentFrame().Navigate(xaml_typename<winrt::ClipLoom::SettingsPoC::Views::AppearancePage>());
    CurrentPageTagText().Text(L"Appearance");
    CurrentPageSummaryText().Text(
        L"Validate preview, search visibility and shell presentation with WinUI controls.");
    SetStatus(L"Appearance page loaded from the demo snapshot.");
    return;
  }

  ContentFrame().Navigate(xaml_typename<winrt::ClipLoom::SettingsPoC::Views::GeneralPage>());
  CurrentPageTagText().Text(L"General");
  CurrentPageSummaryText().Text(
      L"Validate opening trigger, startup behavior and capture toggles with Fluent components.");
  SetStatus(L"General page loaded from the demo snapshot.");
}

void MainWindow::SetStatus(hstring const& message) {
  ActionStatusText().Text(message);
}

}  // namespace winrt::ClipLoom::SettingsPoC::implementation
