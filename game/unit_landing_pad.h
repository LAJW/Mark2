#pragma once
#include "stdafx.h"
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
		class landing_pad final : public mark::unit::base {
		public:
			static constexpr const char* type_name = "unit_landing_pad";

			landing_pad(mark::world& world, const YAML::Node&);
			landing_pad(mark::world& world, mark::vector<double> pos);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override { return false; };
			auto invincible() const -> bool override { return true; };
			void dock(mark::unit::modular* ship);
			void activate(const std::shared_ptr<mark::unit::base>& by) override;
			void command(const mark::command& command) override;
			auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
			void serialize(YAML::Emitter&) const override;
			void resolve_ref(const YAML::Node&, const std::unordered_map<uint64_t, std::weak_ptr<mark::unit::base>>& units) override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			std::weak_ptr<mark::unit::modular> m_ship;
			std::unique_ptr<mark::module::base> m_grabbed;
			mark::vector<double> m_mousepos;
		};
	}
}