#pragma once
#include "stdafx.h"
#include "module_base.h"

namespace mark {
namespace module {

class base_turret : public module::base {
public:
	auto passive() const noexcept -> bool override;
	void command(const command::any&) override final;
protected:
	base_turret(
		vector<unsigned> size,
		const std::shared_ptr<const resource::image>& image,
		bool charged);
	base_turret(resource::manager&, const YAML::Node& node);

	void tick(double dt);
	// Can the inheriting turret perform a shot in this frame. Returns true
	// if yes, false otherwise. Mutable, as requesting charge might reset the
	// internal clock.
	auto request_charge() -> bool;
	// Target at which turret is pointing
	auto target() const -> std::optional<vector<double>>;
	void serialise(YAML::Emitter&) const;
	// Level of turret's charge (between 0 and 1)
	auto charge() const -> float;
private:
	void target(vector<double> pos);
	void queue(vector<double> pos, bool release);

	// Queued modules to shoot (units with offset) or a world position for
	// dumb fire
	using target_type = std::pair<bool, vector<double>>;
	using queue_type = std::deque<
		std::pair<
			std::weak_ptr<unit::base>,
			vector<double>>>;
	std::variant<target_type, queue_type> m_target;
	bool m_charged;          // Is turret chargeable
	bool m_charging = false; // Is turret currently charging
	float m_charge = 0.f;    // Charge in %
};
} }