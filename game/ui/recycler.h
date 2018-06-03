#pragma once
#include <array2d.h>
#include <slot.h>
#include <ui/chunky_window.h>

namespace mark {

namespace ui {
class ui;
class tooltip;
class recycler : public chunky_window
{
public:
	using queue_type = array2d<mark::slot, 16, 32>;
	struct info : chunky_window::info
	{
		optional<mark::unit::modular&> modular;
		optional<tooltip&> tooltip;
		optional<ui&> ui;
	};
	recycler(const info&);
	void update(update_context&) override;
	void recycle(interface::container& container, vi32 pos) noexcept;
	[[nodiscard]] auto has(const mark::interface::item& item) const noexcept
		-> bool;

private:
	ui& m_ui;
	queue_type m_queue;
	tooltip& m_tooltip;
	mark::unit::modular& m_modular;
	const mark::resource::image_ptr m_font;
	const mark::resource::image_ptr m_grid;
};

} // namespace ui
} // namespace mark
