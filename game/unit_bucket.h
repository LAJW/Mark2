#pragma once
#include "adsr.h"
#include "cooldown.h"
#include "unit_base.h"
#include "model_animated.h"
#include "model_shield.h"

namespace mark {
	class world;
	namespace module {
		class base;
	}
	namespace unit {
		class bucket final : public mark::unit::base {
		public:
			bucket(mark::world& world, mark::vector<double> pos, std::unique_ptr<mark::module::base>);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			auto damage(const mark::idamageable::attributes&) -> bool override;
			auto invincible() const -> bool override;
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
		private:
			std::unique_ptr<mark::module::base> m_module;
		};
	}
}