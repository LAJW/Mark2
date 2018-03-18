#pragma once
#include "base.h"
#include <adsr.h>
#include <curve.h>
#include <deque>
#include <stdafx.h>
#include <targeting_system.h>

namespace mark {
namespace module {
class turret final : public module::base {
public:
	static constexpr const char* type_name = "module_turret";

	turret(resource::manager&, const YAML::Node&);
	auto describe() const -> std::string override;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	void command(const command::any&) override;

private:
	void tick(tick_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	void bind(mark::property_manager&) override;

	targeting_system m_targeting_system;
	std::shared_ptr<const resource::image> m_image;
	std::shared_ptr<const resource::image> m_im_orb;
	size_t m_image_variant;
	adsr m_adsr;
	std::shared_ptr<vector<double>> m_shared_target =
		std::make_shared<vector<double>>();

	float m_cur_cooldown = 0;
	float m_rate_of_fire = 1.f;
	bool m_is_chargeable = false; // Is this turret chargeable
	bool m_is_charging = false;   // Is this turret currently charging
	curve::ptr m_rate_of_fire_curve = curve::flat;
	float m_rotation = 0.f;
	float m_angular_velocity = 360.f;
	unsigned m_projectile_count;
	float m_burst_delay = 0.f;
	bool m_guided = false;
	float m_cone = 0.f;
	curve::ptr m_cone_curve = curve::flat;
	float m_heat_per_shot = 5.f;
	float m_critical_chance = 0.1f;
	float m_critical_multiplier = 1.5f;

	float m_physical = 10.f;
	float m_energy = 10.f;
	float m_heat = 1.f;
	float m_projectile_angular_velocity = 0.f;
	float m_velocity = 1000.f;
	float m_acceleration = 100.f;
	float m_aoe_radius = 0.f;
	float m_seek_radius = 500.f;
	float m_range = 2000.f;
	size_t m_piercing;
};
} // namespace module
} // namespace mark
