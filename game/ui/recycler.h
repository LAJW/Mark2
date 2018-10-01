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
		optional<const ui&> ui;
		optional<const queue_type&> queue;
	};
	recycler(const info&);
	void update(update_context&) override;
	void render(update_context&) const;
	[[nodiscard]] bool has(const mark::interface::item& item) const noexcept;

private:
	const ui& m_ui;
	const queue_type& m_queue;
	const mark::resource::image_ptr m_font;
	const mark::resource::image_ptr m_grid;
};

} // namespace ui
} // namespace mark
