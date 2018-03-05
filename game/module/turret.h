#pragma once
#include <deque>
#include "stdafx.h"
#include "adsr.h"
#include "curve.h"
#include "module_base.h"
#include "targeting_system.h"

namespace mark {
namespace module {
class turret final : public module::base {
public:
	static constexpr const char* type_name = "module_turret";

	struct info {
		// base
		resource::manager* resource_manager;
		// turret max health
		float max_health = 100.f;
		// turrent current health
		float cur_health = 100.f;
		// CPU used
		float cpu = 10.f;
		// Module's weight
		float weight = 100.f;

		// turret info

		// number of shots per second
		float rate_of_fire = 1.f;
		// heat to rate of fire unary function
		curve::ptr rate_of_fire_curve = curve::flat;
		// turret turn speed
		float angular_velocity = 360.f;
		// number of projectiles shot at once
		unsigned projectile_count = 2;
		// delay between launching pellets in seconds
		float burst_delay = 0.f;
		// Shoots mouse-guided projectiles
		bool guided = false;
		// cone of fire
		float cone = 0.f;
		// heat-based cone of fire curve
		curve::ptr cone_curve = curve::flat;
		// heat generated per shot
		float heat_per_shot = 10.f;
		// current heat
		float cur_heat = 0.f;
		// turret dimensions on the grid
		vector<uint8_t> size = { 2, 2 };
		// turret current rotation (for deserializer)
		float rotation = 0.f;
		bool is_chargeable = false;

		// projectile info
		float physical = 10.f;
		float energy = 10.f;
		float heat = 1.f;
		float projectile_angular_velocity = 0.f;
		float velocity = 1000.f;
		float acceleration = 100.f;
		float aoe_radius = 0.f;
		float seek_radius = 500.f;
		// maximum projectile travel distance
		float range = 2000.f;
		// critical strike chance [0, 1]
		float critical_chance = 0.1f;
		// damage multiplier for critical
		float critical_multiplier = 1.5f;
		size_t piercing = 1;
	};
	turret(resource::manager&, const YAML::Node&);
	turret(module::turret::info&);
	auto describe() const -> std::string;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	void command(const command::any&) override;
private:
	void tick(tick_context& context) override;

	targeting_system m_targeting_system;
	std::shared_ptr<const resource::image> m_image;
	std::shared_ptr<const resource::image> m_im_orb;
	size_t     m_image_variant;
	adsr       m_adsr;
	std::shared_ptr<vector<double>> m_shared_target
		= std::make_shared<vector<double>>();

	float      m_cur_cooldown = 0;
	float      m_rate_of_fire = 1.f;
	const bool m_is_chargeable = false; // Is this turret chargeable
	bool       m_is_charging = false; // Is this turret currently charging
	curve::ptr m_rate_of_fire_curve = curve::flat;
	float      m_rotation = 0.f;
	float      m_angular_velocity = 360.f;
	unsigned   m_projectile_count;
	float      m_burst_delay = 0.f;
	bool       m_guided = false;
	float      m_cone = 0.f;
	curve::ptr m_cone_curve = curve::flat;
	float      m_heat_per_shot = 5.f;
	float      m_critical_chance = 0.1f;
	float      m_critical_multiplier = 1.5f;

	float      m_physical = 10.f;
	float      m_energy = 10.f;
	float      m_heat = 1.f;
	float      m_projectile_angular_velocity = 0.f;
	float      m_velocity = 1000.f;
	float      m_acceleration = 100.f;
	float      m_aoe_radius = 0.f;
	float      m_seek_radius = 500.f;
	float      m_range = 2000.f;
	size_t     m_piercing;
};
}
}