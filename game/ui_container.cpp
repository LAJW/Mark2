#include "stdafx.h"
#include "ui_container.h"
#include "algorithm.h"
#include "module_cargo.h"
#include "ui_button.h"
#include "ui.h"

mark::ui::container::container(const info& info)
	: window(info.pos)
{
	auto& ui = *info.ui;
	auto& container = *info.container;
	const auto interior_size = container.interior_size();
	for (const auto& pos : range(interior_size)) {
		if (auto& module_ptr = container.modules()[pos.x + pos.y * 16]) {
			auto& module = *module_ptr;
			mark::ui::button::info info;
			info.parent = this;
			info.size = module.size() * 16U;
			const vector<int> button_pos(pos.x * 16, pos.y * 16);
			info.pos = button_pos;
			info.image = module.thumbnail();
			auto button_ptr = std::make_unique<mark::ui::button>(info);
			auto& button = *button_ptr;
			button.on_click.insert(
				[&ui, &container, pos, this, &button](const event& event) {
				if (!ui.grabbed) {
					ui.grabbed = std::move(container.pick(pos));
					this->remove(button);
					container.detachable();
				}
				return true;
			});
			const auto length = static_cast<int>(module.size().x) * 16;
			button.on_hover.insert(
				[&ui, this, &module, button_pos, length](const event& event) {
				ui.tooltip(
					this->pos() + button_pos + vector<int>(length, 0),
					module.describe());
				return true;
			});
			this->insert(std::move(button_ptr));
		}
	}

}

void mark::ui::container::tick(mark::tick_context& context)
{
	this->window::tick(context);
}