#include "stdafx.h"
#include "ui_container.h"
#include "algorithm.h"
#include "module_cargo.h"
#include "ui_button.h"
#include "ui.h"
#include "ui_event.h"
#include "resource_manager.h"
#include "tick_context.h"

mark::ui::container::container(const info& info)
	: window(info.pos)
	, m_ui(*info.ui)
	, m_container(*info.container)
	, m_cargo_bg(info.rm->image("cargo-background.png"))
{
	auto& ui = *info.ui;
	auto& container = *info.container;
	const auto interior_size = container.interior_size();
	for (const auto& pos : range(interior_size)) {
		if (auto& module_ptr = container.modules()[pos.x + pos.y * 16]) {
			auto& module = *module_ptr;
			this->attach(pos, module);
		}
	}

}

void mark::ui::container::tick(mark::tick_context& context)
{
	mark::sprite::info sprite;
	sprite.image = m_cargo_bg;
	sprite.pos = vector<double>(this->pos());
	sprite.size = 64.f;
	sprite.frame = std::numeric_limits<size_t>::max();
	sprite.color = { 255, 255, 255, 200 };
	context.ui_sprites[-1].push_back(sprite);
	this->window::tick(context);
}

bool mark::ui::container::click(const event& event)
{
	if (const auto handled = this->window::click(event)) {
		return true;
	} else if (m_ui.grabbed) {
		auto& module = *m_ui.grabbed;
		const vector<int> module_size(module.size());
		const vector<double> relative_pos(event.cursor - this->pos());
		const auto pos = round(relative_pos / 16.) - module_size / 2;
		if (mark::error::code::success ==
			m_container.attach(pos, m_ui.grabbed)) {
			this->attach(pos, module);
			return true;
		}
	}
	return false;
}

void mark::ui::container::attach(
	mark::vector<int> pos,
	mark::module::base& module)
{
	const auto button_pos = pos * 16;
	mark::ui::button::info info;
	info.parent = this;
	info.size = module.size() * 16U;
	info.pos = button_pos;
	info.image = module.thumbnail();
	auto button_ptr = std::make_unique<mark::ui::button>(info);
	auto& button = *button_ptr;
	button.on_click.insert(
		[pos, this, &button](const event& event) {
		if (!m_ui.grabbed) {
			m_ui.grabbed = m_container.detach(pos);
			if (m_ui.grabbed) {
				m_ui.grabbed_prev_parent = &m_container;
				m_ui.grabbed_prev_pos = m_ui.grabbed->grid_pos();
			}
			this->remove(button);
			m_container.detachable();
		}
		return true;
	});
	const auto length = static_cast<int>(module.size().x) * 16;
	button.on_hover.insert(
		[this, &module, button_pos, length](const event& event) {
		m_ui.tooltip(
			this->pos() + button_pos + vector<int>(length, 0),
			module.describe());
		return true;
	});
	this->insert(std::move(button_ptr));
}