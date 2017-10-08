#pragma once
#include "stdafx.h"
#include "adsr.h"
#include "cooldown.h"
#include "unit_damageable.h"
#include "model_animated.h"
#include "model_shield.h"

namespace mark {
	class world;
	namespace unit {
		class minion final :
			public mark::unit::damageable {
		public:
			static constexpr const char* type_name = "unit_minion";

			minion(mark::world& world, mark::vector<double> pos);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			auto damage(const interface::damageable::info&) -> bool override;
			auto collide(const mark::segment_t&) ->
				std::pair<interface::damageable*, mark::vector<double>> override;
			auto collide(mark::vector<double> center, float radius) ->
				std::vector<std::reference_wrapper<interface::damageable>> override;
			void on_death(mark::tick_context& context) override;
		private:
			mark::model::animated m_model;
			mark::model::shield m_model_shield;
			std::shared_ptr<const mark::resource::image> m_image_explosion;
			mark::cooldown m_gun_cooldown;
			std::vector<mark::vector<double>> m_path_cache; // path cache
			float m_path_age = 0.f;
			float m_rotation = 0.f;
			float m_health = 100.f;
			bool m_dead = false;
		};
	}
}