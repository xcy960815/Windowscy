#include "MainWindow.xaml.h"

#include "Views/GeneralPage.xaml.h"
#include "Views/PreviewPage.xaml.h"

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
  NavigateToTag(L"settings");
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
  if (tag == L"preview") {
    ContentFrame().Navigate(xaml_typename<winrt::ClipLoom::SettingsPoC::Views::PreviewPage>());
    CurrentPageTagText().Text(L"Preview");
    CurrentPageSummaryText().Text(
        L"Replace the owner-draw list and read-only preview edit control with native WinUI layout.");
    SetStatus(L"Preview page loaded. Use this to judge the new history + detail presentation.");
    return;
  }

  ContentFrame().Navigate(xaml_typename<winrt::ClipLoom::SettingsPoC::Views::GeneralPage>());
  CurrentPageTagText().Text(L"Settings");
  CurrentPageSummaryText().Text(
      L"Validate opening trigger, startup behavior and capture toggles with Fluent components.");
  SetStatus(L"Settings page loaded. This is the new configuration shell direction.");
}

void MainWindow::SetStatus(hstring const& message) {
  ActionStatusText().Text(message);
}

}  // namespace winrt::ClipLoom::SettingsPoC::implementation
