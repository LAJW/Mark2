#include "inventory.h"
#include "container.h"
#include <algorithm.h>
#include <module/base.h>
#include <module/cargo.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/container.h>
#include <ui/ui.h>
#include <unit/modular.h>
#include <update_context.h>

let window_margin = mark::vi32(33, 33);
let constexpr chunk_size = 128;
let container_count = 8;
let window_size = mark::vi32(16 * 16, (16 * 4 + 32) * container_count);

mark::ui::inventory::inventory(const info& info)
	: window(info)
	, m_modular(*info.modular)
	, m_ui(*info.ui)
	, m_rm(*info.rm)
	, m_header(info.rm->image("inventory-header.png"))
	, m_background(info.rm->image("window-background.png"))
	, m_grid_bg(info.rm->image("grid-background.png"))
{}

/// Calculate offset for specified position N
static auto offset_and_frame_of(int n, int margin, int chunk, int size)
	-> std::pair<int, int>
{
	if (n == 0) {
		return { -margin, 0 };
	}
	if (-margin * 2 + chunk * (n + 1) < size) {
		return { -margin + chunk * n, 1 };
	}
	return { size - chunk + margin, 2 };
}

static auto offset_and_frame_of(mark::vi32 i) -> std::pair<mark::vi32, int>
{
	let[offset_x, frame_x] =
		offset_and_frame_of(i.x, window_margin.x, chunk_size, window_size.x);
	let[offset_y, frame_y] =
		offset_and_frame_of(i.y, window_margin.y, chunk_size, window_size.y);
	return { mark::vi32(offset_x, offset_y), frame_x + frame_y * 3 };
}

void mark::ui::inventory::update(update_context& context)
{
	let modular = m_ui.landed_modular();
	if (!modular) {
		return;
	}
	for (let i : range(window_size / chunk_size + vi32(1, 1))) {
		context.sprites[100].push_back([&] {
			let[offset, frame] = offset_and_frame_of(i);
			sprite _;
			_.world = false;
			_.centred = false;
			_.size = chunk_size;
			_.frame = frame;
			_.pos = vd(pos() + offset);
			_.image = m_background;
			return _;
		}());
	}
	let containers = modular->containers();
	let[removed, added] =
		diff(this->children(), containers, [](let& a, let& b) {
			let& container = dynamic_cast<const mark::ui::container&>(*a.get());
			let item_count = count_if(b.get().items(), [](let& item) {
				return item.get() != nullptr;
			});
			return &container.cargo() == &b.get()
				&& container.children().size() == item_count;
		});
	for (let& it : removed) {
		this->erase(it);
	}
	for (let& pair : added) {
		auto& [it, container] = pair;
		auto container_window = std::make_unique<mark::ui::container>([&] {
			mark::ui::container::info _;
			_.rm = m_rm;
			_.container = container.get();
			_.ui = m_ui;
			_.relative = true;
			return _;
		}());
		this->insert(it, move(container_window));
	}
	this->window::update(context);
}

auto mark::ui::inventory::size() const -> vi32
{
	return window_size;
}

auto mark::ui::inventory::pos() const noexcept -> vi32 { return m_pos; }

auto mark::ui::inventory::hover(const event& event) -> bool
{
	m_mouse_pos = event.world_cursor;
	return this->mark::ui::window::hover(event);
}
