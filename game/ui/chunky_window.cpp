#include "chunky_window.h"
#include <algorithm/range.h>
#include <resource_manager.h>
#include <sprite.h>
#include <update_context.h>

let window_margin = mark::vi32(33, 33);
let constexpr chunk_size = 128;

mark::ui::chunky_window::chunky_window(const info& info)
	: window(info)
	, m_background(info.rm->image("window-background.png"))
	, m_size(info.size)
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

static auto offset_and_frame_of(mark::vi32 i, mark::vi32 window_size)
	-> std::pair<mark::vi32, int>
{
	let[offset_x, frame_x] =
		offset_and_frame_of(i.x, window_margin.x, chunk_size, window_size.x);
	let[offset_y, frame_y] =
		offset_and_frame_of(i.y, window_margin.y, chunk_size, window_size.y);
	return { mark::vi32(offset_x, offset_y), frame_x + frame_y * 3 };
}

void mark::ui::chunky_window::update(update_context& context)
{
	this->render(context);
	this->window::update(context);
}

void mark::ui::chunky_window::render(update_context& context) const noexcept
{
	for (let i : range(size() / chunk_size + vi32(1, 1))) {
		context.sprites[100].push_back([&] {
			let[offset, frame] = offset_and_frame_of(i, size());
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
}

auto mark::ui::chunky_window::size() const -> vi32 { return m_size; }

auto mark::ui::chunky_window::pos() const noexcept -> vi32 { return m_pos; }

void mark::ui::chunky_window::pos(const vi32& pos) { m_pos = pos; }
