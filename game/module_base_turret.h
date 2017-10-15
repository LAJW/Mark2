#pragma once
#include "stdafx.h"
#include "module_base.h"
#include <deque>

namespace mark { namespace module {
class base_turret : public mark::module::base {
public:
	base_turret(
		mark::vector<unsigned> size,
		const std::shared_ptr<const mark::resource::image>& image);
	base_turret(mark::resource::manager&, const YAML::Node& node);
	void target(mark::vector<double> pos) override;
	void shoot(mark::vector<double> pos, bool release) override;
	void queue(mark::vector<double> pos, bool release) override;
	auto passive() const noexcept -> bool override;
protected:
	void tick_ai();
	// Returns true if there are objects still to be destroyed before
	// switching to manual control
	bool queued();

	std::shared_ptr<mark::vector<double>> m_target;
	bool m_shoot = false;
	void serialize_base(YAML::Emitter&) const;
private:
	std::deque<
		std::pair<
			std::weak_ptr<mark::unit::base>,
			vector<double>>> m_queue;
};
} }