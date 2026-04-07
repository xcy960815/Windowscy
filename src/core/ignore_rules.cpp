/**
 * @file ignore_rules.cpp
 * @brief 忽略规则实现
 */

#include "core/ignore_rules.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <string>
#include <string_view>

namespace maccy {

namespace {

/**
 * @brief 标准化标识符
 * @details 将标识符转换为小写形式
 * @param value 要标准化的标识符
 * @return std::string 标准化后的标识符
 */
std::string NormalizeIdentifier(std::string_view value) {
  std::string normalized;
  normalized.reserve(value.size());

  for (unsigned char ch : value) {
    normalized.push_back(static_cast<char>(std::tolower(ch)));
  }

  return normalized;
}

/**
 * @brief 检查是否匹配列表中的值
 * @param values 值列表
 * @param candidate 要检查的候选值
 * @return bool 是否匹配
 */
bool MatchesListValue(const std::vector<std::string>& values, std::string_view candidate) {
  if (candidate.empty()) {
    return false;
  }

  const auto normalized_candidate = NormalizeIdentifier(candidate);
  return std::any_of(
      values.begin(),
      values.end(),
      [&normalized_candidate](const std::string& value) {
        return NormalizeIdentifier(value) == normalized_candidate;
      });
}

/**
 * @brief 检查格式是否启用
 * @param rules 忽略规则
 * @param blob 内容块
 * @return bool 格式是否启用
 */
bool IsFormatEnabled(const IgnoreRules& rules, const ContentBlob& blob) {
  const auto ignored_by_name = std::any_of(
      rules.ignored_formats.begin(),
      rules.ignored_formats.end(),
      [&blob](const std::string& ignored) {
        const auto normalized = NormalizeIdentifier(ignored);
        return normalized == NormalizeIdentifier(ContentFormatName(blob.format)) ||
               (!blob.format_name.empty() && normalized == NormalizeIdentifier(blob.format_name));
      });
  if (ignored_by_name) {
    return false;
  }

  switch (blob.format) {
    case ContentFormat::kPlainText:
      return rules.capture_text;
    case ContentFormat::kHtml:
      return rules.capture_html;
    case ContentFormat::kRtf:
      return rules.capture_rtf;
    case ContentFormat::kImage:
      return rules.capture_images;
    case ContentFormat::kFileList:
      return rules.capture_files;
    case ContentFormat::kCustom:
      return true;
  }

  return true;
}

/**
 * @brief 检查是否为文本内容块
 * @param blob 内容块
 * @return bool 是否为文本内容
 */
bool IsTextualBlob(const ContentBlob& blob) {
  return blob.format != ContentFormat::kImage;
}

/**
 * @brief 检查是否匹配忽略模式
 * @param rules 忽略规则
 * @param item 历史记录项
 * @return bool 是否匹配忽略模式
 */
bool MatchesIgnoredPattern(const IgnoreRules& rules, const HistoryItem& item) {
  if (rules.ignored_patterns.empty()) {
    return false;
  }

  std::vector<std::string> candidates = {
      item.title,
      item.PreferredDisplayText(),
      item.PreferredContentText(),
      item.metadata.source_application,
  };

  for (const auto& blob : item.contents) {
    if (IsTextualBlob(blob) && !blob.text_payload.empty()) {
      candidates.push_back(blob.text_payload);
    }
  }

  for (const auto& pattern_text : rules.ignored_patterns) {
    try {
      const std::regex pattern(pattern_text, std::regex::ECMAScript | std::regex::icase);
      for (const auto& candidate : candidates) {
        if (!candidate.empty() && std::regex_search(candidate, pattern)) {
          return true;
        }
      }
    } catch (const std::regex_error&) {
      continue;
    }
  }

  return false;
}

}  // namespace

IgnoreDecision ApplyIgnoreRules(const AppSettings& settings, HistoryItem item) {
  IgnoreDecision decision;
  decision.item = std::move(item);

  if (settings.ignore.ignore_all) {
    decision.reason = IgnoreReason::kIgnoreAll;
    return decision;
  }

  const auto& source_application = decision.item.metadata.source_application;
  if (settings.ignore.only_listed_applications &&
      !settings.ignore.allowed_applications.empty() &&
      !MatchesListValue(settings.ignore.allowed_applications, source_application)) {
    decision.reason = IgnoreReason::kNotAllowedApplication;
    return decision;
  }

  if (MatchesListValue(settings.ignore.ignored_applications, source_application)) {
    decision.reason = IgnoreReason::kIgnoredApplication;
    return decision;
  }

  std::erase_if(
      decision.item.contents,
      [&settings](const ContentBlob& blob) {
        return !IsFormatEnabled(settings.ignore, blob);
      });

  if (decision.item.contents.empty()) {
    decision.reason = IgnoreReason::kIgnoredFormat;
    return decision;
  }

  if (NormalizeForSearch(decision.item.PreferredDisplayText()).empty() &&
      NormalizeForSearch(decision.item.PreferredContentText()).empty()) {
    decision.reason = IgnoreReason::kEmptyItem;
    return decision;
  }

  if (MatchesIgnoredPattern(settings.ignore, decision.item)) {
    decision.reason = IgnoreReason::kIgnoredPattern;
    return decision;
  }

  decision.reason = IgnoreReason::kNone;
  decision.should_store = true;
  return decision;
}

}  // namespace maccy
