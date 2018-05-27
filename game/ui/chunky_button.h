#pragma once
#include <stdafx.h>
#include <ui/button.h>

namespace mark {
namespace ui {

class chunky_button final : public button_base
{
public:
	struct info : button_base::info
	{
		std::string text;
		resource::image_ptr background = nullptr;
		resource::image_ptr font = nullptr;
	};

	chunky_button(const info&);
	void update(update_context&) override;

private:
	void render(update_context&) const;

	const std::string m_text;
	const not_null<resource::image_ptr> m_background;
	const not_null<resource::image_ptr> m_font;
	double m_opacity = 0.;
};

}
}