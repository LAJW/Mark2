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
		optional<const interface::item&> item;
		optional<const ui&> ui;
		// Is this button the "origin" - i.e. the thing which points to the
		// item you can grab
		bool origin = false;
	};

	item_button(const info&);
	void update(update_context&) override;
	[[nodiscard]] const interface::item& item() const { return m_item; }

private:
	void render(update_context&) const;

	const not_null<resource::image_ptr> m_font;
	const interface::item& m_item;
	const mark::ui::ui& m_ui;
	bool m_origin;
};

}
}