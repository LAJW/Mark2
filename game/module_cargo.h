#pragma once
#include "module_base.h"
#include "resource_image.h"
#include "lfo.h"

namespace mark {
	namespace resource {
		class manager;
	}
	namespace module {
		class cargo : public mark::module::base {
		public:
			cargo(mark::resource::manager& manager);
			virtual void tick(mark::tick_context& context) override;
			inline auto dead() const -> bool override { return false; }
			auto modules() -> std::vector<std::unique_ptr<mark::module::base>>&;
			void drop(mark::vector<int> pos, std::unique_ptr<mark::module::base> module);
			auto pick(mark::vector<int> pos) -> std::unique_ptr<mark::module::base>;
		private:
			std::shared_ptr<const mark::resource::image> m_im_body;
			std::shared_ptr<const mark::resource::image> m_im_light;
			mark::lfo m_lfo;
			std::vector<std::unique_ptr<mark::module::base>> m_modules;
		};
	}
}