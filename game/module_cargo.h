#pragma once
#include "module_base.h"
#include "resource_image.h"
#include "lfo.h"

namespace mark {
	class tick_context;
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
			auto can_drop(mark::vector<int> pos, const std::unique_ptr<mark::module::base>& module) const -> bool;
			auto pick(mark::vector<int> pos) -> std::unique_ptr<mark::module::base>;
			void render_contents(mark::vector<double> pos, mark::tick_context& context);
			auto interior_size() const -> mark::vector<int>; // size of the contents of the cargo hold in modular units
		private:
			std::shared_ptr<const mark::resource::image> m_im_body;
			std::shared_ptr<const mark::resource::image> m_im_light;
			mark::lfo m_lfo;
			std::vector<std::unique_ptr<mark::module::base>> m_modules;
		};
	}
}