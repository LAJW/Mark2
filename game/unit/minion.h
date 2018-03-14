#pragma once
#include "mobile.h"
#include <cooldown.h>
#include <model_animated.h>
#include <model_shield.h>
#include <stdafx.h>

namespace mark {
namespace unit {
class minion final : public unit::mobile {
public:
	static constexpr const char* type_name = "unit_minion";

	using unit::mobile::info;
	minion(const mobile::info&);
	auto dead() const -> bool override;
	auto damage(const interface::damageable::info&) -> bool override;
	auto collide(const segment_t&) -> std::optional<std::pair<
		std::reference_wrapper<interface::damageable>,
		vector<double>>> override;
	auto collide(vector<double> center, float radius)
		-> std::vector<std::reference_wrapper<interface::damageable>> override;
	void on_death(tick_context& context) override;
	auto radius() const -> double override;

private:
	void tick(tick_context& context) override;

	model::animated m_model;
	model::shield m_model_shield;
	std::shared_ptr<const resource::image> m_image_explosion;
	cooldown m_gun_cooldown;
	float m_rotation = 0.f;
	float m_health = 100.f;
};
} // namespace unit
} // namespace mark