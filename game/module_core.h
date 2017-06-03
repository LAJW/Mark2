#pragma once
#include <vector>
#include "module_base.h"
#include "resource_image.h"

namespace mark {
	namespace resource {
		class manager;
		class sprite;
	};

	namespace module {
		class core : public base {
		public:
			static constexpr const char* type_name = "module_core";

			core(mark::resource::manager&, const YAML::Node&);
			core(mark::resource::manager& resource_manager);
			void tick(mark::tick_context& context) override;
			auto detachable() const -> bool;
			auto describe() const -> std::string override;
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}
