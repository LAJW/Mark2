#pragma once
#include "command.h"
#include "stdafx.h"

namespace mark {
namespace interface {
class world_object;
};
namespace unit {
class base;
}

class targeting_system final
{
public:
	targeting_system(interface::world_object& parent);
	void command(const command::any&);
	void update(update_context& update_context);
	// Can the inheriting turret perform a shot in this frame.
	auto can_shoot() const -> bool;
	// Target at which turret is pointing
	auto target() const -> std::optional<vd>;
	// Is turret in the AI mode (is it using queue for targeting)
	auto ai() const -> bool;

protected:
private:
	void target(vd pos);
	void queue(vd pos, bool release);

	// Queued modules to shoot (units with offset) or a world position for
	// dumb fire
	using target_type = std::pair<bool, vd>;
	using queue_type =
		std::deque<std::pair<weak_ptr<const unit::base>, vd>>;
	std::variant<target_type, queue_type> m_target;
	interface::world_object& m_parent;
};
} // namespace mark
