#include "core/double_click_modifier.h"

#include <utility>

namespace maccy {

std::string_view ToString(DoubleClickModifierKey key) {
  switch (key) {
    case DoubleClickModifierKey::kAlt:
      return "alt";
    case DoubleClickModifierKey::kShift:
      return "shift";
    case DoubleClickModifierKey::kControl:
      return "control";
    case DoubleClickModifierKey::kNone:
    default:
      return "none";
  }
}

DoubleClickModifierKey ParseDoubleClickModifierKey(std::string_view value) {
  if (value == "alt") {
    return DoubleClickModifierKey::kAlt;
  }
  if (value == "shift") {
    return DoubleClickModifierKey::kShift;
  }
  if (value == "control") {
    return DoubleClickModifierKey::kControl;
  }
  return DoubleClickModifierKey::kNone;
}

std::uint32_t ModifierFlagsForDoubleClickModifierKey(DoubleClickModifierKey key) {
  switch (key) {
    case DoubleClickModifierKey::kAlt:
      return kDoubleClickModifierFlagAlt;
    case DoubleClickModifierKey::kShift:
      return kDoubleClickModifierFlagShift;
    case DoubleClickModifierKey::kControl:
      return kDoubleClickModifierFlagControl;
    case DoubleClickModifierKey::kNone:
    default:
      return 0;
  }
}

DoubleClickModifierKey StandaloneDoubleClickModifierKey(std::uint32_t modifier_flags) {
  switch (modifier_flags) {
    case kDoubleClickModifierFlagAlt:
      return DoubleClickModifierKey::kAlt;
    case kDoubleClickModifierFlagShift:
      return DoubleClickModifierKey::kShift;
    case kDoubleClickModifierFlagControl:
      return DoubleClickModifierKey::kControl;
    default:
      return DoubleClickModifierKey::kNone;
  }
}

DoubleClickModifierKeyDetector::DoubleClickModifierKeyDetector(
    std::chrono::milliseconds double_click_interval)
    : double_click_interval_(double_click_interval) {}

void DoubleClickModifierKeyDetector::Reset() {
  active_key_ = DoubleClickModifierKey::kNone;
  last_standalone_tap_.reset();
  current_press_used_with_chord_ = false;
}

void DoubleClickModifierKeyDetector::HandleKeyDown() {
  if (active_key_ == DoubleClickModifierKey::kNone) {
    last_standalone_tap_.reset();
  } else {
    current_press_used_with_chord_ = true;
  }
}

std::optional<DoubleClickModifierKey> DoubleClickModifierKeyDetector::HandleModifierFlagsChanged(
    std::uint32_t modifier_flags,
    Clock::time_point now) {
  if (active_key_ == DoubleClickModifierKey::kNone) {
    if (const auto key = StandaloneDoubleClickModifierKey(modifier_flags);
        key != DoubleClickModifierKey::kNone) {
      active_key_ = key;
      current_press_used_with_chord_ = false;
    } else if (modifier_flags != 0) {
      last_standalone_tap_.reset();
    }
    return std::nullopt;
  }

  const std::uint32_t active_flags = ModifierFlagsForDoubleClickModifierKey(active_key_);
  if ((modifier_flags & active_flags) != 0) {
    if (modifier_flags != active_flags) {
      current_press_used_with_chord_ = true;
    }
    return std::nullopt;
  }

  const auto released_key = active_key_;
  active_key_ = DoubleClickModifierKey::kNone;

  const bool current_press_used_with_chord = current_press_used_with_chord_;
  current_press_used_with_chord_ = false;

  if (current_press_used_with_chord) {
    last_standalone_tap_.reset();
    return std::nullopt;
  }

  if (last_standalone_tap_.has_value() &&
      last_standalone_tap_->first == released_key &&
      now - last_standalone_tap_->second < double_click_interval_) {
    last_standalone_tap_.reset();
    return released_key;
  }

  last_standalone_tap_ = std::make_pair(released_key, now);
  return std::nullopt;
}

}  // namespace maccy
