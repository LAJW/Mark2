#pragma once
#include <stdafx.h>

namespace mark {
class mode_stack;

namespace ui {
class window;

auto make_options(mark::resource::manager& rm, mark::mode_stack& stack)
	-> unique_ptr<window>;

} // namespace ui
} // namespace mark
