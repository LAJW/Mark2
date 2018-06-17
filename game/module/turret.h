#pragma once
#include "base.h"
#include <adsr.h>
#include <curve.h>
#include <deque>
#include <stdafx.h>
#include <unit/projectile.h>

namespace mark {
namespace module {
class turret final
	: public module::base
	, private projectile_config
{
public:
	static constexpr const char* type_name = "module_turret";

	turret(resource::manager&, const YAML::Node&);
	~turret();
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	void command(const command::any&) override;

private:
	void update(update_context& context) override;
	void render(update_context& context) const;
	// Calculate rotation for the next frame
	auto rotation(double dt) const noexcept -> float;
	auto cooldown(double dt) const noexcept -> float;
	auto can_shoot() const noexcept -> bool;
	auto
	make_projectile(update_context& context, mark::world&, size_t index) const
		-> shared_ptr<unit::projectile>;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	// Get targeting system
	// Returns parent targeting system if turret is stationary
	// Returns this module's targeting system if turret is not stationary
	auto targeting_system() noexcept -> mark::targeting_system&;
	auto targeting_system() const noexcept -> const mark::targeting_system&;

	unique_ptr<mark::targeting_system> m_targeting_system;
	resource::image_ptr m_image;
	resource::image_ptr m_im_orb;
	size_t m_image_variant;
	adsr m_adsr;
	shared_ptr<vd> m_shared_target = std::make_shared<vd>();

	float m_cur_cooldown = 0;
	float m_rate_of_fire = 1.f;
	// Is this turret chargeable
	bool m_is_chargeable = false;
	// Is this turret currently charging
	bool m_is_charging = false;
	curve::ptr m_rate_of_fire_curve = curve::flat;
	float m_rotation = 0.f;
	float m_angular_velocity = 360.f;
	unsigned m_projectile_count = 0;
	float m_burst_delay = 0.f;
	bool m_guided = false;
	float m_cone = 0.f;
	curve::ptr m_cone_curve = curve::flat;
	float m_heat_per_shot = 5.f;

	// Does the projectile wobble when flying
	bool m_unstable = false;
};
} // namespace module
} // namespace mark
