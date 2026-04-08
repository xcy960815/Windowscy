#pragma once

#include "MainWindow.g.h"

#include "ViewModels/SettingsViewModel.h"

namespace winrt::ClipLoom::SettingsPoC::implementation {

struct MainWindow : MainWindowT<MainWindow> {
  MainWindow();

  void NavigationViewSelectionChanged(
      Microsoft::UI::Xaml::Controls::NavigationView const& sender,
      Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);
  void SaveButtonClick(
      winrt::Windows::Foundation::IInspectable const& sender,
      Microsoft::UI::Xaml::RoutedEventArgs const& args);
  void ApplyButtonClick(
      winrt::Windows::Foundation::IInspectable const& sender,
      Microsoft::UI::Xaml::RoutedEventArgs const& args);
  void CloseButtonClick(
      winrt::Windows::Foundation::IInspectable const& sender,
      Microsoft::UI::Xaml::RoutedEventArgs const& args);

 private:
  ClipLoom::SettingsPoC::ViewModels::SettingsViewModel m_view_model;

  void NavigateToTag(winrt::hstring const& tag);
  void SetStatus(winrt::hstring const& message);
};

}  // namespace winrt::ClipLoom::SettingsPoC::implementation

namespace winrt::ClipLoom::SettingsPoC::factory_implementation {

struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow> {};

}  // namespace winrt::ClipLoom::SettingsPoC::factory_implementation
