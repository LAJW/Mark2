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
	};
	chunky_window(const info&);

	void update(update_context&) override;
	auto size() const -> vi32 override;
	auto pos() const noexcept -> vi32 override;

private:
	void render(update_context&) const noexcept;

	const not_null<resource::image_ptr> m_background;
};

} // namespace ui
} // namespace mark
