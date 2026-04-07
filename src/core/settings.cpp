/**
 * @file settings.cpp
 * @brief 应用程序设置实现
 */

#include "core/settings.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace maccy {

namespace {

/** 设置文件魔数头部 */
constexpr char kMagicHeader[] = "MACCY_SETTINGS_V1";

/**
 * @brief 转义文本
 * @details 对特殊字符进行转义：\\、\\n、\\r、\\t
 * @param value 要转义的文本
 * @return std::string 转义后的文本
 */
std::string EscapeText(std::string_view value) {
  std::string escaped;
  escaped.reserve(value.size());

  for (char ch : value) {
    switch (ch) {
      case '\\':
        escaped += "\\\\";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped.push_back(ch);
        break;
    }
  }

  return escaped;
}

/**
 * @brief 反转义文本
 * @param value 要反转义的文本
 * @return std::string 反转义后的文本
 */
std::string UnescapeText(std::string_view value) {
  std::string unescaped;
  unescaped.reserve(value.size());

  bool escaping = false;
  for (char ch : value) {
    if (!escaping) {
      if (ch == '\\') {
        escaping = true;
      } else {
        unescaped.push_back(ch);
      }
      continue;
    }

    switch (ch) {
      case 'n':
        unescaped.push_back('\n');
        break;
      case 'r':
        unescaped.push_back('\r');
        break;
      case 't':
        unescaped.push_back('\t');
        break;
      case '\\':
        unescaped.push_back('\\');
        break;
      default:
        unescaped.push_back(ch);
        break;
    }

    escaping = false;
  }

  if (escaping) {
    unescaped.push_back('\\');
  }

  return unescaped;
}

/**
 * @brief 分割 Tab 分隔的字段
 * @param line 要分割的行
 * @return std::vector<std::string> 分割后的字段列表
 */
std::vector<std::string> SplitTabFields(const std::string& line) {
  std::vector<std::string> fields;
  std::string current;

  for (char ch : line) {
    if (ch == '\t') {
      fields.push_back(current);
      current.clear();
    } else {
      current.push_back(ch);
    }
  }

  fields.push_back(current);
  return fields;
}

/**
 * @brief 解析布尔值
 * @param value 字符串值
 * @return bool 解析后的布尔值
 */
bool ParseBool(std::string_view value) {
  return value == "1" || value == "true";
}

/**
 * @brief 解析大小值
 * @param value 字符串值
 * @return std::size_t 解析后的大小值
 */
std::size_t ParseSize(std::string_view value) {
  try {
    return static_cast<std::size_t>(std::stoull(std::string(value)));
  } catch (...) {
    return 0;
  }
}

/**
 * @brief 解析整数值
 * @param value 字符串值
 * @return int 解析后的整数值
 */
int ParseInt(std::string_view value) {
  try {
    return std::stoi(std::string(value));
  } catch (...) {
    return 0;
  }
}

/**
 * @brief 解析无符号 32 位整数
 * @param value 字符串值
 * @return std::uint32_t 解析后的整数值
 */
std::uint32_t ParseUInt32(std::string_view value) {
  try {
    return static_cast<std::uint32_t>(std::stoul(std::string(value)));
  } catch (...) {
    return 0;
  }
}

/**
 * @brief 将固定位置转换为字符串
 * @param position 固定位置
 * @return std::string_view 位置对应的字符串
 */
std::string_view ToString(PinPosition position) {
  switch (position) {
    case PinPosition::kTop:
      return "top";
    case PinPosition::kBottom:
      return "bottom";
  }

  return "top";
}

/**
 * @brief 将历史排序方式转换为字符串
 * @param order 历史排序方式
 * @return std::string_view 排序方式对应的字符串
 */
std::string_view ToString(HistorySortOrder order) {
  switch (order) {
    case HistorySortOrder::kLastCopied:
      return "last_copied";
    case HistorySortOrder::kFirstCopied:
      return "first_copied";
    case HistorySortOrder::kCopyCount:
      return "copy_count";
  }

  return "last_copied";
}

/**
 * @brief 解析固定位置
 * @param value 字符串值
 * @return PinPosition 对应的固定位置
 */
PinPosition ParsePinPosition(std::string_view value) {
  return value == "bottom" ? PinPosition::kBottom : PinPosition::kTop;
}

/**
 * @brief 解析历史排序方式
 * @param value 字符串值
 * @return HistorySortOrder 对应的排序方式
 */
HistorySortOrder ParseHistorySortOrder(std::string_view value) {
  if (value == "first_copied") {
    return HistorySortOrder::kFirstCopied;
  }
  if (value == "copy_count") {
    return HistorySortOrder::kCopyCount;
  }
  return HistorySortOrder::kLastCopied;
}

/**
 * @brief 解析搜索模式
 * @param value 字符串值
 * @return SearchMode 对应的搜索模式
 */
SearchMode ParseSearchMode(std::string_view value) {
  if (value == "exact") {
    return SearchMode::kExact;
  }
  if (value == "fuzzy") {
    return SearchMode::kFuzzy;
  }
  if (value == "regexp") {
    return SearchMode::kRegexp;
  }
  return SearchMode::kMixed;
}

/**
 * @brief 限制弹窗设置值
 * @param popup 弹窗设置
 */
void ClampPopupSettings(PopupSettings& popup) {
  popup.width = std::max(420, popup.width);
  popup.height = std::max(280, popup.height);
  popup.preview_width = std::clamp(popup.preview_width, 180, 480);
}

}  // namespace

bool SaveSettingsFile(
    const std::filesystem::path& path,
    const AppSettings& settings) {
  std::error_code error;
  if (!path.parent_path().empty()) {
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
      return false;
    }
  }

  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output.is_open()) {
    return false;
  }

  const auto write_line = [&output](std::string_view key, std::string_view value) {
    output << key << '\t' << EscapeText(value) << '\n';
  };

  output << kMagicHeader << '\n';
  write_line("max_history_items", std::to_string(settings.max_history_items));
  write_line("pin_position", std::string(ToString(settings.pin_position)));
  write_line("sort_order", std::string(ToString(settings.sort_order)));
  write_line("search_mode", std::string(ToString(settings.search_mode)));
  write_line("popup_hotkey_modifiers", std::to_string(settings.popup_hotkey_modifiers));
  write_line("popup_hotkey_virtual_key", std::to_string(settings.popup_hotkey_virtual_key));
  write_line("double_click_popup_enabled", settings.double_click_popup_enabled ? "1" : "0");
  write_line("double_click_modifier_key", std::string(ToString(settings.double_click_modifier_key)));
  write_line("show_startup_guide", settings.show_startup_guide ? "1" : "0");
  write_line("capture_enabled", settings.capture_enabled ? "1" : "0");
  write_line("auto_paste", settings.auto_paste ? "1" : "0");
  write_line("paste_plain_text", settings.paste_plain_text ? "1" : "0");
  write_line("start_on_login", settings.start_on_login ? "1" : "0");
  write_line("clear_history_on_exit", settings.clear_history_on_exit ? "1" : "0");
  write_line("clear_system_clipboard_on_exit", settings.clear_system_clipboard_on_exit ? "1" : "0");
  write_line("show_search", settings.popup.show_search ? "1" : "0");
  write_line("show_preview", settings.popup.show_preview ? "1" : "0");
  write_line("remember_last_position", settings.popup.remember_last_position ? "1" : "0");
  write_line("has_last_position", settings.popup.has_last_position ? "1" : "0");
  write_line("popup_x", std::to_string(settings.popup.x));
  write_line("popup_y", std::to_string(settings.popup.y));
  write_line("popup_width", std::to_string(settings.popup.width));
  write_line("popup_height", std::to_string(settings.popup.height));
  write_line("preview_width", std::to_string(settings.popup.preview_width));
  write_line("ignore_all", settings.ignore.ignore_all ? "1" : "0");
  write_line("only_listed_applications", settings.ignore.only_listed_applications ? "1" : "0");
  write_line("capture_text", settings.ignore.capture_text ? "1" : "0");
  write_line("capture_html", settings.ignore.capture_html ? "1" : "0");
  write_line("capture_rtf", settings.ignore.capture_rtf ? "1" : "0");
  write_line("capture_images", settings.ignore.capture_images ? "1" : "0");
  write_line("capture_files", settings.ignore.capture_files ? "1" : "0");

  for (const auto& value : settings.ignore.ignored_applications) {
    write_line("ignored_app", value);
  }
  for (const auto& value : settings.ignore.allowed_applications) {
    write_line("allowed_app", value);
  }
  for (const auto& value : settings.ignore.ignored_patterns) {
    write_line("ignored_pattern", value);
  }
  for (const auto& value : settings.ignore.ignored_formats) {
    write_line("ignored_format", value);
  }

  return output.good();
}

AppSettings LoadSettingsFile(const std::filesystem::path& path) {
  AppSettings settings;
  bool loaded_only_listed_applications = false;

  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    return settings;
  }

  std::string header;
  std::getline(input, header);
  if (header != kMagicHeader) {
    return settings;
  }

  std::string line;
  while (std::getline(input, line)) {
    if (line.empty()) {
      continue;
    }

    const auto fields = SplitTabFields(line);
    if (fields.size() < 2) {
      continue;
    }

    const std::string value = UnescapeText(fields[1]);

    if (fields[0] == "max_history_items") {
      settings.max_history_items = std::max<std::size_t>(1, ParseSize(value));
    } else if (fields[0] == "pin_position") {
      settings.pin_position = ParsePinPosition(value);
    } else if (fields[0] == "sort_order") {
      settings.sort_order = ParseHistorySortOrder(value);
    } else if (fields[0] == "search_mode") {
      settings.search_mode = ParseSearchMode(value);
    } else if (fields[0] == "popup_hotkey_modifiers") {
      settings.popup_hotkey_modifiers = ParseUInt32(value);
    } else if (fields[0] == "popup_hotkey_virtual_key") {
      settings.popup_hotkey_virtual_key = ParseUInt32(value);
    } else if (fields[0] == "double_click_popup_enabled") {
      settings.double_click_popup_enabled = ParseBool(value);
    } else if (fields[0] == "double_click_modifier_key") {
      settings.double_click_modifier_key = ParseDoubleClickModifierKey(value);
    } else if (fields[0] == "show_startup_guide") {
      settings.show_startup_guide = ParseBool(value);
    } else if (fields[0] == "capture_enabled") {
      settings.capture_enabled = ParseBool(value);
    } else if (fields[0] == "auto_paste") {
      settings.auto_paste = ParseBool(value);
    } else if (fields[0] == "paste_plain_text") {
      settings.paste_plain_text = ParseBool(value);
    } else if (fields[0] == "start_on_login") {
      settings.start_on_login = ParseBool(value);
    } else if (fields[0] == "clear_history_on_exit") {
      settings.clear_history_on_exit = ParseBool(value);
    } else if (fields[0] == "clear_system_clipboard_on_exit") {
      settings.clear_system_clipboard_on_exit = ParseBool(value);
    } else if (fields[0] == "show_search") {
      settings.popup.show_search = ParseBool(value);
    } else if (fields[0] == "show_preview") {
      settings.popup.show_preview = ParseBool(value);
    } else if (fields[0] == "remember_last_position") {
      settings.popup.remember_last_position = ParseBool(value);
    } else if (fields[0] == "has_last_position") {
      settings.popup.has_last_position = ParseBool(value);
    } else if (fields[0] == "popup_x") {
      settings.popup.x = ParseInt(value);
    } else if (fields[0] == "popup_y") {
      settings.popup.y = ParseInt(value);
    } else if (fields[0] == "popup_width") {
      settings.popup.width = ParseInt(value);
    } else if (fields[0] == "popup_height") {
      settings.popup.height = ParseInt(value);
    } else if (fields[0] == "preview_width") {
      settings.popup.preview_width = ParseInt(value);
    } else if (fields[0] == "ignore_all") {
      settings.ignore.ignore_all = ParseBool(value);
    } else if (fields[0] == "only_listed_applications") {
      settings.ignore.only_listed_applications = ParseBool(value);
      loaded_only_listed_applications = true;
    } else if (fields[0] == "capture_text") {
      settings.ignore.capture_text = ParseBool(value);
    } else if (fields[0] == "capture_html") {
      settings.ignore.capture_html = ParseBool(value);
    } else if (fields[0] == "capture_rtf") {
      settings.ignore.capture_rtf = ParseBool(value);
    } else if (fields[0] == "capture_images") {
      settings.ignore.capture_images = ParseBool(value);
    } else if (fields[0] == "capture_files") {
      settings.ignore.capture_files = ParseBool(value);
    } else if (fields[0] == "ignored_app") {
      settings.ignore.ignored_applications.push_back(value);
    } else if (fields[0] == "allowed_app") {
      settings.ignore.allowed_applications.push_back(value);
    } else if (fields[0] == "ignored_pattern") {
      settings.ignore.ignored_patterns.push_back(value);
    } else if (fields[0] == "ignored_format") {
      settings.ignore.ignored_formats.push_back(value);
    }
  }

  if (!loaded_only_listed_applications && !settings.ignore.allowed_applications.empty()) {
    settings.ignore.only_listed_applications = true;
  }

  ClampPopupSettings(settings.popup);
  return settings;
}

}  // namespace maccy
