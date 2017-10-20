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
		class gate final : public unit::base {
		public:
			static constexpr const char* type_name = "gate";

			gate(mark::world& world, const YAML::Node&);
			gate(mark::world& world, vector<double> pos);
			void tick(tick_context& context) override;
			auto dead() const -> bool override { return false; };
			void activate(const std::shared_ptr<unit::base>& by) override;
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const resource::image> m_image;
		};
	}
}
