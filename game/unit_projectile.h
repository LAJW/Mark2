#pragma once
#include "stdafx.h"
#include "unit_base.h"

namespace mark {
namespace unit {
class projectile final : public unit::base {
public:
	static constexpr const char* type_name = "unit_projectile";
	// projectile constructor attributes
	struct info {
		mark::world* world = nullptr;
		// for mouse-guided missiles
		std::shared_ptr<const vector<double>> guide;
		vector<double> pos;
		float rotation = NAN;
		float velocity = NAN;
		float seek_radius = 0.f;
		float aoe_radius = 0.f;
		float critical_chance = 0.1f;
		float critical_multiplier = 0.1f;
		// Can missile go through multiple targets
		// 0 - infinite targets
		// 1, 2, 3 - 1, 2, 3 targets.
		size_t piercing = 1;
		size_t team = 0;
	};
	projectile(mark::world&, const YAML::Node&);
	projectile(const projectile::info&);
	auto dead() const -> bool override;
	void serialise(YAML::Emitter&) const override;
	void resolve_ref(
		const YAML::Node&,
		const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>& units);
private:
	projectile(const projectile::info&, bool);
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image;
	std::shared_ptr<const resource::image> m_im_tail;
	std::unordered_set<interface::damageable*> m_damaged;
	std::shared_ptr<const vector<double>> m_guide;
	float m_rotation;
	float m_velocity;
	float m_seek_radius;
	float m_aoe_radius;
	float m_critical_chance;
	float m_critical_multiplier;
	size_t m_piercing;
	bool m_dead = false;
};
}
}