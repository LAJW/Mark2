#pragma once
#include "damageable.h"

namespace mark {
namespace unit {
class mobile : public damageable
{
public:
	void serialise(YAML::Emitter& out) const override;
	void stop();

protected:
	void command(const command::any&) override;
	mobile(mark::world& world, const YAML::Node&);
	struct info : base::info
	{
		std::optional<vector<double>> moveto;
		double velocity = 0.0;
	};
	mobile(const info& info);
	struct tick_movement_info
	{
		// delta time
		double dt;
		// maximum velocity of the ship
		double max_velocity;
		double acceleration;
		// is this an AI object (AI differs in behaviour from non-AI units)
		bool ai;
	};
	void tick_movement(const tick_movement_info&);

private:
	// Implementation of tick_movement
	// - random_can_pathfind - optimization value for AI opponents. There is
	//   a limit as to how many AI ships can look for path in a specific frame.
	//   This option is for randomizing that value. It's so that every frame
	//   a different ship is looking for a path, not only first N ships.
	// Returns a tuple of:
	//  - new position
	//  - new velocity
	//  - new path
	//  - new path age
	auto tick_movement_impl(const tick_movement_info&, bool random_can_pathfind)
		const -> std::
			tuple<vector<double>, double, std::vector<vector<double>>, float>;
	double m_velocity = 0.0;
	vector<double> m_moveto;
	std::vector<vector<double>> m_path_cache;
	float m_path_age = 0.f;
};
} // namespace unit
} // namespace mark
