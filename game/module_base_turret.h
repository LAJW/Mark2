#pragma once
#include "stdafx.h"
#include "module_base.h"
#include <deque>

namespace mark {
namespace unit {
class base;
}
namespace module {
class base_turret : public module::base {
public:
	base_turret(
		vector<unsigned> size,
		const std::shared_ptr<const resource::image>& image);
	base_turret(resource::manager&, const YAML::Node& node);
	void target(vector<double> pos) override;
	void shoot(vector<double> pos, bool release) override;
	void queue(vector<double> pos, bool release) override;
	auto passive() const noexcept -> bool override;
protected:
	void tick_ai();
	// Returns true if there are objects still to be destroyed before
	// switching to manual control
	auto queued() const -> bool;
	auto shoot() const -> bool;
	auto target() const -> vector<double>;
	void serialize_base(YAML::Emitter&) const;
private:
	vector<double> m_target;
	std::deque<
		std::pair<
			std::weak_ptr<unit::base>,
			vector<double>>> m_queue;
	bool m_shoot = false;
};
} }