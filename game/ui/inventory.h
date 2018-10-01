#pragma once
#include <ui/chunky_window.h>

namespace mark {

namespace ui {
class ui;
struct event;

class inventory final : public chunky_window
{
public:
	struct info : chunky_window::info
	{
		optional<const mark::unit::modular&> modular;
		optional<const mark::ui::ui&> ui;
	};
	inventory(const info& info);
	void update(update_context&) override;
	[[nodiscard]] handler_result hover(const event&) override;

private:
	inventory(const info& info, bool);

	const ui& m_ui;
	resource::manager& m_rm;
	const mark::unit::modular& m_modular;
	vd m_mouse_pos;
};
} // namespace ui
} // namespace mark
