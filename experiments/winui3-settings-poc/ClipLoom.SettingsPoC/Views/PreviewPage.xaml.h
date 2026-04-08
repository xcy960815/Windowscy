#pragma once

#include "Views/PreviewPage.g.h"

namespace winrt::ClipLoom::SettingsPoC::Views::implementation {

struct PreviewPage : PreviewPageT<PreviewPage> {
  PreviewPage();
};

}  // namespace winrt::ClipLoom::SettingsPoC::Views::implementation

namespace winrt::ClipLoom::SettingsPoC::Views::factory_implementation {

struct PreviewPage : PreviewPageT<PreviewPage, implementation::PreviewPage> {};

}  // namespace winrt::ClipLoom::SettingsPoC::Views::factory_implementation
