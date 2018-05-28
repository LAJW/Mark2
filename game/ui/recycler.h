#pragma once
#include <slot.h>
#include <ui/chunky_window.h>
#include <array2d.h>

namespace mark {
namespace ui {
class recycler : public chunky_window
{
public:
	struct info : chunky_window::info
	{
		optional<mark::unit::modular&> modular;
	};
	recycler(const info&);
	void update(update_context&) override;
	void recycle(interface::container& container, vi32 pos) noexcept;

private:
	array2d<mark::slot, 16, 32> m_queue;
	mark::unit::modular& m_modular;
};
} // namespace ui
} // namespace mark
