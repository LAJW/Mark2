#include "item_button.h"
#include <sprite.h>
#include <update_context.h>

mark::ui::item_button::item_button(const info& info)
	: button_base(info)
	, m_thumbnail(info.thumbnail)
{}

void mark::ui::item_button::update(update_context& context)
{
	this->render(context);
}

void mark::ui::item_button::render(update_context& context) const
{
	let size = this->size();
	context.sprites[102].emplace_back([&] {
		sprite _;
		_.image = m_thumbnail;
		let pos = vd(this->pos());
		_.pos = pos - vd(0, (size.x - size.y) / 2.);
		_.size = static_cast<float>(std::max(size.x, size.y));
		_.world = false;
		_.centred = false;
		return _;
	}());
}
