#include "stdafx.h"
#include "ui_container.h"
#include "algorithm.h"
#include "module_cargo.h"
#include "ui_button.h"
#include "ui.h"
#include "ui_event.h"

mark::ui::container::container(const info& info)
	: window(info.pos)
	, m_ui(*info.ui)
	, m_container(*info.container)
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
			m_container.drop(pos, std::move(m_ui.grabbed))) {
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
			m_ui.grabbed = std::move(m_container.pick(pos));
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