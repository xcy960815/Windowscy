#include <cassert>
#include <chrono>

#include "core/double_click_modifier.h"

namespace {

using Detector = maccy::DoubleClickModifierKeyDetector;
using Key = maccy::DoubleClickModifierKey;
using TimePoint = Detector::Clock::time_point;

TimePoint AtMilliseconds(int milliseconds) {
  return TimePoint(std::chrono::milliseconds(milliseconds));
}

}  // namespace

int main() {
  {
    Detector detector;
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagAlt, AtMilliseconds(0)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(100)).has_value());
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagAlt, AtMilliseconds(200)).has_value());
    const auto key = detector.HandleModifierFlagsChanged(0, AtMilliseconds(300));
    assert(key.has_value() && *key == Key::kAlt);
  }

  {
    Detector detector;
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagShift, AtMilliseconds(0)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(100)).has_value());
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagShift, AtMilliseconds(200)).has_value());
    const auto key = detector.HandleModifierFlagsChanged(0, AtMilliseconds(300));
    assert(key.has_value() && *key == Key::kShift);
  }

  {
    Detector detector;
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagControl, AtMilliseconds(0)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(100)).has_value());
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagControl, AtMilliseconds(200)).has_value());
    const auto key = detector.HandleModifierFlagsChanged(0, AtMilliseconds(300));
    assert(key.has_value() && *key == Key::kControl);
  }

  {
    Detector detector;
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagAlt, AtMilliseconds(0)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(100)).has_value());
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagAlt, AtMilliseconds(1000)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(1100)).has_value());
  }

  {
    Detector detector;
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagAlt, AtMilliseconds(0)).has_value());
    detector.HandleKeyDown();
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(100)).has_value());
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagAlt, AtMilliseconds(200)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(300)).has_value());
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagAlt, AtMilliseconds(400)).has_value());
    const auto key = detector.HandleModifierFlagsChanged(0, AtMilliseconds(500));
    assert(key.has_value() && *key == Key::kAlt);
  }

  {
    Detector detector;
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagAlt, AtMilliseconds(0)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(100)).has_value());
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagShift, AtMilliseconds(200)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(300)).has_value());
  }

  {
    Detector detector;
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagWin, AtMilliseconds(0)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(100)).has_value());
    assert(!detector.HandleModifierFlagsChanged(maccy::kDoubleClickModifierFlagWin, AtMilliseconds(200)).has_value());
    assert(!detector.HandleModifierFlagsChanged(0, AtMilliseconds(300)).has_value());
  }

  return 0;
}
