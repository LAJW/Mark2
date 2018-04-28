#pragma once
#include <stdafx.h>
#include <unit/base.h>
#include <lfo.h>

namespace mark {
namespace unit {
class projectile final : public unit::base
{
public:
	static constexpr const char* type_name = "unit_projectile";
	// projectile constructor attributes
	struct info : base::info
	{
		// for mouse-guided missiles
		std::shared_ptr<const vd> guide;
		float rotation = NAN;
		float phase = 0.f;
		float lfo = 0.f;
		double velocity = NAN;
		float physical = 10.f;
		double seek_radius = 0.f;
		double aoe_radius = 0.f;
		float critical_chance = 0.1f;
		float critical_multiplier = 0.1f;
		// Can missile go through multiple targets
		// 0 - infinite targets
		// 1, 2, 3 - 1, 2, 3 targets.
		size_t piercing = 1;
		double knockback = 0.;
	};
	projectile(mark::world&, const YAML::Node&);
	projectile(const projectile::info&);
	auto dead() const -> bool override;
	void serialize(YAML::Emitter&) const override;
	void resolve_ref(
		const YAML::Node&,
		const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>& units)
		override;

private:
	projectile(const projectile::info&, bool);
	void update(update_context& context) override;

	resource::image_ptr m_image;
	resource::image_ptr m_im_tail;
	resource::image_ptr m_im_explosion;
	std::unordered_set<gsl::not_null<interface::damageable*>> m_damaged;
	std::unordered_set<gsl::not_null<interface::damageable*>> m_knocked;
	std::shared_ptr<const vd> m_guide;
	float m_physical;
	float m_rotation;
	lfo m_rotation_lfo;
	double m_velocity;
	double m_seek_radius;
	double m_aoe_radius;
	float m_critical_chance;
	float m_critical_multiplier;
	size_t m_piercing;
	double m_knockback;
	bool m_dead = false;
};
} // namespace unit
} // namespace mark
