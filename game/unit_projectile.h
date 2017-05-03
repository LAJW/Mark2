#pragma once
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
		class projectile final : public mark::unit::base {
		public:
			// projectile constructor attributes
			struct attributes {
				mark::world* world = nullptr;
				mark::vector<double> pos;
				float rotation = NAN;
				float velocity = NAN;
				float seek_radius = 0.f;
				// Can missile go through multiple targets
				// 0 - infinite targets
				// 1, 2, 3 - 1, 2, 3 targets.
				size_t piercing = 1;
				size_t team = 0;
			};
			projectile(const attributes& essence);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			auto invincible() const -> bool override;
			auto collides(mark::vector<double> pos, float radius) const -> bool override;
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
		private:
			projectile(const attributes& essence, bool);
			std::shared_ptr<const mark::resource::image> m_image;
			std::shared_ptr<const mark::resource::image> m_im_tail;
			float m_rotation;
			float m_velocity;
			float m_seek_radius;
			size_t piercing;
			bool m_dead = false;
		};
	}
}