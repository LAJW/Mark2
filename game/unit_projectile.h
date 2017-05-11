#pragma once
#include <unordered_set>
#include "unit_base.h"

namespace sf {
	class Texture;
}

namespace mark {
	class world;
	namespace resource {
		using image = sf::Texture;
	}
	namespace unit {
		class modular;
		class projectile final : public mark::unit::base {
		public:
			// projectile constructor attributes
			struct info {
				mark::world* world = nullptr;
				// for mouse-guided missiles
				std::weak_ptr<const mark::unit::modular> guide;
				mark::vector<double> pos;
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
			projectile(const projectile::info&);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			auto invincible() const -> bool override;
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
		private:
			projectile(const projectile::info&, bool);
			std::shared_ptr<const mark::resource::image> m_image;
			std::shared_ptr<const mark::resource::image> m_im_tail;
			std::unordered_set<mark::idamageable*> m_damaged;
			std::weak_ptr<const mark::unit::modular> m_guide;
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