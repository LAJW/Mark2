#pragma once
#include "adsr.h"
#include "cooldown.h"
#include "unit_base.h"
#include "model_animated.h"
#include "model_shield.h"

namespace mark {
	class world;
	namespace unit {
		class minion final : public mark::unit::base {
		public:
			minion(mark::world& world, mark::vector<double> pos);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			auto damage(const mark::idamageable::attributes&) -> bool override;
			auto invincible() const -> bool override;
			auto collides(mark::vector<double> pos, float radius) const -> bool override;
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
		private:
			mark::model::animated m_model;
			mark::model::shield m_model_shield;
			mark::vector<float> m_direction = { 1, 0 };
			float m_health = 100.f;
			bool m_dead = false;
			mark::cooldown m_gun_cooldown;
			std::shared_ptr<const mark::resource::image> m_image_explosion;
		};
	}
}