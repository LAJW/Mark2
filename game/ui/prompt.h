#pragma once
#include <stdafx.h>

namespace mark {
class mode_stack;

namespace ui {
class window;

[[nodiscard]] unique_ptr<window> make_prompt(mark::resource::manager& rm);

} // namespace ui
} // namespace mark
