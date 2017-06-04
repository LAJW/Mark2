#pragma once
#include "unit_base.h"

namespace mark {
	namespace resource {
		class image;
	}
	class world;
	namespace module {
		class base;
	}
	namespace unit {
		class modular;
		class gate final : public mark::unit::base {
		public:
			static constexpr const char* type_name = "gate";

			gate(mark::world& world, const YAML::Node&);
			gate(mark::world& world, mark::vector<double> pos);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override { return false; };
			auto invincible() const -> bool override { return true; };
			void activate(const std::shared_ptr<mark::unit::base>& by) override;
			void serialize(YAML::Emitter&) const override;
			auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}
