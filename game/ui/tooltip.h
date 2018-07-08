#pragma once
#include <adsr.h>
#include <stdafx.h>

namespace mark {
namespace ui {

class tooltip final
{
public:
	explicit tooltip(resource::manager& rm) noexcept;
	/// Update tooltip state, render
	void update(update_context&) noexcept;
	/// Set tooltip to render at a specified screen/world position with supplied
	/// text. object_id is there to force redrawing of the tooltip when pointing
	/// at a different object, while allowing to update existing tooltip
	/// information without triggering fade-in animation
	void
	set(std::variant<vd, vi32> pos,
		const void* object_id,
		const std::string& text);

private:
	/// Render a tooltip in screen coordinates
	void render(update_context&) const;

	std::string m_text;
	std::variant<vd, vi32> m_pos;
	const resource::image_ptr m_font;
	const resource::image_ptr m_background;
	adsr m_adsr;
	adsr m_load;
	const void* m_object_id = nullptr;
};

} // namespace ui
} // namespace mark
