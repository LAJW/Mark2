﻿#pragma once
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
	void update(update_context&, vector<double> screen_size);
	auto map() const -> const map&;
	auto camera() const -> vector<double>;

	struct true_predicate
	{
		template <typename T>
		auto operator()(const T&) const
		{
			return true;
		}
	};

	template <typename unit_type = unit::base, typename T = true_predicate>
	auto find(
		const vector<double>& pos,
		const double radius,
		T pred = true_predicate{}) const
		-> std::vector<std::shared_ptr<unit_type>>
	{
		return mark::find<unit_type>(m_space_bins, pos, radius, pred);
	}

	template <typename unit_type = unit::base, typename T = true_predicate>
	auto find_one(
		const vector<double>& pos,
		const double radius,
		T pred = true_predicate{}) const -> std::shared_ptr<unit_type>
	{
		return mark::find_one<unit_type>(m_space_bins, pos, radius, pred);
	}

	void command(const command::any& command);
	// set target for commmands
	void target(const std::shared_ptr<unit::base>& target);
	// get target for commands
	auto target() -> std::shared_ptr<unit::base>;
	auto target() const -> std::shared_ptr<const unit::base>;
	void attach(const std::shared_ptr<mark::unit::base>& unit);
	struct damage_info
	{
		interface::damageable::info damage;
		update_context* context = nullptr;
		segment_t segment;
		size_t piercing = 1; // Number of objects to pierce
		float aoe_radius = 0.f;
	};
	// Returns pair containing list of collision points and boolean signaling
	// whether or not terrain was hit
	auto damage(world::damage_info)
		-> std::pair<std::vector<vector<double>>, bool>;
	// go to the next map
	void next();
	void prev();
	void serialise(YAML::Emitter& out) const;
	auto blueprints() const
		-> const std::unordered_map<std::string, YAML::Node>&;

	const std::shared_ptr<const resource::image> image_bar;
	const std::shared_ptr<const resource::image> image_font;
	const std::shared_ptr<const resource::image> image_stun;

private:
	using collision_type = std::
		pair<std::reference_wrapper<interface::damageable>, vector<double>>;
	// Collide with units and terrain
	// Returns all collisions in a line stopping at the first terrain collision
	// Returns monostate if didn't collide with anything
	auto collide(const segment_t&)
		-> std::pair<std::deque<collision_type>, std::optional<vector<double>>>;
	auto collide(vector<double> center, float radius)
		-> std::vector<std::reference_wrapper<interface::damageable>>;
	void update_spatial_partition();

	resource::manager& m_resource_manager;
	std::unique_ptr<mark::map> m_map;
	std::vector<std::shared_ptr<unit::base>> m_units;
	space_bins<unit::base> m_space_bins;
	std::unique_ptr<mark::camera> m_camera;
	std::vector<particle> m_particles;
	world_stack* m_stack = nullptr;
};
} // namespace mark
