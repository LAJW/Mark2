#pragma once
#include "stdafx.h"
#include "interface_has_bindings.h"
#include "unit_base.h"
#include "unit_modular.h"

namespace mark {
	namespace resource {
		class image;
	}
	class world;
	namespace module {
		class base;
	}
	namespace unit {
		class landing_pad final:
			public unit::base,
			public interface::has_bindings {
		public:
			static constexpr const char* type_name = "unit_landing_pad";

			landing_pad(world& world, const YAML::Node&);
			landing_pad(world& world, vector<double> pos);
			void tick(tick_context& context) override;
			auto dead() const -> bool override { return false; };
			void dock(unit::modular* ship);
			void activate(
				const std::shared_ptr<unit::base>& by)
				override;
			void command(const mark::command& command) override;
			void serialize(YAML::Emitter&) const override;
			void resolve_ref(
				const YAML::Node&,
				const std::unordered_map<uint64_t,
					std::weak_ptr<unit::base>>& units) override;
			auto bindings() const -> interface::has_bindings::bindings_t override;
			auto ship() -> std::shared_ptr<unit::modular>;
			auto ship() const -> std::shared_ptr<const unit::modular>;
		private:
			std::shared_ptr<const resource::image> m_image;
			std::weak_ptr<unit::modular> m_ship;
			vector<double> m_mousepos;
		};
	}
}