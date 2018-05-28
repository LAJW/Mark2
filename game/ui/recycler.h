#pragma once
#include <slot.h>
#include <ui/chunky_window.h>

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
	std::vector<mark::slot> m_queue;
	mark::unit::modular& m_modular;
};
} // namespace ui
} // namespace mark
