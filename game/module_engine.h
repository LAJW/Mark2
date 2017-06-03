#pragma once
#include "module_base.h"

namespace mark {
	namespace resource {
		class manager;
		class image;
	}
	namespace module {
		class engine : public mark::module::base {
		public:
			static constexpr const char* type_name = "module_engine";

			engine(mark::resource::manager&, const YAML::Node&);
			engine(mark::resource::manager& manager);
			void tick(mark::tick_context& context) override;
			auto describe() const->std::string override;
			auto global_modifiers() const->mark::module::modifiers override;
			void shoot(mark::vector<double> pos, bool release) override;
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const mark::resource::image> m_image_base;
			bool m_active = false;
		};
	}
}