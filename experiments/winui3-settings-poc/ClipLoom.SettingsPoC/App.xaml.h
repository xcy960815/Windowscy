#pragma once

#include "App.xaml.g.h"

namespace winrt::ClipLoom::SettingsPoC::implementation {

struct App : AppT<App> {
  App();

  void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const& args);

 private:
  Microsoft::UI::Xaml::Window m_window{nullptr};
};

}  // namespace winrt::ClipLoom::SettingsPoC::implementation

namespace winrt::ClipLoom::SettingsPoC::factory_implementation {

struct App : AppT<App, implementation::App> {};

}  // namespace winrt::ClipLoom::SettingsPoC::factory_implementation
