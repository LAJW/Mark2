#pragma once
#include "damageable.h"

namespace mark {
namespace unit {
class mobile : public damageable
{
public:
	void serialize(YAML::Emitter& out) const override;
	void stop();

protected:
	void command(const command::any&) override;
	mobile(mark::world& world, const YAML::Node&);
	struct info : base::info
	{
		std::optional<vd> moveto;
		double velocity = 0.0;
	};
	mobile(const info& info);
	struct update_movement_info
	{
		// delta time
		double dt;
		// maximum velocity of the ship
		double max_velocity;
		double acceleration;
		// is this an AI object (AI differs in behaviour from non-AI units)
		bool ai;
	};
	void update_movement(const update_movement_info&);

private:
	// Implementation of update_movement
	// - random_can_pathfind - optimization value for AI opponents. There is
	//   a limit as to how many AI ships can look for path in a specific frame.
	//   This option is for randomizing that value. It's so that every frame
	//   a different ship is looking for a path, not only first N ships.
	// Returns a tuple of:
	//  - new position
	//  - new velocity
	//  - new path
	//  - new path age
	auto update_movement_impl(
		const update_movement_info&,
		bool random_can_pathfind) const
		-> std::tuple<vd, double, std::vector<vd>, float>;

	auto can_calculate_path(bool random_can_pathfind) const -> bool;
	auto calculate_path(bool random_can_pathfind, double dt) const
		-> std::pair<std::vector<vd>, float>;

	double m_velocity = 0.0;
	vd m_moveto;
	std::vector<vd> m_path_cache;
	float m_path_age = 0.f;
	vd m_prev_step;
};
} // namespace unit
} // namespace mark
