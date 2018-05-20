#include "container.h"
#include "button.h"
#include "event.h"
#include "ui.h"
#include <algorithm.h>
#include <module/cargo.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>

mark::ui::container::container(const info& info)
	: window(info)
	, m_ui(*info.ui)
	, m_container(*info.container)
	, m_cargo_bg(info.rm->image("cargo-background.png"))
{
	auto& container = *info.container;
	let interior_size = container.interior_size();
	for (let& pos : range(interior_size)) {
		if (auto& item_ptr = container.items()[pos.x + pos.y * 16]) {
			auto& item = *item_ptr;
			this->attach(pos, item);
		}
	}
}

void mark::ui::container::update(update_context& context)
{
	context.sprites[100].push_back([&] {
		sprite _;
		_.image = m_cargo_bg;
		_.pos = vd(this->pos());
		_.size = 64.f;
		_.frame = std::numeric_limits<size_t>::max();
		_.color = { 255, 255, 255, 200 };
		_.world = false;
		_.centred = false;
		return _;
	}());
	this->window::update(context);
}

bool mark::ui::container::click(const event& event)
{
	if (let handled = this->window::click(event)) {
		return true;
	}
	let grabbed = m_ui.grabbed();
	if (!grabbed) {
		return false;
	}
	auto& module = *grabbed;
	const vi32 module_size(module.size());
	const vd relative_pos(event.cursor - this->pos());
	let pos = round(relative_pos / 16.) - module_size / 2;
	if (!m_container.can_attach(pos, module)) {
		return false;
	}
	let result = m_container.attach(pos, detach(m_ui.m_grabbed));
	Expects(result == error::code::success || result == error::code::stacked);
	this->attach(pos, module);
	return false;
}

auto mark::ui::container::cargo() const -> const module::cargo&
{
	return m_container;
}

auto mark::ui::container::size() const -> vi32
{
	return vi32(m_container.interior_size())
		* static_cast<int>(mark::module::size * 1.5f);
}

void mark::ui::container::attach(vi32 pos, interface::item& item)
{
	let button_pos = pos * 16;
	auto button_ptr = std::make_unique<mark::ui::button>([&] {
		mark::ui::button::info _;
		_.size = item.size() * 16U;
		_.pos = button_pos;
		_.image = item.thumbnail();
		_.relative = false;
		return _;
	}());
	auto& button = *button_ptr;
	button.on_click.insert([pos, this, &button](const event& event) {
		if (m_ui.grabbed()) {
			// TODO: Propagate error/notify user that object cannot be put here
			(void)m_container.attach(pos, detach(m_ui.m_grabbed));
			return true;
		}
		let actual_pos = m_container.pos_at(pos);
		if (actual_pos) {
			if (event.shift) {
				// TODO: Put into the "recycler"
				(void)m_container.detach(pos);
			} else {
				m_ui.m_grabbed = { m_container, *actual_pos };
			}
		}
		// Don't call anything after this. This call destroys "this" lambda.
		this->remove(button);
		return true;
	});
	let length = static_cast<int>(item.size().x) * 16;
	button.on_hover.insert([=, &item](const event&) {
		m_ui.tooltip(
			this->pos() + button_pos + vi32(length, 0), item.describe());
		return true;
	});
	this->insert(move(button_ptr));
}
