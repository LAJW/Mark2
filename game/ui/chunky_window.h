#pragma once
#include <ui/window.h>

namespace mark {
namespace ui {

/// Window with a really thick 90's chunky border
class chunky_window : public window
{
public:
	struct info : window::info
	{
		optional<resource::manager&> rm;
		vu32 size = mark::vu32(16 * 16, 16 * 16);
	};
	chunky_window(const info&);

	void update(update_context&) override;
	auto size() const -> vi32 override;
	auto pos() const noexcept -> vi32 override;
	void pos(const vi32&);

private:
	void render(update_context&) const noexcept;

	const not_null<resource::image_ptr> m_background;
	vi32 m_size;
};

} // namespace ui
} // namespace mark
