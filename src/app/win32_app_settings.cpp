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
constexpr COLORREF kSettingsAccentFill = RGB(87, 129, 224);
constexpr COLORREF kSettingsSidebarBackground = RGB(235, 240, 247);
constexpr COLORREF kSettingsSidebarBorder = RGB(214, 221, 232);
constexpr COLORREF kSettingsSidebarBrandBackground = RGB(224, 232, 244);
constexpr COLORREF kSettingsSidebarSummaryBackground = RGB(248, 250, 253);
constexpr COLORREF kSettingsNavActiveBackground = RGB(255, 255, 255);
constexpr COLORREF kSettingsNavHoverBackground = RGB(243, 247, 255);
constexpr COLORREF kSettingsNavText = RGB(56, 68, 86);
constexpr COLORREF kSettingsButtonPrimary = RGB(28, 99, 222);
constexpr COLORREF kSettingsButtonPrimaryBorder = RGB(19, 79, 190);
constexpr COLORREF kSettingsButtonSecondaryBackground = RGB(255, 255, 255);
constexpr COLORREF kSettingsFieldBorder = RGB(206, 214, 226);
constexpr COLORREF kSettingsCheckboxBorder = RGB(148, 160, 178);
constexpr COLORREF kSettingsCheckboxCheck = RGB(255, 255, 255);
constexpr COLORREF kSettingsInputBackground = RGB(255, 255, 255);
constexpr int kSettingsHeaderTop = 18;
constexpr int kSettingsHeaderHeight = 52;
constexpr int kSettingsOuterPadding = 16;
constexpr int kSettingsCardRadius = 14;
constexpr int kSettingsControlHeight = 28;
constexpr int kSettingsPrimaryButtonWidth = 108;
constexpr int kSettingsActionButtonHeight = 32;
constexpr int kSettingsSidebarWidth = 168;
constexpr int kSettingsNavButtonHeight = 42;
constexpr int kSettingsSidebarBrandHeight = 82;
constexpr int kSettingsSidebarSummaryHeight = 116;

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
  settings_current_page_index_ = clamped_page_index;

  for (int index = 0; index < static_cast<int>(sizeof(pages) / sizeof(pages[0])); ++index) {
    if (pages[index] != nullptr) {
      ShowWindow(pages[index], index == clamped_page_index ? SW_SHOW : SW_HIDE);
    }
  }

  const HWND nav_buttons[] = {
      settings_nav_general_button_,
      settings_nav_storage_button_,
      settings_nav_appearance_button_,
      settings_nav_pins_button_,
      settings_nav_ignore_button_,
      settings_nav_advanced_button_,
  };
  for (HWND button : nav_buttons) {
    if (button != nullptr) {
      InvalidateRect(button, nullptr, TRUE);
    }
  }
  if (settings_window_ != nullptr) {
    InvalidateRect(settings_window_, nullptr, TRUE);
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
            window,
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
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_OWNERDRAW,
                x,
                y,
                width,
                28,
                parent,
                nullptr,
                instance_,
                nullptr);
            apply_font(target, body_font);
          };

      const auto create_combo = [&](HWND parent, HWND& target, int x, int y, int width) {
        target = CreateWindowExW(
            0,
            L"COMBOBOX",
            nullptr,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL | WS_BORDER,
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
            0,
            L"EDIT",
            nullptr,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | WS_BORDER,
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
                0,
                L"EDIT",
                nullptr,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL |
                    WS_BORDER,
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

      const auto create_nav_button = [&](HWND& target, int id, int x, int y, int width, const wchar_t* text) {
        target = CreateWindowExW(
            0,
            L"BUTTON",
            text,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
            x,
            y,
            width,
            kSettingsNavButtonHeight,
            window,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
            instance_,
            nullptr);
        apply_font(target, semibold_font);
      };

      const int padding = kSettingsOuterPadding;
      const int button_width = kSettingsPrimaryButtonWidth;
      const int button_height = kSettingsActionButtonHeight;
      const int header_top = kSettingsHeaderTop;
      const int content_top = header_top + kSettingsHeaderHeight + 20;
      const int button_y = kSettingsClientHeight - padding - button_height;
      const int close_x = kSettingsClientWidth - padding - button_width;
      const int apply_x = close_x - 10 - button_width;
      const int save_x = apply_x - 10 - button_width;
      const int content_bottom = button_y - 18;
      const int nav_x = padding;
      const int nav_y = content_top + kSettingsSidebarBrandHeight + 18;
      const int nav_width = kSettingsSidebarWidth;
      const int nav_gap = 8;
      const int content_x = nav_x + nav_width + 18;
      const int content_area_width = kSettingsClientWidth - content_x - padding;
      const int content_height = content_bottom - content_top;

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
              L"Fine-tune how ClipLoom opens, stores, and presents your clipboard history.",
              L"在这里统一调整 ClipLoom 的打开方式、历史保存策略和界面呈现。"),
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

      const wchar_t* tab_titles[] = {
          UiText(zh, L"General", L"常规"),
          UiText(zh, L"Storage", L"存储"),
          UiText(zh, L"Appearance", L"外观"),
          UiText(zh, L"Pins", L"置顶"),
          UiText(zh, L"Ignore", L"忽略"),
          UiText(zh, L"Advanced", L"高级")};

      create_nav_button(
          settings_nav_general_button_,
          kSettingsNavGeneralButtonId,
          nav_x,
          nav_y,
          nav_width,
          tab_titles[0]);
      create_nav_button(
          settings_nav_storage_button_,
          kSettingsNavStorageButtonId,
          nav_x,
          nav_y + (kSettingsNavButtonHeight + nav_gap) * 1,
          nav_width,
          tab_titles[1]);
      create_nav_button(
          settings_nav_appearance_button_,
          kSettingsNavAppearanceButtonId,
          nav_x,
          nav_y + (kSettingsNavButtonHeight + nav_gap) * 2,
          nav_width,
          tab_titles[2]);
      create_nav_button(
          settings_nav_pins_button_,
          kSettingsNavPinsButtonId,
          nav_x,
          nav_y + (kSettingsNavButtonHeight + nav_gap) * 3,
          nav_width,
          tab_titles[3]);
      create_nav_button(
          settings_nav_ignore_button_,
          kSettingsNavIgnoreButtonId,
          nav_x,
          nav_y + (kSettingsNavButtonHeight + nav_gap) * 4,
          nav_width,
          tab_titles[4]);
      create_nav_button(
          settings_nav_advanced_button_,
          kSettingsNavAdvancedButtonId,
          nav_x,
          nav_y + (kSettingsNavButtonHeight + nav_gap) * 5,
          nav_width,
          tab_titles[5]);

      RECT page_rect{
          content_x,
          content_top,
          content_x + content_area_width,
          content_top + content_height,
      };

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

      settings_save_button_ = CreateWindowExW(
          0,
          L"BUTTON",
          UiText(zh, L"Save", L"保存"),
          WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
          save_x,
          button_y,
          button_width,
          button_height,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettingsSaveButtonId)),
          instance_,
          nullptr);
      apply_font(settings_save_button_, semibold_font);

      settings_apply_button_ = CreateWindowExW(
          0,
          L"BUTTON",
          UiText(zh, L"Apply", L"应用"),
          WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
          apply_x,
          button_y,
          button_width,
          button_height,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettingsApplyButtonId)),
          instance_,
          nullptr);
      apply_font(settings_apply_button_, body_font);

      settings_close_button_ = CreateWindowExW(
          0,
          L"BUTTON",
          UiText(zh, L"Close", L"关闭"),
          WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
          close_x,
          button_y,
          button_width,
          button_height,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettingsCloseButtonId)),
          instance_,
          nullptr);
      apply_font(settings_close_button_, body_font);

      if (settings_double_click_modifier_input_ != nullptr) {
        SetWindowLongPtrW(settings_double_click_modifier_input_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        original_settings_double_click_modifier_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
            settings_double_click_modifier_input_,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(StaticSettingsDoubleClickModifierProc)));
      }

      settings_current_page_index_ = 0;
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

      RECT sidebar_rect{
          kSettingsOuterPadding,
          kSettingsHeaderTop + kSettingsHeaderHeight + 20,
          kSettingsOuterPadding + kSettingsSidebarWidth,
          kSettingsClientHeight - kSettingsOuterPadding - kSettingsActionButtonHeight - 18,
      };
      HBRUSH sidebar_brush = CreateSolidBrush(kSettingsSidebarBackground);
      FillRect(dc, &sidebar_rect, sidebar_brush);
      DeleteObject(sidebar_brush);

      RECT sidebar_border{
          sidebar_rect.right,
          sidebar_rect.top,
          sidebar_rect.right + 1,
          sidebar_rect.bottom,
      };
      HBRUSH sidebar_border_brush = CreateSolidBrush(kSettingsSidebarBorder);
      FillRect(dc, &sidebar_border, sidebar_border_brush);
      DeleteObject(sidebar_border_brush);

      RECT sidebar_brand_rect{
          sidebar_rect.left + 12,
          sidebar_rect.top + 12,
          sidebar_rect.right - 12,
          sidebar_rect.top + 12 + kSettingsSidebarBrandHeight,
      };
      HBRUSH sidebar_brand_brush = CreateSolidBrush(kSettingsSidebarBrandBackground);
      HPEN sidebar_brand_pen = CreatePen(PS_SOLID, 1, kSettingsSidebarBorder);
      const HGDIOBJ old_brand_brush = SelectObject(dc, sidebar_brand_brush);
      const HGDIOBJ old_brand_pen = SelectObject(dc, sidebar_brand_pen);
      RoundRect(
          dc,
          sidebar_brand_rect.left,
          sidebar_brand_rect.top,
          sidebar_brand_rect.right,
          sidebar_brand_rect.bottom,
          18,
          18);
      SelectObject(dc, old_brand_pen);
      SelectObject(dc, old_brand_brush);
      DeleteObject(sidebar_brand_pen);
      DeleteObject(sidebar_brand_brush);

      RECT brand_badge_rect{
          sidebar_brand_rect.left + 12,
          sidebar_brand_rect.top + 14,
          sidebar_brand_rect.left + 52,
          sidebar_brand_rect.top + 54,
      };
      HBRUSH brand_badge_brush = CreateSolidBrush(kSettingsButtonPrimary);
      HPEN brand_badge_pen = CreatePen(PS_SOLID, 1, kSettingsButtonPrimaryBorder);
      const HGDIOBJ old_badge_brush = SelectObject(dc, brand_badge_brush);
      const HGDIOBJ old_badge_pen = SelectObject(dc, brand_badge_pen);
      RoundRect(
          dc,
          brand_badge_rect.left,
          brand_badge_rect.top,
          brand_badge_rect.right,
          brand_badge_rect.bottom,
          14,
          14);
      SelectObject(dc, old_badge_pen);
      SelectObject(dc, old_badge_brush);
      DeleteObject(brand_badge_pen);
      DeleteObject(brand_badge_brush);

      const HGDIOBJ old_font = SelectObject(
          dc,
          settings_ui_semibold_font_ != nullptr ? settings_ui_semibold_font_ : GetStockObject(DEFAULT_GUI_FONT));

      SetTextColor(dc, RGB(255, 255, 255));
      RECT badge_text_rect = brand_badge_rect;
      DrawTextW(dc, L"CL", -1, &badge_text_rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

      SetTextColor(dc, kSettingsAccentText);
      RECT brand_title_rect{
          brand_badge_rect.right + 12,
          sidebar_brand_rect.top + 14,
          sidebar_brand_rect.right - 12,
          sidebar_brand_rect.top + 36,
      };
      DrawTextW(
          dc,
          UiText(use_chinese_ui_, L"Control Center", L"设置中心"),
          -1,
          &brand_title_rect,
          DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

      SelectObject(dc, settings_ui_font_ != nullptr ? settings_ui_font_ : GetStockObject(DEFAULT_GUI_FONT));
      SetTextColor(dc, kSettingsHintText);
      RECT brand_body_rect{
          brand_badge_rect.right + 12,
          sidebar_brand_rect.top + 40,
          sidebar_brand_rect.right - 12,
          sidebar_brand_rect.bottom - 10,
      };
      DrawTextW(
          dc,
          UiText(
              use_chinese_ui_,
              L"Modernize the current shell before moving deeper into WinUI 3.",
              L"先把当前壳层界面打磨到现代可用，再推进到 WinUI 3。"),
          -1,
          &brand_body_rect,
          DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

      const wchar_t* page_titles[] = {
          UiText(use_chinese_ui_, L"General", L"常规"),
          UiText(use_chinese_ui_, L"Storage", L"存储"),
          UiText(use_chinese_ui_, L"Appearance", L"外观"),
          UiText(use_chinese_ui_, L"Pins", L"置顶"),
          UiText(use_chinese_ui_, L"Ignore", L"忽略"),
          UiText(use_chinese_ui_, L"Advanced", L"高级"),
      };
      const wchar_t* page_descriptions[] = {
          UiText(
              use_chinese_ui_,
              L"Hotkeys, double-click open, and the core capture behavior live here.",
              L"这里管理快捷键、双击打开和最核心的捕获行为。"),
          UiText(
              use_chinese_ui_,
              L"Decide how much history to keep and which content types are worth saving.",
              L"决定保留多少历史记录，以及哪些内容类型需要保存。"),
          UiText(
              use_chinese_ui_,
              L"Shape the popup layout, search field visibility, and preview presence.",
              L"统一调整弹窗布局、搜索框显示和预览区呈现。"),
          UiText(
              use_chinese_ui_,
              L"Pinned items stay close to the popup, with shortcut-driven editing.",
              L"置顶项目继续围绕主弹窗操作，并通过快捷键完成编辑。"),
          UiText(
              use_chinese_ui_,
              L"Rules let you exclude apps, formats, and noisy text patterns from capture.",
              L"通过规则排除应用、格式和高噪音文本模式，减少无效捕获。"),
          UiText(
              use_chinese_ui_,
              L"Exit cleanup and safety switches sit here for power users.",
              L"面向高阶用户的退出清理和保护性开关集中在这里。"),
      };
      const int page_index = std::clamp(settings_current_page_index_, 0, 5);
      RECT sidebar_summary_rect{
          sidebar_rect.left + 12,
          sidebar_rect.bottom - 12 - kSettingsSidebarSummaryHeight,
          sidebar_rect.right - 12,
          sidebar_rect.bottom - 12,
      };
      HBRUSH sidebar_summary_brush = CreateSolidBrush(kSettingsSidebarSummaryBackground);
      HPEN sidebar_summary_pen = CreatePen(PS_SOLID, 1, kSettingsCardBorder);
      const HGDIOBJ old_summary_brush = SelectObject(dc, sidebar_summary_brush);
      const HGDIOBJ old_summary_pen = SelectObject(dc, sidebar_summary_pen);
      RoundRect(
          dc,
          sidebar_summary_rect.left,
          sidebar_summary_rect.top,
          sidebar_summary_rect.right,
          sidebar_summary_rect.bottom,
          18,
          18);
      SelectObject(dc, old_summary_pen);
      SelectObject(dc, old_summary_brush);
      DeleteObject(sidebar_summary_pen);
      DeleteObject(sidebar_summary_brush);

      HBRUSH summary_pill_brush = CreateSolidBrush(RGB(233, 240, 255));
      HPEN summary_pill_pen = CreatePen(PS_SOLID, 1, RGB(198, 214, 245));
      RECT summary_pill_rect{
          sidebar_summary_rect.left + 14,
          sidebar_summary_rect.top + 14,
          sidebar_summary_rect.left + 104,
          sidebar_summary_rect.top + 36,
      };
      const HGDIOBJ old_pill_brush = SelectObject(dc, summary_pill_brush);
      const HGDIOBJ old_pill_pen = SelectObject(dc, summary_pill_pen);
      RoundRect(
          dc,
          summary_pill_rect.left,
          summary_pill_rect.top,
          summary_pill_rect.right,
          summary_pill_rect.bottom,
          10,
          10);
      SelectObject(dc, old_pill_pen);
      SelectObject(dc, old_pill_brush);
      DeleteObject(summary_pill_pen);
      DeleteObject(summary_pill_brush);

      SelectObject(dc, settings_ui_semibold_font_ != nullptr ? settings_ui_semibold_font_ : GetStockObject(DEFAULT_GUI_FONT));
      SetTextColor(dc, kSettingsAccentText);
      RECT summary_caption_rect = summary_pill_rect;
      DrawTextW(
          dc,
          UiText(use_chinese_ui_, L"Now Editing", L"当前页面"),
          -1,
          &summary_caption_rect,
          DT_CENTER | DT_SINGLELINE | DT_VCENTER);

      RECT summary_title_rect{
          sidebar_summary_rect.left + 14,
          sidebar_summary_rect.top + 46,
          sidebar_summary_rect.right - 14,
          sidebar_summary_rect.top + 72,
      };
      DrawTextW(
          dc,
          page_titles[page_index],
          -1,
          &summary_title_rect,
          DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

      SelectObject(dc, settings_ui_font_ != nullptr ? settings_ui_font_ : GetStockObject(DEFAULT_GUI_FONT));
      SetTextColor(dc, kSettingsHintText);
      RECT summary_body_rect{
          sidebar_summary_rect.left + 14,
          sidebar_summary_rect.top + 72,
          sidebar_summary_rect.right - 14,
          sidebar_summary_rect.bottom - 14,
      };
      DrawTextW(dc, page_descriptions[page_index], -1, &summary_body_rect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

      SelectObject(dc, old_font);

      RECT divider{
          kSettingsOuterPadding,
          kSettingsClientHeight - kSettingsOuterPadding - 40,
          kSettingsClientWidth - kSettingsOuterPadding,
          kSettingsClientHeight - kSettingsOuterPadding - 39,
      };
      HBRUSH divider_brush = CreateSolidBrush(kSettingsCardBorder);
      FillRect(dc, &divider, divider_brush);
      DeleteObject(divider_brush);
      EndPaint(window, &paint);
      return 0;
    }
    case WM_DRAWITEM: {
      const auto* draw_item = reinterpret_cast<const DRAWITEMSTRUCT*>(lparam);
      if (draw_item == nullptr || draw_item->CtlType != ODT_BUTTON) {
        break;
      }

      const HWND button = draw_item->hwndItem;
      const UINT control_id = static_cast<UINT>(draw_item->CtlID);
      const auto is_checkbox_button = [&](HWND candidate) {
        return candidate == settings_capture_enabled_check_ || candidate == settings_auto_paste_check_ ||
               candidate == settings_plain_text_check_ || candidate == settings_start_on_login_check_ ||
               candidate == settings_double_click_open_check_ || candidate == settings_hotkey_ctrl_check_ ||
               candidate == settings_hotkey_alt_check_ || candidate == settings_hotkey_shift_check_ ||
               candidate == settings_hotkey_win_check_ || candidate == settings_show_search_check_ ||
               candidate == settings_show_preview_check_ || candidate == settings_remember_position_check_ ||
               candidate == settings_show_startup_guide_check_ || candidate == settings_ignore_all_check_ ||
               candidate == settings_only_listed_apps_check_ || candidate == settings_capture_text_check_ ||
               candidate == settings_capture_html_check_ || candidate == settings_capture_rtf_check_ ||
               candidate == settings_capture_images_check_ || candidate == settings_capture_files_check_ ||
               candidate == settings_clear_history_on_exit_check_ ||
               candidate == settings_clear_clipboard_on_exit_check_;
      };
      const bool is_nav_button =
          control_id >= kSettingsNavGeneralButtonId && control_id <= kSettingsNavAdvancedButtonId;
      const bool is_checkbox = is_checkbox_button(button);
      const bool is_active_nav =
          is_nav_button && (control_id - kSettingsNavGeneralButtonId) == static_cast<UINT>(settings_current_page_index_);
      const bool is_primary_action = control_id == kSettingsSaveButtonId;
      const bool is_secondary_action = control_id == kSettingsApplyButtonId;
      const bool is_tertiary_action = control_id == kSettingsCloseButtonId;
      const bool is_pressed = (draw_item->itemState & ODS_SELECTED) != 0;
      const bool is_focused = (draw_item->itemState & ODS_FOCUS) != 0;
      const bool is_checked = SendMessageW(button, BM_GETCHECK, 0, 0) == BST_CHECKED;

      wchar_t label[128]{};
      GetWindowTextW(button, label, ARRAYSIZE(label));

      RECT rect = draw_item->rcItem;
      HDC dc = draw_item->hDC;
      SetBkMode(dc, TRANSPARENT);

      if (is_checkbox) {
        FillRect(
            dc,
            &rect,
            settings_card_brush_ != nullptr
                ? settings_card_brush_
                : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

        RECT box_rect{
            rect.left + 2,
            rect.top + 4,
            rect.left + 22,
            rect.top + 24,
        };
        const COLORREF checkbox_fill = is_checked ? kSettingsButtonPrimary : RGB(255, 255, 255);
        const COLORREF checkbox_border =
            is_checked ? kSettingsButtonPrimaryBorder : (is_pressed ? RGB(118, 132, 152) : kSettingsCheckboxBorder);
        HBRUSH box_brush = CreateSolidBrush(checkbox_fill);
        HPEN box_pen = CreatePen(PS_SOLID, 1, checkbox_border);
        const HGDIOBJ old_brush = SelectObject(dc, box_brush);
        const HGDIOBJ old_pen = SelectObject(dc, box_pen);
        RoundRect(dc, box_rect.left, box_rect.top, box_rect.right, box_rect.bottom, 6, 6);
        SelectObject(dc, old_pen);
        SelectObject(dc, old_brush);
        DeleteObject(box_pen);
        DeleteObject(box_brush);

        if (is_checked) {
          HPEN check_pen = CreatePen(PS_SOLID, 2, kSettingsCheckboxCheck);
          const HGDIOBJ old_check_pen = SelectObject(dc, check_pen);
          MoveToEx(dc, box_rect.left + 5, box_rect.top + 11, nullptr);
          LineTo(dc, box_rect.left + 9, box_rect.top + 15);
          LineTo(dc, box_rect.left + 16, box_rect.top + 7);
          SelectObject(dc, old_check_pen);
          DeleteObject(check_pen);
        }

        SetTextColor(dc, kSettingsPrimaryText);
        RECT text_rect{
            box_rect.right + 10,
            rect.top,
            rect.right,
            rect.bottom,
        };
        DrawTextW(dc, label, -1, &text_rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

        if (is_focused) {
          RECT focus_rect = rect;
          focus_rect.left = text_rect.left - 2;
          InflateRect(&focus_rect, -2, -5);
          DrawFocusRect(dc, &focus_rect);
        }
        return TRUE;
      }

      COLORREF fill_color = is_nav_button ? kSettingsSidebarBackground : kSettingsButtonSecondaryBackground;
      COLORREF border_color = is_nav_button ? kSettingsSidebarBackground : kSettingsCardBorder;
      COLORREF text_color = is_nav_button ? kSettingsNavText : kSettingsPrimaryText;

      if (is_active_nav) {
        fill_color = kSettingsNavActiveBackground;
        border_color = kSettingsCardBorder;
        text_color = kSettingsAccentText;
      } else if (is_pressed) {
        fill_color = is_nav_button ? RGB(229, 236, 248) : RGB(236, 241, 248);
      } else if ((draw_item->itemState & ODS_HOTLIGHT) != 0) {
        fill_color = is_nav_button ? kSettingsNavHoverBackground : RGB(247, 249, 252);
      }

      if (is_primary_action) {
        fill_color = is_pressed ? RGB(21, 86, 201) : kSettingsButtonPrimary;
        border_color = kSettingsButtonPrimaryBorder;
        text_color = RGB(255, 255, 255);
      } else if (is_secondary_action) {
        fill_color = is_pressed ? RGB(226, 236, 253) : RGB(239, 245, 255);
        border_color = RGB(188, 206, 243);
        text_color = kSettingsAccentText;
      } else if (is_tertiary_action) {
        fill_color = is_pressed ? RGB(244, 246, 250) : RGB(255, 255, 255);
        border_color = RGB(214, 221, 232);
      }

      HBRUSH fill_brush = CreateSolidBrush(fill_color);
      HPEN border_pen = CreatePen(PS_SOLID, 1, border_color);
      const HGDIOBJ old_brush = SelectObject(dc, fill_brush);
      const HGDIOBJ old_pen = SelectObject(dc, border_pen);
      RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, 14, 14);
      SelectObject(dc, old_pen);
      SelectObject(dc, old_brush);
      DeleteObject(border_pen);
      DeleteObject(fill_brush);

      if (is_active_nav) {
        RECT accent_rect{
            rect.left + 8,
            rect.top + 10,
            rect.left + 12,
            rect.bottom - 10,
        };
        HBRUSH accent_brush = CreateSolidBrush(kSettingsAccentFill);
        FillRect(dc, &accent_rect, accent_brush);
        DeleteObject(accent_brush);
      }

      SetTextColor(dc, text_color);
      RECT text_rect = rect;
      if (is_nav_button) {
        text_rect.left += 22;
      }
      DrawTextW(
          dc,
          label,
          -1,
          &text_rect,
          DT_VCENTER | DT_SINGLELINE | (is_nav_button ? DT_LEFT : DT_CENTER) | DT_END_ELLIPSIS);

      if (is_focused) {
        RECT focus_rect = rect;
        InflateRect(&focus_rect, -4, -4);
        DrawFocusRect(dc, &focus_rect);
      }
      return TRUE;
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
      if (control == settings_header_title_ || control == settings_header_subtitle_) {
        return reinterpret_cast<INT_PTR>(
            settings_window_background_brush_ != nullptr
                ? settings_window_background_brush_
                : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
      }
      return reinterpret_cast<INT_PTR>(
          settings_window_background_brush_ != nullptr
              ? settings_window_background_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
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
        case kSettingsNavGeneralButtonId:
          ShowSettingsPage(0);
          return 0;
        case kSettingsNavStorageButtonId:
          ShowSettingsPage(1);
          return 0;
        case kSettingsNavAppearanceButtonId:
          ShowSettingsPage(2);
          return 0;
        case kSettingsNavPinsButtonId:
          ShowSettingsPage(3);
          return 0;
        case kSettingsNavIgnoreButtonId:
          ShowSettingsPage(4);
          return 0;
        case kSettingsNavAdvancedButtonId:
          ShowSettingsPage(5);
          return 0;
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
      settings_nav_general_button_ = nullptr;
      settings_nav_storage_button_ = nullptr;
      settings_nav_appearance_button_ = nullptr;
      settings_nav_pins_button_ = nullptr;
      settings_nav_ignore_button_ = nullptr;
      settings_nav_advanced_button_ = nullptr;
      settings_save_button_ = nullptr;
      settings_apply_button_ = nullptr;
      settings_close_button_ = nullptr;
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
      settings_current_page_index_ = 0;
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
    case WM_DRAWITEM:
      if (settings_window_ != nullptr) {
        return SendMessageW(settings_window_, message, wparam, lparam);
      }
      return FALSE;
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

          RECT accent_rect{
              section.rect.left + 18,
              section.rect.top + 12,
              section.rect.left + 62,
              section.rect.top + 16,
          };
          HBRUSH accent_brush = CreateSolidBrush(kSettingsAccentFill);
          FillRect(dc, &accent_rect, accent_brush);
          DeleteObject(accent_brush);
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
    case WM_COMMAND:
      if (settings_window_ != nullptr) {
        return SendMessageW(settings_window_, message, wparam, lparam);
      }
      return 0;
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
