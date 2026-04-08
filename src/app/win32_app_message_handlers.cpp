#ifdef _WIN32

#include "app/win32_app_internal.h"

namespace maccy {

namespace {

constexpr COLORREF kPopupWindowBackground = RGB(246, 248, 252);
constexpr COLORREF kPopupSurfaceBackground = RGB(255, 255, 255);
constexpr COLORREF kPopupSurfaceBorder = RGB(220, 226, 236);
constexpr COLORREF kPopupInputBackground = RGB(255, 255, 255);
constexpr COLORREF kPopupPrimaryText = RGB(26, 31, 43);
constexpr int kPopupOuterPadding = 14;
constexpr int kPopupCardInset = 8;
constexpr int kPopupCardRadius = 14;

}  // namespace

LRESULT Win32App::HandleControllerMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == taskbar_created_message_) {
    SetupTrayIcon();
    return 0;
  }

  switch (message) {
    case kActivateExistingInstanceMessage:
      ShowPopup();
      return 0;
    case kDoubleClickModifierTriggeredMessage:
      HandleDoubleClickModifierTriggered();
      return 0;
    case WM_HOTKEY:
      if (static_cast<int>(wparam) == kToggleHotKeyId) {
        TogglePopup();
        return 0;
      }
      break;
    case WM_CLIPBOARDUPDATE:
      HandleClipboardUpdate();
      return 0;
    case kTrayMessage:
      switch (TrayNotifyCode(lparam)) {
        case WM_LBUTTONUP:
        case NIN_KEYSELECT:
          TogglePopup();
          return 0;
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU:
          {
            const POINT anchor = TrayAnchorPoint(wparam);
            ShowTrayMenu(&anchor);
          }
          return 0;
        default:
          break;
      }
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      break;
  }

  return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT Win32App::HandlePopupMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_CREATE: {
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

      const HFONT default_font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
      if (popup_ui_font_ == nullptr) {
        popup_ui_font_ = create_font(10, FW_NORMAL);
      }
      if (popup_ui_emphasis_font_ == nullptr) {
        popup_ui_emphasis_font_ = create_font(10, FW_SEMIBOLD);
      }
      if (popup_window_background_brush_ == nullptr) {
        popup_window_background_brush_ = CreateSolidBrush(kPopupWindowBackground);
      }
      if (popup_surface_brush_ == nullptr) {
        popup_surface_brush_ = CreateSolidBrush(kPopupSurfaceBackground);
      }
      if (popup_input_brush_ == nullptr) {
        popup_input_brush_ = CreateSolidBrush(kPopupInputBackground);
      }
      if (popup_surface_border_pen_ == nullptr) {
        popup_surface_border_pen_ = CreatePen(PS_SOLID, 1, kPopupSurfaceBorder);
      }

      const HFONT body_font = popup_ui_font_ != nullptr ? popup_ui_font_ : default_font;

      search_edit_ = CreateWindowExW(
          0,
          L"EDIT",
          nullptr,
          WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
          0,
          0,
          0,
          0,
          window,
          nullptr,
          instance_,
          nullptr);

      list_box_ = CreateWindowExW(
          0,
          L"LISTBOX",
          nullptr,
          WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT |
              LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
          0,
          0,
          0,
          0,
          window,
          nullptr,
          instance_,
          nullptr);

      preview_edit_ = CreateWindowExW(
          0,
          L"EDIT",
          nullptr,
          WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
          0,
          0,
          0,
          0,
          window,
          nullptr,
          instance_,
          nullptr);

      if (search_edit_ != nullptr) {
        SendMessageW(search_edit_, WM_SETFONT, reinterpret_cast<WPARAM>(body_font), TRUE);
        SendMessageW(search_edit_, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(12, 12));
        SendMessageW(
            search_edit_,
            EM_SETCUEBANNER,
            0,
            reinterpret_cast<LPARAM>(UiText(
                use_chinese_ui_,
                L"Search clipboard history, source apps, or pinned items",
                L"搜索剪贴板历史、来源应用或置顶项目")));
        SetWindowLongPtrW(search_edit_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        original_search_edit_proc_ = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(search_edit_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StaticSearchEditProc)));
      }

      if (list_box_ != nullptr) {
        SendMessageW(list_box_, WM_SETFONT, reinterpret_cast<WPARAM>(body_font), TRUE);
        SetWindowLongPtrW(list_box_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        original_list_box_proc_ = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(list_box_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StaticListBoxWindowProc)));
      }

      if (preview_edit_ != nullptr) {
        SendMessageW(preview_edit_, WM_SETFONT, reinterpret_cast<WPARAM>(body_font), TRUE);
        SendMessageW(preview_edit_, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(12, 12));
      }
      return 0;
    }
    case WM_SIZE: {
      const int client_width = LOWORD(lparam);
      const int client_height = HIWORD(lparam);
      const int outer_padding = kPopupOuterPadding;
      const int card_inset = kPopupCardInset;
      const bool show_search = settings_.popup.show_search && search_edit_ != nullptr;
      const int available_width = std::max(0, client_width - outer_padding * 2);
      const int search_card_height = show_search ? (kSearchEditHeight + 16) : 0;
      const int search_card_bottom = outer_padding + search_card_height;
      const int content_top = show_search ? (search_card_bottom + kPopupPadding) : outer_padding;
      const int content_height = std::max(0, client_height - content_top - outer_padding);

      popup_search_card_rect_ = RECT{0, 0, 0, 0};
      popup_list_card_rect_ = RECT{0, 0, 0, 0};
      popup_preview_card_rect_ = RECT{0, 0, 0, 0};

      if (show_search) {
        popup_search_card_rect_ = RECT{
            outer_padding,
            outer_padding,
            outer_padding + available_width,
            search_card_bottom,
        };
      }

      if (search_edit_ != nullptr) {
        ShowWindow(search_edit_, show_search ? SW_SHOW : SW_HIDE);
        if (show_search) {
          MoveWindow(
              search_edit_,
              popup_search_card_rect_.left + card_inset,
              popup_search_card_rect_.top + 8,
              std::max(120, available_width - card_inset * 2),
              kSearchEditHeight,
              TRUE);
        }
      }

      if (list_box_ != nullptr) {
        if (settings_.popup.show_preview && preview_edit_ != nullptr) {
          const int minimum_list_width = 200;
          const int preview_width = std::clamp(
              settings_.popup.preview_width,
              kPreviewMinWidth,
              std::max(kPreviewMinWidth, available_width - minimum_list_width - kPopupPadding));
          const int list_width = std::max(minimum_list_width, available_width - preview_width - kPopupPadding);
          popup_list_card_rect_ = RECT{
              outer_padding,
              content_top,
              outer_padding + list_width,
              content_top + content_height,
          };
          popup_preview_card_rect_ = RECT{
              popup_list_card_rect_.right + kPopupPadding,
              content_top,
              outer_padding + available_width,
              content_top + content_height,
          };
          MoveWindow(
              list_box_,
              popup_list_card_rect_.left + card_inset,
              popup_list_card_rect_.top + card_inset,
              std::max(180, list_width - card_inset * 2),
              std::max(120, content_height - card_inset * 2),
              TRUE);
          MoveWindow(
              preview_edit_,
              popup_preview_card_rect_.left + card_inset,
              popup_preview_card_rect_.top + card_inset,
              std::max(120, popup_preview_card_rect_.right - popup_preview_card_rect_.left - card_inset * 2),
              std::max(120, content_height - card_inset * 2),
              TRUE);
          ShowWindow(preview_edit_, SW_SHOW);
        } else {
          popup_list_card_rect_ = RECT{
              outer_padding,
              content_top,
              outer_padding + available_width,
              content_top + content_height,
          };
          MoveWindow(
              list_box_,
              popup_list_card_rect_.left + card_inset,
              popup_list_card_rect_.top + card_inset,
              std::max(220, available_width - card_inset * 2),
              std::max(120, content_height - card_inset * 2),
              TRUE);
          if (preview_edit_ != nullptr) {
            ShowWindow(preview_edit_, SW_HIDE);
          }
        }
      }
      InvalidateRect(window, nullptr, TRUE);
      if (list_box_ != nullptr) {
        InvalidateRect(list_box_, nullptr, TRUE);
      }
      return 0;
    }
    case WM_DRAWITEM: {
      const auto* draw_item = reinterpret_cast<const DRAWITEMSTRUCT*>(lparam);
      if (draw_item != nullptr && draw_item->hwndItem == list_box_) {
        DrawPopupListItem(*draw_item);
        return TRUE;
      }
      break;
    }
    case WM_MEASUREITEM: {
      auto* measure_item = reinterpret_cast<MEASUREITEMSTRUCT*>(lparam);
      if (measure_item != nullptr && measure_item->CtlType == ODT_LISTBOX) {
        measure_item->itemHeight = 56;
        return TRUE;
      }
      break;
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
          popup_window_background_brush_ != nullptr
              ? popup_window_background_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

      const auto draw_surface = [&](const RECT& rect) {
        if (rect.right <= rect.left || rect.bottom <= rect.top) {
          return;
        }
        const HGDIOBJ old_brush = SelectObject(
            dc,
            popup_surface_brush_ != nullptr
                ? popup_surface_brush_
                : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
        const HGDIOBJ old_pen = SelectObject(
            dc,
            popup_surface_border_pen_ != nullptr
                ? popup_surface_border_pen_
                : static_cast<HPEN>(GetStockObject(BLACK_PEN)));
        RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, kPopupCardRadius, kPopupCardRadius);
        SelectObject(dc, old_pen);
        SelectObject(dc, old_brush);
      };

      draw_surface(popup_search_card_rect_);
      draw_surface(popup_list_card_rect_);
      draw_surface(popup_preview_card_rect_);

      EndPaint(window, &paint);
      return 0;
    }
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
      HDC dc = reinterpret_cast<HDC>(wparam);
      SetTextColor(dc, kPopupPrimaryText);
      SetBkColor(dc, kPopupInputBackground);
      return reinterpret_cast<INT_PTR>(
          popup_input_brush_ != nullptr
              ? popup_input_brush_
              : static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    }
    case WM_SETFOCUS:
      if (settings_.popup.show_search && search_edit_ != nullptr) {
        SetFocus(search_edit_);
        return 0;
      }
      if (list_box_ != nullptr) {
        SetFocus(list_box_);
        return 0;
      }
      break;
    case WM_ACTIVATE:
      if (LOWORD(wparam) == WA_INACTIVE) {
        HidePopup();
      }
      return 0;
    case WM_COMMAND:
      if (reinterpret_cast<HWND>(lparam) == search_edit_ && HIWORD(wparam) == EN_CHANGE) {
        UpdateSearchQueryFromEdit();
        return 0;
      }
      if (reinterpret_cast<HWND>(lparam) == list_box_ && HIWORD(wparam) == LBN_DBLCLK) {
        ActivateSelectedItem();
        return 0;
      }
      if (reinterpret_cast<HWND>(lparam) == list_box_ && HIWORD(wparam) == LBN_SELCHANGE) {
        UpdatePreview();
        return 0;
      }
      break;
    case WM_CLOSE:
      HidePopup();
      return 0;
    case WM_DESTROY:
      if (popup_ui_font_ != nullptr) {
        DeleteObject(popup_ui_font_);
        popup_ui_font_ = nullptr;
      }
      if (popup_ui_emphasis_font_ != nullptr) {
        DeleteObject(popup_ui_emphasis_font_);
        popup_ui_emphasis_font_ = nullptr;
      }
      if (popup_window_background_brush_ != nullptr) {
        DeleteObject(popup_window_background_brush_);
        popup_window_background_brush_ = nullptr;
      }
      if (popup_surface_brush_ != nullptr) {
        DeleteObject(popup_surface_brush_);
        popup_surface_brush_ = nullptr;
      }
      if (popup_input_brush_ != nullptr) {
        DeleteObject(popup_input_brush_);
        popup_input_brush_ = nullptr;
      }
      if (popup_surface_border_pen_ != nullptr) {
        DeleteObject(popup_surface_border_pen_);
        popup_surface_border_pen_ = nullptr;
      }
      popup_search_card_rect_ = RECT{0, 0, 0, 0};
      popup_list_card_rect_ = RECT{0, 0, 0, 0};
      popup_preview_card_rect_ = RECT{0, 0, 0, 0};
      return 0;
    default:
      break;
  }

  return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT Win32App::HandleListBoxMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_KEYDOWN) {
    if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
      switch (wparam) {
        case 'P':
          ToggleSelectedPin();
          return 0;
        case 'R':
          OpenPinEditor(true);
          return 0;
        case 'E':
          OpenPinEditor(false);
          return 0;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          if (list_box_ != nullptr && !visible_item_ids_.empty()) {
            SelectVisibleIndex(static_cast<int>(wparam - '1'));
          }
          return 0;
        default:
          break;
      }
    }

    switch (wparam) {
      case VK_RETURN:
        ActivateSelectedItem();
        return 0;
      case VK_DELETE:
        DeleteSelectedItem();
        return 0;
      case VK_ESCAPE:
        HidePopup();
        return 0;
      case VK_UP:
        if (settings_.popup.show_search && SendMessageW(window, LB_GETCURSEL, 0, 0) == 0 && search_edit_ != nullptr) {
          SetFocus(search_edit_);
          return 0;
        }
        break;
      default:
        break;
    }
  }

  return CallWindowProcW(original_list_box_proc_, window, message, wparam, lparam);
}

LRESULT Win32App::HandleSearchEditMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_KEYDOWN) {
    if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
      switch (wparam) {
        case 'P':
          ToggleSelectedPin();
          return 0;
        case 'R':
          OpenPinEditor(true);
          return 0;
        case 'E':
          OpenPinEditor(false);
          return 0;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          if (list_box_ != nullptr && !visible_item_ids_.empty()) {
            SetFocus(list_box_);
            SelectVisibleIndex(static_cast<int>(wparam - '1'));
          }
          return 0;
        default:
          break;
      }
    }

    switch (wparam) {
      case VK_DOWN:
        if (list_box_ != nullptr && !visible_item_ids_.empty()) {
          SetFocus(list_box_);
          SelectVisibleIndex(0);
          return 0;
        }
        break;
      case VK_RETURN:
        ActivateSelectedItem();
        return 0;
      case VK_DELETE:
        DeleteSelectedItem();
        return 0;
      case VK_ESCAPE:
        HidePopup();
        return 0;
      default:
        break;
    }
  }

  return CallWindowProcW(original_search_edit_proc_, window, message, wparam, lparam);
}

LRESULT Win32App::HandlePinEditorMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_CREATE: {
      const bool zh = use_chinese_ui_;
      const HFONT default_font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

      pin_editor_edit_ = CreateWindowExW(
          WS_EX_CLIENTEDGE,
          L"EDIT",
          nullptr,
          WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL,
          0,
          0,
          0,
          0,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPinEditorEditControlId)),
          instance_,
          nullptr);

      HWND save_button = CreateWindowExW(
          0,
          L"BUTTON",
          UiText(zh, L"Save", L"保存"),
          WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
          0,
          0,
          0,
          0,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPinEditorSaveButtonId)),
          instance_,
          nullptr);

      HWND cancel_button = CreateWindowExW(
          0,
          L"BUTTON",
          UiText(zh, L"Cancel", L"取消"),
          WS_CHILD | WS_VISIBLE | WS_TABSTOP,
          0,
          0,
          0,
          0,
          window,
          reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPinEditorCancelButtonId)),
          instance_,
          nullptr);

      if (pin_editor_edit_ != nullptr) {
        SendMessageW(pin_editor_edit_, WM_SETFONT, reinterpret_cast<WPARAM>(default_font), TRUE);
      }
      if (save_button != nullptr) {
        SendMessageW(save_button, WM_SETFONT, reinterpret_cast<WPARAM>(default_font), TRUE);
      }
      if (cancel_button != nullptr) {
        SendMessageW(cancel_button, WM_SETFONT, reinterpret_cast<WPARAM>(default_font), TRUE);
      }
      LayoutPinEditorControls();
      return 0;
    }
    case WM_SIZE:
      LayoutPinEditorControls();
      return 0;
    case WM_COMMAND:
      switch (LOWORD(wparam)) {
        case kPinEditorSaveButtonId:
          CommitPinEditor();
          return 0;
        case kPinEditorCancelButtonId:
          ClosePinEditor();
          return 0;
        default:
          break;
      }
      break;
    case WM_CLOSE:
      ClosePinEditor();
      return 0;
    case WM_DESTROY:
      pin_editor_window_ = nullptr;
      pin_editor_edit_ = nullptr;
      return 0;
    default:
      break;
  }

  return DefWindowProcW(window, message, wparam, lparam);
}


}  // namespace maccy

#endif  // _WIN32
