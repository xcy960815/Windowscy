#pragma once

#include "Views/GeneralPage.g.h"

namespace winrt::ClipLoom::SettingsPoC::Views::implementation {

struct GeneralPage : GeneralPageT<GeneralPage> {
  GeneralPage();
};

}  // namespace winrt::ClipLoom::SettingsPoC::Views::implementation

namespace winrt::ClipLoom::SettingsPoC::Views::factory_implementation {

struct GeneralPage : GeneralPageT<GeneralPage, implementation::GeneralPage> {};

}  // namespace winrt::ClipLoom::SettingsPoC::Views::factory_implementation
