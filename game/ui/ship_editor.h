#pragma once
#include <stdafx.h>
#include <ui/window.h>

namespace mark {
namespace ui {

class ui;
class ship_editor final : public window
{
public:
	struct info : window::info
	{
		optional<const ui&> ui;
		optional<resource::manager&> resource_manager;
		optional<const unit::modular&> modular;
		vi32 resolution;
	};

	ship_editor(const info& info);
	[[nodiscard]] handler_result click(const event& event) override;
	[[nodiscard]] handler_result hover(const event& event) override;
	void update(update_context& context) override;

private:
	[[nodiscard]] handler_result drop(const vd relative) const;
	[[nodiscard]] handler_result
	drag(const vd relative, const bool shift) const;
	void update_recycler_position(const vi32 resolution);

	const ui& m_ui;
	resource::image_ptr m_grid_bg;
};

} // namespace ui
} // namespace mark
