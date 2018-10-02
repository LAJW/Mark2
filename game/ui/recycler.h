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
	struct info : chunky_window::info
	{
		optional<const ui&> ui;
	};
	recycler(const info&);
	void update(update_context&) override;
	void render(update_context&) const;
	[[nodiscard]] bool has(const mark::interface::item& item) const noexcept;

private:
	const ui& m_ui;
	const mark::resource::image_ptr m_font;
	const mark::resource::image_ptr m_grid;
};

} // namespace ui
} // namespace mark
