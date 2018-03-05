﻿#include "stdafx.h"
#include "ui_container.h"
#include "algorithm.h"
#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "ui_button.h"
#include "ui.h"
#include "ui_event.h"

mark::ui::container::container(const info& info)
	: window(info.pos)
	, m_ui(*info.ui)
	, m_container(*info.container)
	, m_cargo_bg(info.rm->image("cargo-background.png"))
{
	auto& container = *info.container;
	let interior_size = container.interior_size();
	for (let& pos : range(interior_size)) {
		if (auto& module_ptr = container.modules()[pos.x + pos.y * 16]) {
			auto& module = *module_ptr;
			this->attach(pos, module);
		}
	}

}

void mark::ui::container::tick(tick_context& context)
{
	sprite sprite;
	sprite.image = m_cargo_bg;
	sprite.pos = vector<double>(this->pos());
	sprite.size = 64.f;
	sprite.frame = std::numeric_limits<size_t>::max();
	sprite.color = { 255, 255, 255, 200 };
	sprite.world = false;
	sprite.centred = false;
	context.sprites[100].push_back(sprite);
	this->window::tick(context);
}

bool mark::ui::container::click(const event& event)
{
	if (let handled = this->window::click(event)) {
		return true;
	} else if (m_ui.grabbed) {
		auto& module = *m_ui.grabbed;
		const vector<int> module_size(module.size());
		const vector<double> relative_pos(event.cursor - this->pos());
		let pos = round(relative_pos / 16.) - module_size / 2;
		if (error::code::success ==
			m_container.attach(pos, m_ui.grabbed)) {
			this->attach(pos, module);
			return true;
		}
	}
	return false;
}

auto mark::ui::container::cargo() const -> const module::cargo&
{ return m_container; }

auto mark::ui::container::size() const -> vector<int>
{ return vector<int>(m_container.interior_size()) * static_cast<int>(mark::module::size * 1.5f); }

void mark::ui::container::attach(
	vector<int> pos,
	module::base& module)
{
	let button_pos = pos * 16;
	mark::ui::button::info info;
	info.parent = this;
	info.size = module.size() * 16U;
	info.pos = button_pos;
	info.image = module.thumbnail();
	auto button_ptr = std::make_unique<mark::ui::button>(info);
	auto& button = *button_ptr;
	button.on_click.insert(
		[pos, this, &button](const event&) {
		if (!m_ui.grabbed) {
			m_ui.grabbed = m_container.detach(pos);
			if (m_ui.grabbed) {
				m_ui.grabbed_prev_parent = &m_container;
				m_ui.grabbed_prev_pos = m_ui.grabbed->grid_pos();
				m_ui.grabbed_bind.clear();
			}
			this->remove(button);
			m_container.detachable();
		}
		return true;
	});
	let length = static_cast<int>(module.size().x) * 16;
	button.on_hover.insert(
		[this, &module, button_pos, length](const event&) {
		m_ui.tooltip(
			this->pos() + button_pos + vector<int>(length, 0),
			module.describe());
		return true;
	});
	this->insert(std::move(button_ptr));
}