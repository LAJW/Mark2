#include "recycler.h"
#include <algorithm.h>
#include <item/shard.h>
#include <module/base.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/chunky_button.h>
#include <ui/ui.h>
#include <unit/modular.h>
#include <update_context.h>

mark::ui::recycler::recycler(const info& info)
	: chunky_window(info)
	, m_modular(*info.modular)
{
	auto& rm = *info.rm;
	auto recycle_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-blue-button.png");
		_.pos = { 0, 770 - 46 };
		_.relative = false;
		_.size = { 130, 50 };
		_.text = "Recycle";
		return _;
	}());
	recycle_button->on_click.insert([&](let&) {
		for (auto& slot : m_queue) {
			(void)detach(slot);
			(void)push(m_modular, std::make_unique<item::shard>(rm));
		}
		m_queue.clear();
		return true;
	});
	this->insert(move(recycle_button));
	auto cancel_recycle_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-red-button.png");
		_.pos = { 130, 770 - 46 };
		_.relative = false;
		_.size = { 130, 50 };
		_.text = "Cancel";
		return _;
	}());
	cancel_recycle_button->on_click.insert([&](let&) {
		m_queue.clear();
		return true;
	});
	this->insert(move(cancel_recycle_button));
}

void mark::ui::recycler::update(update_context& context)
{
	this->chunky_window::update(context);
	// TODO: Pull the actual resolution
	vi32 resolution = { 1920, 1080 };
	for (let & [ i, slot ] : enumerate(m_queue)) {
		context.sprites[100].emplace_back([&] {
			let constexpr item_size = 4 * module::size;
			let& item = item_of(slot);
			let item_pos_x = static_cast<double>(i) * 1.5 * item_size;
			sprite _;
			_.image = item.thumbnail();
			_.pos = { resolution.x - item_size, item_pos_x };
			_.size = item_size;
			_.centred = false;
			_.world = false;
			return _;
		}());
	}
}

void mark::ui::recycler::recycle(
	interface::container& container,
	vi32 pos) noexcept
{
	if (all_of(m_queue, [&](let& slot) {
			return slot != mark::slot{ container, pos };
		})) {
		m_queue.emplace_back(container, pos);
	}
}
