#include "App.xaml.h"

#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::ClipLoom::SettingsPoC::implementation {

App::App() {
  InitializeComponent();
}

void App::OnLaunched(LaunchActivatedEventArgs const&) {
  m_window = make<MainWindow>();
  m_window.Activate();
}

}  // namespace winrt::ClipLoom::SettingsPoC::implementation
