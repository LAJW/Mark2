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
		resource::image_ptr thumbnail;
		resource::image_ptr font;
		optional<const mark::interface::item&> item;
		optional<const mark::ui::ui&> ui;
	};

	item_button(const info&);
	void update(update_context&) override;

private:
	void render(update_context&) const;

	const not_null<resource::image_ptr> m_thumbnail;
	const not_null<resource::image_ptr> m_font;
	const mark::interface::item& m_item;
	optional<const mark::ui::ui&> m_ui;
};

}
}