#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {

class tooltip final
{
public:
	explicit tooltip(resource::manager& rm) noexcept;
	/// Update tooltip state, render
	void update(update_context&) noexcept;
	/// Set tooltip to render at a specified screen position with supplied text
	void set(vi32 pos, const std::string& text);
	/// Set tooltip to render at a specified world position with supplied text
	void set(vd pos, const std::string& text);

private:
	/// Render a tooltip in screen coordinates
	void render(update_context&) const;

	std::string m_text;
	vd m_pos;
	const resource::image_ptr m_font;
	const resource::image_ptr m_background;
	bool m_world = false;
};

} // namespace ui
} // namespace mark
