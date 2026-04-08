#ifdef _WIN32

#include "app/win32_app_internal.h"

namespace maccy {

namespace {

constexpr COLORREF kSettingsWindowBackground = RGB(246, 248, 252);
constexpr COLORREF kSettingsCardBackground = RGB(255, 255, 255);
constexpr COLORREF kSettingsCardBorder = RGB(220, 226, 236);
constexpr COLORREF kSettingsPrimaryText = RGB(26, 31, 43);
constexpr COLORREF kSettingsHintText = RGB(101, 112, 128);
constexpr COLORREF kSettingsAccentText = RGB(23, 88, 195);
constexpr COLORREF kSettingsInputBackground = RGB(255, 255, 255);
constexpr int kSettingsHeaderTop = 18;
constexpr int kSettingsHeaderHeight = 52;
constexpr int kSettingsOuterPadding = 16;
constexpr int kSettingsCardRadius = 14;
constexpr int kSettingsControlHeight = 28;
constexpr int kSettingsPrimaryButtonWidth = 108;

}  // namespace

void Win32App::OpenSettingsWindow() {
  active_double_click_modifier_flags_ = 0;
  double_click_modifier_detector_.Reset();

  if (settings_window_ != nullptr && IsWindow(settings_window_) != FALSE) {
    ShowWindow(settings_window_, IsIconic(settings_window_) != FALSE ? SW_RESTORE : SW_SHOWNORMAL);
    SetForegroundWindow(settings_window_);
    return;
  }

  constexpr DWORD window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
  constexpr DWORD window_ex_style = WS_EX_TOOLWINDOW | WS_EX_CONTROLPARENT;

  RECT window_rect{0, 0, kSettingsClientWidth, kSettingsClientHeight};
  AdjustWindowRectEx(&window_rect, window_style, FALSE, window_ex_style);

  settings_window_ = CreateWindowExW(
      window_ex_style,
      kSettingsWindowClass,
      UiText(use_chinese_ui_, L"ClipLoom Settings", L"ClipLoom 设置"),
      window_style,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      static_cast<int>(window_rect.right - window_rect.left),
      static_cast<int>(window_rect.bottom - window_rect.top),
      controller_window_,
      nullptr,
      instance_,
      this);
  if (settings_window_ == nullptr) {
    return;
  }

  SyncSettingsWindowControls();
  ShowWindow(settings_window_, SW_SHOWNORMAL);
  SetForegroundWindow(settings_window_);
}

void Win32App::CloseSettingsWindow() {
  if (settings_window_ != nullptr && IsWindow(settings_window_) != FALSE) {
    DestroyWindow(settings_window_);
  }
}

bool Win32App::ApplySettingsWindowChanges() {
  if (settings_window_ == nullptr || IsWindow(settings_window_) == FALSE) {
    return false;
  }

  AppSettings next_settings = settings_;
  const AppSettings previous_settings = settings_;
  const bool previous_capture_enabled = capture_enabled_;
  const bool next_capture_enabled = IsCheckboxChecked(settings_capture_enabled_check_);
  std::uint32_t next_hotkey_modifiers = 0;

  if (IsCheckboxChecked(settings_hotkey_ctrl_check_)) {
    next_hotkey_modifiers |= kHotKeyModControl;
  }
  if (IsCheckboxChecked(settings_hotkey_alt_check_)) {
    next_hotkey_modifiers |= kHotKeyModAlt;
  }
  if (IsCheckboxChecked(settings_hotkey_shift_check_)) {
    next_hotkey_modifiers |= kHotKeyModShift;
  }
  if (IsCheckboxChecked(settings_hotkey_win_check_)) {
    next_hotkey_modifiers |= kHotKeyModWin;
  }

  next_settings.popup_hotkey_modifiers = next_hotkey_modifiers;
  next_settings.popup_hotkey_virtual_key = PopupHotKeyVirtualKeyFromComboSelection(settings_hotkey_key_combo_);
  next_settings.double_click_popup_enabled = IsCheckboxChecked(settings_double_click_open_check_);
  next_settings.double_click_modifier_key = settings_double_click_modifier_selection_;
  const PopupOpenTriggerConfiguration requested_open_trigger = OpenTriggerConfigurationForSettings(next_settings);

  if (requested_open_trigger == PopupOpenTriggerConfiguration::kRegularShortcut &&
      !IsValidPopupHotKey(next_settings.popup_hotkey_modifiers, next_settings.popup_hotkey_virtual_key)) {
    ShowDialog(
        settings_window_,
        UiText(
            use_chinese_ui_,
            L"Choose at least one modifier key and one trigger key for the global open hotkey.",
            L"请至少选择一个组合键修饰键，以及一个用于打开窗口的触发按键。"),
        MB_ICONWARNING);
    return false;
  }

  next_settings.auto_paste = IsCheckboxChecked(settings_auto_paste_check_);
  next_settings.paste_plain_text = IsCheckboxChecked(settings_plain_text_check_);
  next_settings.start_on_login = IsCheckboxChecked(settings_start_on_login_check_);
  next_settings.show_startup_guide = IsCheckboxChecked(settings_show_startup_guide_check_);
  next_settings.clear_history_on_exit = IsCheckboxChecked(settings_clear_history_on_exit_check_);
  next_settings.clear_system_clipboard_on_exit = IsCheckboxChecked(settings_clear_clipboard_on_exit_check_);
  next_settings.popup.show_search = IsCheckboxChecked(settings_show_search_check_);
  next_settings.popup.show_preview = IsCheckboxChecked(settings_show_preview_check_);
  next_settings.popup.remember_last_position = IsCheckboxChecked(settings_remember_position_check_);
  if (!next_settings.popup.remember_last_position) {
    next_settings.popup.has_last_position = false;
  }

  next_settings.search_mode = SearchModeFromComboSelection(settings_search_mode_combo_);
  next_settings.sort_order = SortOrderFromComboSelection(settings_sort_order_combo_);
  next_settings.pin_position = PinPositionFromComboSelection(settings_pin_position_combo_);
  next_settings.max_history_items = HistoryLimitFromComboSelection(settings_history_limit_combo_);

  next_settings.ignore.ignore_all = IsCheckboxChecked(settings_ignore_all_check_);
  next_settings.ignore.only_listed_applications = IsCheckboxChecked(settings_only_listed_apps_check_);
  next_settings.ignore.capture_text = IsCheckboxChecked(settings_capture_text_check_);
  next_settings.ignore.capture_html = IsCheckboxChecked(settings_capture_html_check_);
  next_settings.ignore.capture_rtf = IsCheckboxChecked(settings_capture_rtf_check_);
  next_settings.ignore.capture_images = IsCheckboxChecked(settings_capture_images_check_);
  next_settings.ignore.capture_files = IsCheckboxChecked(settings_capture_files_check_);
  next_settings.ignore.ignored_applications = SplitMultilineText(ReadWindowText(settings_ignored_apps_edit_));
  next_settings.ignore.allowed_applications = SplitMultilineText(ReadWindowText(settings_allowed_apps_edit_));
  next_settings.ignore.ignored_patterns = SplitMultilineText(ReadWindowText(settings_ignored_patterns_edit_));
  next_settings.ignore.ignored_formats = SplitMultilineText(ReadWindowText(settings_ignored_formats_edit_));

  if (next_settings.start_on_login != settings_.start_on_login && !win32::SetStartOnLogin(next_settings.start_on_login)) {
    ShowDialog(
        settings_window_,
        UiText(
            use_chinese_ui_,
            L"Couldn't update the Start on Login setting. Check your Windows startup permissions and try again.",
            L"无法更新“开机启动”设置。请检查 Windows 启动权限后重试。"),
        MB_ICONERROR);
    return false;
  }

  settings_ = std::move(next_settings);
  capture_enabled_ = next_capture_enabled;
  if (!RefreshOpenTriggerRegistration()) {
    settings_ = previous_settings;
    capture_enabled_ = previous_capture_enabled;
    (void)RefreshOpenTriggerRegistration();

    std::wstring warning;
    if (requested_open_trigger == PopupOpenTriggerConfiguration::kDoubleClick) {
      warning = std::wstring(
                    UiText(
                        use_chinese_ui_,
                        L"Couldn't start the selected double-click open trigger. The previous open trigger ",
                        L"无法启动你选择的双击打开触发方式。已恢复之前的打开方式 ")) +
                DescribeOpenTrigger(use_chinese_ui_, previous_settings) + UiText(use_chinese_ui_, L" has been restored.", L"。");
    } else {
      warning = std::wstring(
                    UiText(
                        use_chinese_ui_,
                        L"Couldn't register the selected open hotkey. The previous open trigger ",
                        L"无法注册你选择的打开快捷键。已恢复之前的打开方式 ")) +
                DescribeOpenTrigger(use_chinese_ui_, previous_settings) + UiText(use_chinese_ui_, L" has been restored.", L"。");
    }
    ShowDialog(settings_window_, warning, MB_ICONERROR);
    SyncSettingsWindowControls();
    return false;
  }

  ApplyStoreOptions();
  PersistSettings();
  PersistHistory();
  RefreshPopupList();
  SyncPopupLayout();
  UpdatePreview();
  UpdateTrayIcon();
  SyncSettingsWindowControls();
  return true;
}

void Win32App::SetSettingsDoubleClickModifierSelection(DoubleClickModifierKey key) {
  settings_double_click_modifier_selection_ = key;

  if (settings_double_click_modifier_input_ == nullptr) {
    return;
  }

  const bool recorder_enabled =
      settings_double_click_open_check_ != nullptr && IsCheckboxChecked(settings_double_click_open_check_);
  const std::wstring text = DoubleClickModifierRecorderText(use_chinese_ui_, key, recorder_enabled);
  SetWindowTextW(settings_double_click_modifier_input_, text.c_str());
}

void Win32App::SyncSettingsWindowControls() {
  if (settings_window_ == nullptr || IsWindow(settings_window_) == FALSE) {
    return;
  }

  SetCheckboxChecked(settings_capture_enabled_check_, capture_enabled_);
  SetCheckboxChecked(settings_auto_paste_check_, settings_.auto_paste);
  SetCheckboxChecked(settings_plain_text_check_, settings_.paste_plain_text);
  SetCheckboxChecked(settings_start_on_login_check_, settings_.start_on_login);
  SetCheckboxChecked(settings_double_click_open_check_, settings_.double_click_popup_enabled);
  SetCheckboxChecked(settings_hotkey_ctrl_check_, (settings_.popup_hotkey_modifiers & kHotKeyModControl) != 0);
  SetCheckboxChecked(settings_hotkey_alt_check_, (settings_.popup_hotkey_modifiers & kHotKeyModAlt) != 0);
  SetCheckboxChecked(settings_hotkey_shift_check_, (settings_.popup_hotkey_modifiers & kHotKeyModShift) != 0);
  SetCheckboxChecked(settings_hotkey_win_check_, (settings_.popup_hotkey_modifiers & kHotKeyModWin) != 0);
  SetCheckboxChecked(settings_clear_history_on_exit_check_, settings_.clear_history_on_exit);
  SetCheckboxChecked(settings_clear_clipboard_on_exit_check_, settings_.clear_system_clipboard_on_exit);
  SetCheckboxChecked(settings_show_search_check_, settings_.popup.show_search);
  SetCheckboxChecked(settings_show_preview_check_, settings_.popup.show_preview);
  SetCheckboxChecked(settings_remember_position_check_, settings_.popup.remember_last_position);
  SetCheckboxChecked(settings_show_startup_guide_check_, settings_.show_startup_guide);
  SetCheckboxChecked(settings_ignore_all_check_, settings_.ignore.ignore_all);
  SetCheckboxChecked(settings_only_listed_apps_check_, settings_.ignore.only_listed_applications);
  SetCheckboxChecked(settings_capture_text_check_, settings_.ignore.capture_text);
  SetCheckboxChecked(settings_capture_html_check_, settings_.ignore.capture_html);
  SetCheckboxChecked(settings_capture_rtf_check_, settings_.ignore.capture_rtf);
  SetCheckboxChecked(settings_capture_images_check_, settings_.ignore.capture_images);
  SetCheckboxChecked(settings_capture_files_check_, settings_.ignore.capture_files);

  SetComboSelection(settings_hotkey_key_combo_, PopupHotKeyComboIndex(settings_.popup_hotkey_virtual_key));
  SetSettingsDoubleClickModifierSelection(settings_.double_click_modifier_key);
  SetComboSelection(settings_search_mode_combo_, SearchModeComboIndex(settings_.search_mode));
  SetComboSelection(settings_sort_order_combo_, SortOrderComboIndex(settings_.sort_order));
  SetComboSelection(settings_pin_position_combo_, PinPositionComboIndex(settings_.pin_position));
  SetComboSelection(settings_history_limit_combo_, HistoryLimitComboIndex(settings_.max_history_items));

  if (settings_double_click_modifier_input_ != nullptr) {
    EnableWindow(settings_double_click_modifier_input_, TRUE);
    SetSettingsDoubleClickModifierSelection(settings_.double_click_modifier_key);
  }

  const std::wstring ignored_apps = JoinMultilineText(settings_.ignore.ignored_applications);
  const std::wstring allowed_apps = JoinMultilineText(settings_.ignore.allowed_applications);
  const std::wstring ignored_patterns = JoinMultilineText(settings_.ignore.ignored_patterns);
  const std::wstring ignored_formats = JoinMultilineText(settings_.ignore.ignored_formats);

  if (settings_ignored_apps_edit_ != nullptr) {
    SetWindowTextW(settings_ignored_apps_edit_, ignored_apps.c_str());
  }
  if (settings_allowed_apps_edit_ != nullptr) {
    SetWindowTextW(settings_allowed_apps_edit_, allowed_apps.c_str());
  }
  if (settings_ignored_patterns_edit_ != nullptr) {
    SetWindowTextW(settings_ignored_patterns_edit_, ignored_patterns.c_str());
  }
  if (settings_ignored_formats_edit_ != nullptr) {
    SetWindowTextW(settings_ignored_formats_edit_, ignored_formats.c_str());
  }
}

void Win32App::ShowSettingsPage(int page_index) {
  const HWND pages[] = {
      settings_general_page_,
      settings_storage_page_,
      settings_appearance_page_,
      settings_pins_page_,
      settings_ignore_page_,
      settings_advanced_page_,
  };
  const int clamped_page_index = std::clamp(
      page_index,
      0,
      static_cast<int>(sizeof(pages) / sizeof(pages[0])) - 1);

  for (int index = 0; index < static_cast<int>(sizeof(pages) / sizeof(pages[0])); ++index) {
    if (pages[index] != nullptr) {
      ShowWindow(pages[index], index == clamped_page_index ? SW_SHOW : SW_HIDE);
    }
  }
}

LRESULT Win32App::HandleSettingsWindowMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_CREATE: {
      settings_window_ = window;
      const bool zh = use_chinese_ui_;
      settings_text_elements_.clear();
      settings_sections_.clear();

      if (settings_window_background_brush_ == nullptr) {
        settings_window_background_brush_ = CreateSolidBrush(kSettingsWindowBackground);
      }
      if (settings_card_brush_ == nullptr) {
        settings_card_brush_ = CreateSolidBrush(kSettingsCardBackground);
      }
      if (settings_input_brush_ == nullptr) {
        settings_input_brush_ = CreateSolidBrush(kSettingsInputBackground);
      }
      if (settings_card_border_pen_ == nullptr) {
        settings_card_border_pen_ = CreatePen(PS_SOLID, 1, kSettingsCardBorder);
      }

      const HFONT default_font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
      HDC window_dc = GetDC(window);
      const int dpi = window_dc != nullptr ? GetDeviceCaps(window_dc, LOGPIXELSY) : 96;
      if (window_dc != nullptr) {
        ReleaseDC(window, window_dc);
      }

      const auto create_font = [dpi](int point_size, int weight) {
        return CreateFontW(
            -MulDiv(point_size, dpi, 72),
            0,
            0,
            0,
            weight,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI");
      };

      if (settings_ui_font_ == nullptr) {
        settings_ui_font_ = create_font(10, FW_NORMAL);
      }
      if (settings_ui_semibold_font_ == nullptr) {
        settings_ui_semibold_font_ = create_font(10, FW_SEMIBOLD);
      }
      if (settings_ui_title_font_ == nullptr) {
        settings_ui_title_font_ = create_font(17, FW_SEMIBOLD);
      }

      const HFONT body_font = settings_ui_font_ != nullptr ? settings_ui_font_ : default_font;
      const HFONT semibold_font = settings_ui_semibold_font_ != nullptr ? settings_ui_semibold_font_ : default_font;
      const HFONT title_font = settings_ui_title_font_ != nullptr ? settings_ui_title_font_ : default_font;
      const auto apply_font = [](HWND control, HFONT font) {
        if (control != nullptr) {
          SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        }
      };
      const auto track_text = [&](HWND control, SettingsTextRole role, HFONT font) {
        if (control == nullptr) {
          return;
        }

        apply_font(control, font);
        settings_text_elements_.push_back({control, role});
      };

      const auto create_page = [&](HWND& target, const RECT& rect) {
        target = CreateWindowExW(
            WS_EX_CONTROLPARENT,
            L"STATIC",
            nullptr,
            WS_CHILD | WS_VISIBLE,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            settings_tab_,
            nullptr,
            instance_,
            nullptr);
        if (target != nullptr) {
          SetWindowLongPtrW(target, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
          const auto original_proc = reinterpret_cast<WNDPROC>(
              SetWindowLongPtrW(target, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StaticSettingsPageProc)));
          if (original_settings_page_proc_ == nullptr) {
            original_settings_page_proc_ = original_proc;
          }
        }
      };

      const auto create_card = [&](HWND parent, int x, int y, int width, int height, const wchar_t* title) {
        settings_sections_.push_back({parent, RECT{x, y, x + width, y + height}});
        HWND control = CreateWindowExW(
            0,
            L"STATIC",
            title,
            WS_CHILD | WS_VISIBLE,
            x + 18,
            y + 16,
            width - 36,
            20,
            parent,
            nullptr,
            instance_,
            nullptr);
        track_text(control, SettingsTextRole::kSectionTitle, semibold_font);
        return control;
      };

      const auto create_label = [&](HWND parent,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    const wchar_t* text,
                                    SettingsTextRole role = SettingsTextRole::kBody) {
        HWND control = CreateWindowExW(
            0,
            L"STATIC",
            text,
            WS_CHILD | WS_VISIBLE,
            x,
            y,
            width,
            height,
            parent,
            nullptr,
            instance_,
            nullptr);
        track_text(
            control,
            role,
            role == SettingsTextRole::kSectionTitle ? semibold_font : body_font);
        return control;
      };

      const auto create_checkbox =
          [&](HWND parent, HWND& target, int x, int y, int width, const wchar_t* text) {
            target = CreateWindowExW(
                0,
                L"BUTTON",
                text,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                x,
                y,
                width,
                24,
                parent,
                nullptr,
                instance_,
                nullptr);
            apply_font(target, body_font);
          };

      const auto create_combo = [&](HWND parent, HWND& target, int x, int y, int width) {
        target = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"COMBOBOX",
            nullptr,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
            x,
            y,
            width,
            240,
            parent,
            nullptr,
            instance_,
            nullptr);
        apply_font(target, body_font);
        if (target != nullptr) {
          SendMessageW(target, CB_SETITEMHEIGHT, static_cast<WPARAM>(-1), 24);
          SendMessageW(target, CB_SETITEMHEIGHT, 0, 20);
        }
      };

      const auto create_readonly_input = [&](HWND parent, HWND& target, int x, int y, int width) {
        target = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            nullptr,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            x,
            y,
            width,
            kSettingsControlHeight,
            parent,
            nullptr,
            instance_,
            nullptr);
        apply_font(target, body_font);
        if (target != nullptr) {
          SendMessageW(target, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(10, 10));
        }
      };

      const auto create_multiline_edit =
          [&](HWND parent, HWND& target, int x, int y, int width, int height) {
            target = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                L"EDIT",
                nullptr,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL,
                x,
                y,
                width,
                height,
                parent,
                nullptr,
                instance_,
                nullptr);
            apply_font(target, body_font);
            if (target != nullptr) {
              SendMessageW(target, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(10, 10));
            }
          };

      const int padding = kSettingsOuterPadding;
      const int button_width = kSettingsPrimaryButtonWidth;
      const int button_height = 32;
      const int header_top = kSettingsHeaderTop;
      const int tab_top = header_top + kSettingsHeaderHeight + 4;
      const int button_y = kSettingsClientHeight - padding - button_height;
      const int close_x = kSettingsClientWidth - padding - button_width;
      const int apply_x = close_x - 10 - button_width;
      const int save_x = apply_x - 10 - button_width;
      const int tab_height = button_y - tab_top - 14;

      settings_header_title_ = CreateWindowExW(
          0,
          L"STATIC",
          UiText(zh, L"ClipLoom Settings", L"ClipLoom 设置"),
          WS_CHILD | WS_VISIBLE,
          padding,
          header_top,
          340,
          30,
          window,
          nullptr,
          instance_,
          nullptr);
      apply_font(settings_header_title_, title_font);

      settings_header_subtitle_ = CreateWindowExW(
          0,
          L"STATIC",
          UiText(
              zh,
              L"Refresh the current Win32 interface first, then use it as the bridge toward a WinUI 3 shell.",
              L"先把当前 Win32 界面打磨到可用且更现代，再以它为基线推进到 WinUI 3 外壳。"),
          WS_CHILD | WS_VISIBLE,
          padding,
          header_top + 30,
          kSettingsClientWidth - padding * 2 - 220,
          20,
          window,
          nullptr,
          instance_,
          nullptr);
      apply_font(settings_header_subtitle_, body_font);

      settings_tab_ = CreateWindowExW(
          0,
          WC_TABCONTROLW,
          L"",
          WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP,
          padding,
          tab_top,
          kSettingsClientWidth - padding * 2,
          tab_height,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettingsTabControlId)),
          instance_,
          nullptr);
      apply_font(settings_tab_, semibold_font);

      const wchar_t* tab_titles[] = {
          UiText(zh, L"General", L"常规"),
          UiText(zh, L"Storage", L"存储"),
          UiText(zh, L"Appearance", L"外观"),
          UiText(zh, L"Pins", L"置顶"),
          UiText(zh, L"Ignore", L"忽略"),
          UiText(zh, L"Advanced", L"高级")};
      for (int index = 0; index < static_cast<int>(sizeof(tab_titles) / sizeof(tab_titles[0])); ++index) {
        TCITEMW item{};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<LPWSTR>(tab_titles[index]);
        TabCtrl_InsertItem(settings_tab_, index, &item);
      }

      RECT page_rect{};
      GetClientRect(settings_tab_, &page_rect);
      TabCtrl_AdjustRect(settings_tab_, FALSE, &page_rect);

      create_page(settings_general_page_, page_rect);
      create_page(settings_storage_page_, page_rect);
      create_page(settings_appearance_page_, page_rect);
      create_page(settings_pins_page_, page_rect);
      create_page(settings_ignore_page_, page_rect);
      create_page(settings_advanced_page_, page_rect);

      const int page_width = page_rect.right - page_rect.left;
      const int page_height = page_rect.bottom - page_rect.top;
      const int page_padding = 10;
      const int content_width = page_width - page_padding * 2;

      create_card(settings_general_page_, page_padding, 12, content_width, 208, UiText(zh, L"Open", L"打开方式"));
      create_label(
          settings_general_page_,
          page_padding + 16,
          48,
          content_width - 32,
          18,
          UiText(zh, L"Choose the global hotkey used to open the clipboard history popup.", L"选择用于打开剪贴板历史弹窗的全局快捷键。"),
          SettingsTextRole::kHint);
      create_checkbox(settings_general_page_, settings_hotkey_ctrl_check_, page_padding + 16, 78, 70, L"Ctrl");
      create_checkbox(settings_general_page_, settings_hotkey_alt_check_, page_padding + 92, 78, 60, L"Alt");
      create_checkbox(settings_general_page_, settings_hotkey_shift_check_, page_padding + 158, 78, 72, L"Shift");
      create_checkbox(settings_general_page_, settings_hotkey_win_check_, page_padding + 236, 78, 64, L"Win");
      create_combo(settings_general_page_, settings_hotkey_key_combo_, page_padding + 320, 74, 150);
      create_label(
          settings_general_page_,
          page_padding + 16,
          112,
          content_width - 32,
          18,
          UiText(zh, L"The previous hotkey is restored automatically if the new one can't be registered.", L"如果新的快捷键无法注册，程序会自动恢复之前的快捷键。"),
          SettingsTextRole::kHint);
      create_checkbox(
          settings_general_page_,
          settings_double_click_open_check_,
          page_padding + 16,
          142,
          content_width - 32,
          UiText(zh, L"Enable double-click modifier key to open", L"启用双击修饰键打开"));
      create_label(
          settings_general_page_,
          page_padding + 16,
          172,
          110,
          18,
          UiText(zh, L"Modifier key", L"修饰键"));
      create_readonly_input(settings_general_page_, settings_double_click_modifier_input_, page_padding + 132, 168, 220);
      create_label(
          settings_general_page_,
          page_padding + 16,
          198,
          content_width - 32,
          36,
          UiText(
              zh,
              L"When enabled, click the field and press Ctrl, Alt, or Shift to record it. Double-press that modifier to toggle clipboard history. Press Delete or Backspace to clear it.",
              L"启用后，点击输入框并按下 Ctrl、Alt 或 Shift 完成录入。之后连按两次该修饰键即可切换剪贴板历史弹窗。按 Delete 或 Backspace 可清除。"),
          SettingsTextRole::kHint);

      create_card(settings_general_page_, page_padding, 232, content_width, 178, UiText(zh, L"Behavior", L"行为"));
      int general_y = 268;
      create_checkbox(
          settings_general_page_,
          settings_capture_enabled_check_,
          page_padding + 16,
          general_y,
          content_width - 32,
          UiText(zh, L"Enable clipboard capture", L"启用剪贴板捕获"));
      general_y += 24;
      create_checkbox(
          settings_general_page_,
          settings_auto_paste_check_,
          page_padding + 16,
          general_y,
          content_width - 32,
          UiText(zh, L"Auto paste after selection", L"选择后自动粘贴"));
      general_y += 24;
      create_checkbox(
          settings_general_page_,
          settings_plain_text_check_,
          page_padding + 16,
          general_y,
          content_width - 32,
          UiText(zh, L"Paste as plain text", L"粘贴为纯文本"));
      general_y += 24;
      create_checkbox(
          settings_general_page_,
          settings_start_on_login_check_,
          page_padding + 16,
          general_y,
          content_width - 32,
          UiText(zh, L"Start on login", L"开机启动"));
      general_y += 24;
      create_checkbox(
          settings_general_page_,
          settings_show_startup_guide_check_,
          page_padding + 16,
          general_y,
          content_width - 32,
          UiText(zh, L"Show startup guide", L"显示启动引导"));

      create_card(settings_general_page_, page_padding, 422, content_width, 98, UiText(zh, L"Search", L"搜索"));
      create_label(settings_general_page_, page_padding + 16, 458, 110, 18, UiText(zh, L"Search mode", L"搜索模式"));
      create_combo(settings_general_page_, settings_search_mode_combo_, page_padding + 132, 454, 180);
      create_label(
          settings_general_page_,
          page_padding + 16,
          486,
          content_width - 32,
          18,
          UiText(zh, L"Matches the source project's exact, fuzzy, regexp, and mixed search modes.", L"对应源项目中的精确、模糊、正则和混合搜索模式。"),
          SettingsTextRole::kHint);

      create_card(settings_storage_page_, page_padding, 12, content_width, 138, UiText(zh, L"History", L"历史记录"));
      create_label(settings_storage_page_, page_padding + 16, 52, 110, 18, UiText(zh, L"History limit", L"历史条数"));
      create_combo(settings_storage_page_, settings_history_limit_combo_, page_padding + 132, 48, 180);
      create_label(settings_storage_page_, page_padding + 16, 88, 110, 18, UiText(zh, L"Sort order", L"排序方式"));
      create_combo(settings_storage_page_, settings_sort_order_combo_, page_padding + 132, 84, 180);

      create_card(settings_storage_page_, page_padding, 162, content_width, 188, UiText(zh, L"Saved content types", L"保存的内容类型"));
      int storage_y = 198;
      create_checkbox(
          settings_storage_page_,
          settings_capture_text_check_,
          page_padding + 16,
          storage_y,
          content_width - 32,
          UiText(zh, L"Save plain text", L"保存纯文本"));
      storage_y += 24;
      create_checkbox(
          settings_storage_page_,
          settings_capture_html_check_,
          page_padding + 16,
          storage_y,
          content_width - 32,
          UiText(zh, L"Save HTML", L"保存 HTML"));
      storage_y += 24;
      create_checkbox(
          settings_storage_page_,
          settings_capture_rtf_check_,
          page_padding + 16,
          storage_y,
          content_width - 32,
          UiText(zh, L"Save rich text", L"保存富文本"));
      storage_y += 24;
      create_checkbox(
          settings_storage_page_,
          settings_capture_images_check_,
          page_padding + 16,
          storage_y,
          content_width - 32,
          UiText(zh, L"Save images", L"保存图像"));
      storage_y += 24;
      create_checkbox(
          settings_storage_page_,
          settings_capture_files_check_,
          page_padding + 16,
          storage_y,
          content_width - 32,
          UiText(zh, L"Save file lists", L"保存文件列表"));

      create_card(settings_appearance_page_, page_padding, 12, content_width, 132, UiText(zh, L"Popup", L"弹窗"));
      int appearance_y = 48;
      create_checkbox(
          settings_appearance_page_,
          settings_show_search_check_,
          page_padding + 16,
          appearance_y,
          content_width - 32,
          UiText(zh, L"Show search field", L"显示搜索框"));
      appearance_y += 24;
      create_checkbox(
          settings_appearance_page_,
          settings_show_preview_check_,
          page_padding + 16,
          appearance_y,
          content_width - 32,
          UiText(zh, L"Show preview pane", L"显示预览区"));
      appearance_y += 24;
      create_checkbox(
          settings_appearance_page_,
          settings_remember_position_check_,
          page_padding + 16,
          appearance_y,
          content_width - 32,
          UiText(zh, L"Remember popup position", L"记住弹窗位置"));

      create_card(settings_appearance_page_, page_padding, 184, content_width, 96, UiText(zh, L"Pins", L"置顶"));
      create_label(settings_appearance_page_, page_padding + 16, 224, 110, 18, UiText(zh, L"Pin position", L"置顶位置"));
      create_combo(settings_appearance_page_, settings_pin_position_combo_, page_padding + 132, 220, 180);

      create_card(settings_pins_page_, page_padding, 12, content_width, 182, UiText(zh, L"Pinned items", L"置顶项目"));
      create_label(
          settings_pins_page_,
          page_padding + 16,
          52,
          content_width - 32,
          20,
          UiText(zh, L"Windows pin management is available directly from the history popup.", L"Windows 版的置顶管理可直接在历史弹窗中完成。"),
          SettingsTextRole::kHint);
      create_label(settings_pins_page_, page_padding + 16, 86, content_width - 32, 18, UiText(zh, L"Use Ctrl+P to pin or unpin the selected item.", L"按 Ctrl+P 可置顶或取消置顶当前选中项。"));
      create_label(settings_pins_page_, page_padding + 16, 114, content_width - 32, 18, UiText(zh, L"Use Ctrl+R to rename a pinned item.", L"按 Ctrl+R 可重命名置顶项。"));
      create_label(settings_pins_page_, page_padding + 16, 142, content_width - 32, 18, UiText(zh, L"Use Ctrl+E to edit pinned plain text.", L"按 Ctrl+E 可编辑置顶的纯文本内容。"));

      create_card(settings_ignore_page_, page_padding, 12, content_width, 96, UiText(zh, L"Ignore behavior", L"忽略行为"));
      create_checkbox(
          settings_ignore_page_,
          settings_ignore_all_check_,
          page_padding + 16,
          48,
          content_width - 32,
          UiText(zh, L"Ignore all clipboard captures", L"忽略所有剪贴板捕获"));
      create_checkbox(
          settings_ignore_page_,
          settings_only_listed_apps_check_,
          page_padding + 16,
          74,
          content_width - 32,
          UiText(zh, L"Only capture applications listed in the allowed applications box", L"仅捕获“允许的应用程序”列表中列出的应用"));

      const int ignore_column_gap = 12;
      const int ignore_column_width = (content_width - ignore_column_gap) / 2;
      const int ignore_edit_height = 132;
      create_card(settings_ignore_page_, page_padding, 120, content_width, page_height - 132, UiText(zh, L"Rules", L"规则"));

      create_label(
          settings_ignore_page_,
          page_padding + 16,
          158,
          ignore_column_width - 8,
          18,
          UiText(zh, L"Ignored applications", L"忽略的应用程序"));
      create_multiline_edit(
          settings_ignore_page_,
          settings_ignored_apps_edit_,
          page_padding + 16,
          182,
          ignore_column_width - 8,
          ignore_edit_height);

      create_label(
          settings_ignore_page_,
          page_padding + ignore_column_width + ignore_column_gap + 8,
          158,
          ignore_column_width - 8,
          18,
          UiText(zh, L"Allowed applications", L"允许的应用程序"));
      create_multiline_edit(
          settings_ignore_page_,
          settings_allowed_apps_edit_,
          page_padding + ignore_column_width + ignore_column_gap + 8,
          182,
          ignore_column_width - 8,
          ignore_edit_height);

      create_label(
          settings_ignore_page_,
          page_padding + 16,
          334,
          ignore_column_width - 8,
          18,
          UiText(zh, L"Ignored text patterns", L"忽略的文本模式"));
      create_multiline_edit(
          settings_ignore_page_,
          settings_ignored_patterns_edit_,
          page_padding + 16,
          358,
          ignore_column_width - 8,
          ignore_edit_height);

      create_label(
          settings_ignore_page_,
          page_padding + ignore_column_width + ignore_column_gap + 8,
          334,
          ignore_column_width - 8,
          18,
          UiText(zh, L"Ignored content formats", L"忽略的内容格式"));
      create_multiline_edit(
          settings_ignore_page_,
          settings_ignored_formats_edit_,
          page_padding + ignore_column_width + ignore_column_gap + 8,
          358,
          ignore_column_width - 8,
          ignore_edit_height);

      create_card(settings_advanced_page_, page_padding, 12, content_width, 102, UiText(zh, L"Advanced", L"高级"));
      create_checkbox(
          settings_advanced_page_,
          settings_clear_history_on_exit_check_,
          page_padding + 16,
          48,
          content_width - 32,
          UiText(zh, L"Clear unpinned history when the app exits", L"应用退出时清除未置顶历史记录"));
      create_checkbox(
          settings_advanced_page_,
          settings_clear_clipboard_on_exit_check_,
          page_padding + 16,
          74,
          content_width - 32,
          UiText(zh, L"Clear the Windows clipboard when the app exits", L"应用退出时清空 Windows 剪贴板"));

      create_card(settings_advanced_page_, page_padding, 126, content_width, 124, UiText(zh, L"Notes", L"说明"));
      create_label(
          settings_advanced_page_,
          page_padding + 16,
          166,
          content_width - 32,
          20,
          UiText(zh, L"These options match the source project's advanced housekeeping behavior.", L"这些选项对应源项目中的高级清理行为。"),
          SettingsTextRole::kHint);
      create_label(
          settings_advanced_page_,
          page_padding + 16,
          194,
          content_width - 32,
          36,
          UiText(zh, L"History clearing keeps pinned items. Clipboard clearing removes the current clipboard contents on shutdown.", L"清理历史记录时会保留置顶项。清空剪贴板会在退出时移除当前系统剪贴板内容。"),
          SettingsTextRole::kHint);

      for (const auto& choice : kPopupHotKeyChoices) {
        AddComboItem(settings_hotkey_key_combo_, PopupHotKeyLabel(zh, choice.virtual_key));
      }
      AddComboItem(settings_search_mode_combo_, SearchModeLabel(zh, SearchMode::kMixed));
      AddComboItem(settings_search_mode_combo_, SearchModeLabel(zh, SearchMode::kExact));
      AddComboItem(settings_search_mode_combo_, SearchModeLabel(zh, SearchMode::kFuzzy));
      AddComboItem(settings_search_mode_combo_, SearchModeLabel(zh, SearchMode::kRegexp));
      AddComboItem(settings_sort_order_combo_, SortOrderLabel(zh, HistorySortOrder::kLastCopied));
      AddComboItem(settings_sort_order_combo_, SortOrderLabel(zh, HistorySortOrder::kFirstCopied));
      AddComboItem(settings_sort_order_combo_, SortOrderLabel(zh, HistorySortOrder::kCopyCount));
      AddComboItem(settings_pin_position_combo_, PinPositionLabel(zh, PinPosition::kTop));
      AddComboItem(settings_pin_position_combo_, PinPositionLabel(zh, PinPosition::kBottom));
      AddComboItem(settings_history_limit_combo_, L"50");
      AddComboItem(settings_history_limit_combo_, L"100");
      AddComboItem(settings_history_limit_combo_, L"200");
      AddComboItem(settings_history_limit_combo_, L"500");

      HWND save_button = CreateWindowExW(
          0,
          L"BUTTON",
          UiText(zh, L"Save", L"保存"),
          WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
          save_x,
          button_y,
          button_width,
          button_height,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettingsSaveButtonId)),
          instance_,
          nullptr);
      apply_font(save_button, semibold_font);

      HWND apply_button = CreateWindowExW(
          0,
          L"BUTTON",
          UiText(zh, L"Apply", L"应用"),
          WS_CHILD | WS_VISIBLE | WS_TABSTOP,
          apply_x,
          button_y,
          button_width,
          button_height,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettingsApplyButtonId)),
          instance_,
          nullptr);
      apply_font(apply_button, body_font);

      HWND close_button = CreateWindowExW(
          0,
          L"BUTTON",
          UiText(zh, L"Close", L"关闭"),
          WS_CHILD | WS_VISIBLE | WS_TABSTOP,
          close_x,
          button_y,
          button_width,
          button_height,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettingsCloseButtonId)),
          instance_,
          nullptr);
      apply_font(close_button, body_font);

      if (settings_double_click_modifier_input_ != nullptr) {
        SetWindowLongPtrW(settings_double_click_modifier_input_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        original_settings_double_click_modifier_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
            settings_double_click_modifier_input_,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(StaticSettingsDoubleClickModifierProc)));
      }

      TabCtrl_SetCurSel(settings_tab_, 0);
      ShowSettingsPage(0);
      SyncSettingsWindowControls();
      return 0;
    }
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT: {
      PAINTSTRUCT paint{};
      HDC dc = BeginPaint(window, &paint);
      RECT client_rect{};
      GetClientRect(window, &client_rect);
      FillRect(
          dc,
          &client_rect,
          settings_window_background_brush_ != nullptr
              ? settings_window_background_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

      RECT divider{
          kSettingsOuterPadding,
          kSettingsClientHeight - kSettingsOuterPadding - 40,
          kSettingsClientWidth - kSettingsOuterPadding,
          kSettingsClientHeight - kSettingsOuterPadding - 39,
      };
      FillRect(
          dc,
          &divider,
          settings_card_brush_ != nullptr
              ? settings_card_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
      EndPaint(window, &paint);
      return 0;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = reinterpret_cast<HDC>(wparam);
      const HWND control = reinterpret_cast<HWND>(lparam);
      SetBkMode(dc, TRANSPARENT);
      if (control == settings_header_title_) {
        SetTextColor(dc, kSettingsPrimaryText);
      } else if (control == settings_header_subtitle_) {
        SetTextColor(dc, kSettingsHintText);
      }
      return reinterpret_cast<INT_PTR>(
          settings_window_background_brush_ != nullptr
              ? settings_window_background_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    }
    case WM_NOTIFY: {
      const auto* header = reinterpret_cast<const NMHDR*>(lparam);
      if (header != nullptr && header->hwndFrom == settings_tab_ && header->code == TCN_SELCHANGE) {
        ShowSettingsPage(TabCtrl_GetCurSel(settings_tab_));
        return 0;
      }
      break;
    }
    case WM_COMMAND:
      if (reinterpret_cast<HWND>(lparam) == settings_double_click_open_check_ && HIWORD(wparam) == BN_CLICKED) {
        if (settings_double_click_modifier_input_ != nullptr) {
          SetSettingsDoubleClickModifierSelection(settings_double_click_modifier_selection_);
          if (IsCheckboxChecked(settings_double_click_open_check_)) {
            SetFocus(settings_double_click_modifier_input_);
          }
        }
        return 0;
      }
      switch (LOWORD(wparam)) {
        case kSettingsSaveButtonId:
          if (ApplySettingsWindowChanges()) {
            CloseSettingsWindow();
          }
          return 0;
        case kSettingsApplyButtonId:
          ApplySettingsWindowChanges();
          return 0;
        case kSettingsCloseButtonId:
          CloseSettingsWindow();
          return 0;
        default:
          break;
      }
      break;
    case WM_CLOSE:
      CloseSettingsWindow();
      return 0;
    case WM_DESTROY:
      if (settings_window_ == window) {
        settings_window_ = nullptr;
      }
      settings_header_title_ = nullptr;
      settings_header_subtitle_ = nullptr;
      settings_tab_ = nullptr;
      settings_general_page_ = nullptr;
      settings_storage_page_ = nullptr;
      settings_appearance_page_ = nullptr;
      settings_pins_page_ = nullptr;
      settings_ignore_page_ = nullptr;
      settings_advanced_page_ = nullptr;
      settings_capture_enabled_check_ = nullptr;
      settings_auto_paste_check_ = nullptr;
      settings_plain_text_check_ = nullptr;
      settings_start_on_login_check_ = nullptr;
      settings_double_click_open_check_ = nullptr;
      settings_double_click_modifier_input_ = nullptr;
      settings_hotkey_ctrl_check_ = nullptr;
      settings_hotkey_alt_check_ = nullptr;
      settings_hotkey_shift_check_ = nullptr;
      settings_hotkey_win_check_ = nullptr;
      settings_hotkey_key_combo_ = nullptr;
      settings_show_search_check_ = nullptr;
      settings_show_preview_check_ = nullptr;
      settings_remember_position_check_ = nullptr;
      settings_show_startup_guide_check_ = nullptr;
      settings_ignore_all_check_ = nullptr;
      settings_only_listed_apps_check_ = nullptr;
      settings_capture_text_check_ = nullptr;
      settings_capture_html_check_ = nullptr;
      settings_capture_rtf_check_ = nullptr;
      settings_capture_images_check_ = nullptr;
      settings_capture_files_check_ = nullptr;
      settings_clear_history_on_exit_check_ = nullptr;
      settings_clear_clipboard_on_exit_check_ = nullptr;
      settings_search_mode_combo_ = nullptr;
      settings_sort_order_combo_ = nullptr;
      settings_pin_position_combo_ = nullptr;
      settings_history_limit_combo_ = nullptr;
      settings_ignored_apps_edit_ = nullptr;
      settings_allowed_apps_edit_ = nullptr;
      settings_ignored_patterns_edit_ = nullptr;
      settings_ignored_formats_edit_ = nullptr;
      settings_text_elements_.clear();
      settings_sections_.clear();
      if (settings_ui_font_ != nullptr) {
        DeleteObject(settings_ui_font_);
        settings_ui_font_ = nullptr;
      }
      if (settings_ui_semibold_font_ != nullptr) {
        DeleteObject(settings_ui_semibold_font_);
        settings_ui_semibold_font_ = nullptr;
      }
      if (settings_ui_title_font_ != nullptr) {
        DeleteObject(settings_ui_title_font_);
        settings_ui_title_font_ = nullptr;
      }
      if (settings_window_background_brush_ != nullptr) {
        DeleteObject(settings_window_background_brush_);
        settings_window_background_brush_ = nullptr;
      }
      if (settings_card_brush_ != nullptr) {
        DeleteObject(settings_card_brush_);
        settings_card_brush_ = nullptr;
      }
      if (settings_input_brush_ != nullptr) {
        DeleteObject(settings_input_brush_);
        settings_input_brush_ = nullptr;
      }
      if (settings_card_border_pen_ != nullptr) {
        DeleteObject(settings_card_border_pen_);
        settings_card_border_pen_ = nullptr;
      }
      original_settings_page_proc_ = nullptr;
      original_settings_double_click_modifier_proc_ = nullptr;
      return 0;
    default:
      break;
  }

  return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT Win32App::HandleSettingsPageMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT: {
      PAINTSTRUCT paint{};
      HDC dc = BeginPaint(window, &paint);

      RECT client_rect{};
      GetClientRect(window, &client_rect);
      FillRect(
          dc,
          &client_rect,
          settings_window_background_brush_ != nullptr
              ? settings_window_background_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

      if (settings_card_brush_ != nullptr && settings_card_border_pen_ != nullptr) {
        const HGDIOBJ old_brush = SelectObject(dc, settings_card_brush_);
        const HGDIOBJ old_pen = SelectObject(dc, settings_card_border_pen_);
        for (const auto& section : settings_sections_) {
          if (section.page != window) {
            continue;
          }

          RoundRect(
              dc,
              section.rect.left,
              section.rect.top,
              section.rect.right,
              section.rect.bottom,
              kSettingsCardRadius,
              kSettingsCardRadius);
        }
        SelectObject(dc, old_pen);
        SelectObject(dc, old_brush);
      }

      EndPaint(window, &paint);
      return 0;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = reinterpret_cast<HDC>(wparam);
      const HWND control = reinterpret_cast<HWND>(lparam);
      SetBkMode(dc, TRANSPARENT);
      COLORREF text_color = kSettingsPrimaryText;
      for (const auto& text_element : settings_text_elements_) {
        if (text_element.handle != control) {
          continue;
        }

        switch (text_element.role) {
          case SettingsTextRole::kSectionTitle:
            text_color = kSettingsAccentText;
            break;
          case SettingsTextRole::kHint:
            text_color = kSettingsHintText;
            break;
          case SettingsTextRole::kBody:
          default:
            text_color = kSettingsPrimaryText;
            break;
        }
        break;
      }
      SetTextColor(dc, text_color);
      return reinterpret_cast<INT_PTR>(GetStockObject(NULL_BRUSH));
    }
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
      HDC dc = reinterpret_cast<HDC>(wparam);
      SetTextColor(dc, kSettingsPrimaryText);
      SetBkColor(dc, kSettingsInputBackground);
      return reinterpret_cast<INT_PTR>(
          settings_input_brush_ != nullptr
              ? settings_input_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    }
    case WM_CTLCOLORBTN: {
      HDC dc = reinterpret_cast<HDC>(wparam);
      SetBkMode(dc, TRANSPARENT);
      SetTextColor(dc, kSettingsPrimaryText);
      return reinterpret_cast<INT_PTR>(
          settings_card_brush_ != nullptr
              ? settings_card_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    }
    default:
      break;
  }

  if (original_settings_page_proc_ != nullptr) {
    return CallWindowProcW(original_settings_page_proc_, window, message, wparam, lparam);
  }

  return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT Win32App::HandleSettingsDoubleClickModifierMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_GETDLGCODE: {
      const LRESULT dialog_code = CallWindowProcW(
          original_settings_double_click_modifier_proc_,
          window,
          message,
          wparam,
          lparam);
      return dialog_code | DLGC_WANTALLKEYS;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
      if (const auto key = DoubleClickModifierKeyFromVirtualKey(static_cast<DWORD>(wparam));
          key != DoubleClickModifierKey::kNone) {
        SetSettingsDoubleClickModifierSelection(key);
        return 0;
      }

      switch (wparam) {
        case VK_DELETE:
        case VK_BACK:
        case VK_ESCAPE:
          SetSettingsDoubleClickModifierSelection(DoubleClickModifierKey::kNone);
          return 0;
        default:
          break;
      }
      break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_CHAR:
    case WM_SYSCHAR:
      return 0;
    default:
      break;
  }

  return CallWindowProcW(original_settings_double_click_modifier_proc_, window, message, wparam, lparam);
}



}  // namespace maccy

#endif  // _WIN32
