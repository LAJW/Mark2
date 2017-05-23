#pragma once
#include "module_base.h"
#include "resource_image.h"
#include "lfo.h"

namespace mark {
	struct tick_context;
	namespace resource {
		class manager;
	}
	namespace module {
		class cargo : public mark::module::base {
		public:
			cargo(mark::resource::manager& manager);
			virtual void tick(mark::tick_context& context) override;
			auto modules() -> std::vector<std::unique_ptr<mark::module::base>>&;
			void drop(mark::vector<int> pos, std::unique_ptr<mark::module::base> module);
			auto can_drop(mark::vector<int> pos, const std::unique_ptr<mark::module::base>& module) const -> bool;
			auto module(mark::vector<int> pos) -> mark::module::base*;
			auto module(mark::vector<int> pos) const->const mark::module::base*;
			auto pick(mark::vector<int> pos) -> std::unique_ptr<mark::module::base>;
			void render_contents(mark::vector<double> pos, mark::tick_context& context);
			auto interior_size() const -> mark::vector<int>; // size of the contents of the cargo hold in modular units
			auto detachable() const -> bool override;
			auto describe() const -> std::string;
			void on_death(mark::tick_context& context);
			// try to push element to the container
			// return true on success
			// return false if module could not be pushed
			bool push(std::unique_ptr<mark::module::base>& module);
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const mark::resource::image> m_im_body;
			std::shared_ptr<const mark::resource::image> m_im_light;
			mark::lfo m_lfo;
			std::vector<std::unique_ptr<mark::module::base>> m_modules;
		};
	}
}