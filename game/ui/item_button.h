#pragma once
#include <stdafx.h>
#include <ui/button.h>

namespace mark {
namespace ui {

class item_button final : public button_base
{
public:
	struct info : button_base::info
	{
		resource::image_ptr thumbnail = nullptr;
	};

	item_button(const info&);
	void update(update_context&) override;

private:
	void render(update_context&) const;

	const not_null<resource::image_ptr> m_thumbnail;
};

}
}