#pragma once

#include "Views/AppearancePage.g.h"

namespace winrt::ClipLoom::SettingsPoC::Views::implementation {

struct AppearancePage : AppearancePageT<AppearancePage> {
  AppearancePage();
};

}  // namespace winrt::ClipLoom::SettingsPoC::Views::implementation

namespace winrt::ClipLoom::SettingsPoC::Views::factory_implementation {

struct AppearancePage : AppearancePageT<AppearancePage, implementation::AppearancePage> {};

}  // namespace winrt::ClipLoom::SettingsPoC::Views::factory_implementation
