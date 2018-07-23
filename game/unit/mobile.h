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
		optional<random&> random;
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
	struct update_movement_impl_result final {
		vd pos;
		double velocity;
		std::vector<vd> path;
		float path_age;
	};
	auto update_movement_impl(const update_movement_info&) const
		-> update_movement_impl_result;

	auto avoid_present_neighbor_collisions(double step_len) const
		-> std::optional<vd>;
	auto avoid_future_neighbor_collisions(vd step) const -> vd;
	auto avoid_bumping_into_terrain(vd step) const
		-> std::optional<vd>;

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
