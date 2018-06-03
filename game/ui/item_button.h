#pragma once
#include <stdafx.h>
#include <ui/button.h>

namespace mark {
namespace ui {
class ui;

class item_button final : public button_base
{
public:
	struct info : button_base::info
	{
		resource::image_ptr font;
		optional<const mark::interface::item&> item;
		optional<mark::ui::ui&> ui;
		// Is this button the "origin" - i.e. the thing which points to the
		// item you can grab
		bool origin = false;
	};

	item_button(const info&);
	void update(update_context&) override;

private:
	void render(update_context&) const;

	const not_null<resource::image_ptr> m_font;
	const mark::interface::item& m_item;
	mark::ui::ui& m_ui;
	bool m_origin;
};

}
}