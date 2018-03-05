#pragma once
#include "stdafx.h"
#include "command.h"

namespace mark {
namespace interface {
class world_object;
};
namespace unit {
class base;
}

class targeting_system final {
public:
	targeting_system(interface::world_object& parent);
	void command(const command::any&);
	void tick();
	// Can the inheriting turret perform a shot in this frame.  Mutable, as
	// requesting charge might reset the internal clock.
	auto request_charge() -> bool;
	// Target at which turret is pointing
	auto target() const -> std::optional<vector<double>>;
	// Level of turret's charge (between 0 and 1)
protected:
private:
	void target(vector<double> pos);
	void queue(vector<double> pos, bool release);

	// Queued modules to shoot (units with offset) or a world position for
	// dumb fire
	using target_type = std::pair<bool, vector<double>>;
	using queue_type = std::deque<
		std::pair<
			std::weak_ptr<const unit::base>,
			vector<double>>>;
	std::variant<target_type, queue_type> m_target;
	interface::world_object& m_parent;
};
}