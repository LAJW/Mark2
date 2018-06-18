#pragma once
#include "adsr.h"
#include "command.h"
#include "interface/damageable.h"
#include "lfo.h"
#include "space_bins.h"
#include "stdafx.h"
#include "unit/base.h"

namespace mark {
class camera;
class world final
{
public:
	// Small empty world with square map for testing purposes
	world(resource::manager& resource_manager);
	world(
		world_stack& stack,
		resource::manager& resource_manager,
		bool initial = true);
	world(world_stack& stack, resource::manager&, const YAML::Node&);
	~world();
	auto resource_manager() -> resource::manager&;
	void update(update_context&, vd screen_size);
	auto map() const -> const map&;
	auto camera() const -> vd;

	struct true_predicate
	{
		template <typename T>
		auto operator()(const T&) const
		{
			return true;
		}
	};

	template <typename unit_type = unit::base, typename T = true_predicate>
	auto find(vd pos, const double radius, T pred = true_predicate{}) const
	{
		return mark::find<unit_type>(m_space_bins, pos, radius, pred);
	}

	template <typename unit_type = unit::base, typename T = true_predicate>
	auto find_one(vd pos, const double radius, T pred = true_predicate{}) const
	{
		return mark::find_one<unit_type>(m_space_bins, pos, radius, pred);
	}

	void command(const command::any& command);
	// set target for commmands
	void target(const shared_ptr<unit::base>& target);
	// get target for commands
	auto target() -> shared_ptr<unit::base>;
	auto target() const -> shared_ptr<const unit::base>;
	void attach(not_null<shared_ptr<mark::unit::base>> unit);
	struct damage_info
	{
		interface::damageable::info damage;
		update_context* context = nullptr;
		segment_t segment;
		size_t piercing = 1; // Number of objects to pierce
		double aoe_radius = 0.f;
		// Apply damage in the area regardless of whether it did hit something
		// or not
		bool aerial = false;
	};
	struct damage_result
	{
		std::vector<vd> collisions;
		bool hit_terrain = false;
		float reflected = 0.f;
	};
	auto damage(world::damage_info) -> damage_result;
	// go to the next map
	void next();
	void prev();
	void serialize(YAML::Emitter& out) const;
	auto blueprints() const
		-> const std::unordered_map<std::string, YAML::Node>&;

	const resource::image_ptr image_bar;
	const resource::image_ptr image_font;
	const resource::image_ptr image_stun;

private:
	struct collision_type
	{
		ref<interface::damageable> victim;
		vd pos;
	};
	struct collide_result
	{
		std::deque<collision_type> unit_collisions;
		std::optional<mark::collide_result> terrain_collision;
	};
	// Collide with units and terrain
	// Returns all collisions in a line stopping at the first terrain collision
	auto collide(const segment_t&) -> collide_result;
	auto collide(vd center, double radius)
		-> std::vector<ref<interface::damageable>>;
	void update_spatial_partition();

	resource::manager& m_resource_manager;
	unique_ptr<mark::map> m_map;
	std::vector<not_null<shared_ptr<unit::base>>> m_units;
	space_bins<unit::base> m_space_bins;
	unique_ptr<mark::camera> m_camera;
	std::vector<particle> m_particles;
	world_stack* m_stack = nullptr;
};
} // namespace mark
