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
			static constexpr const char* type_name = "unit_bucket";

			bucket(mark::world& world, const YAML::Node&);
			bucket(mark::world& world, mark::vector<double> pos, std::unique_ptr<mark::module::base>);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			auto damage(const mark::idamageable::info&) -> bool override;
			auto invincible() const -> bool override;
			auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
			auto release() -> std::unique_ptr<mark::module::base>;
			void serialize(YAML::Emitter& out) const override;
		private:
			std::unique_ptr<mark::module::base> m_module;
		};
	}
}