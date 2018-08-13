#include "inventory.h"
#include <algorithm/diff.h>
#include <algorithm.h>
#include <module/cargo.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/container.h>
#include <ui/ui.h>
#include <unit/modular.h>
#include <update_context.h>

mark::ui::inventory::inventory(const info& info)
	: chunky_window(info)
	, m_modular(*info.modular)
	, m_ui(*info.ui)
	, m_rm(*info.rm)
{}

void mark::ui::inventory::update(update_context& context)
{
	this->chunky_window::update(context);
	let modular = m_ui.landed_modular();
	if (!modular) {
		return;
	}
	let containers = modular->containers();
	let children = this->children();
	let[removed, added] =
		diff(children, containers, [](let& a, let& b) {
			let& container = dynamic_cast<const mark::ui::container&>(a.get());
			let item_count = count_if(b.get().items(), [](let& item) {
				return item.get() != nullptr;
			});
			return &container.cargo() == &b.get()
				&& container.children().size() == item_count;
		});
	for (let& it : removed) {
		(void)this->remove(it->get());
	}
	for (let& [it, container] : added) {
		auto container_window = std::make_unique<mark::ui::container>([&] {
			mark::ui::container::info _;
			_.rm = m_rm;
			_.container = container.get();
			_.ui = m_ui;
			_.relative = true;
			return _;
		}());
		if (it == children.end()) {
			this->append(move(container_window));
		} else {
			this->insert(it->get(), move(container_window));
		}
	}
	this->window::update(context);
}

auto mark::ui::inventory::hover(const event& event) -> bool
{
	m_mouse_pos = event.world_cursor;
	return this->mark::ui::window::hover(event);
}
