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
	/// text
	void set(std::variant<vd, vi32> pos, const std::string& text);

private:
	/// Render a tooltip in screen coordinates
	void render(update_context&) const;

	std::string m_text;
	std::variant<vd, vi32> m_pos;
	const resource::image_ptr m_font;
	const resource::image_ptr m_background;
	adsr m_adsr;
	adsr m_load;
};

} // namespace ui
} // namespace mark
