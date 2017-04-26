#pragma once
#include "lfo.h"
#include "module_base.h"
#include "model_shield.h"

namespace mark {

	namespace resource {
		class manager;
		class sprite;
	};

	namespace module {
		class shield_generator : public base {
		public:
			shield_generator(mark::resource::manager& resource_manager);
			void tick(mark::tick_context& context) override;
			inline auto dead() const -> bool override { return false; }
			auto collides(mark::vector<double> pos, float radius) const -> bool;
			void damage(unsigned amount, mark::vector<double> pos) override;
			auto describe() const->std::string;
		private:
			std::shared_ptr<const mark::resource::image> m_im_generator;
			mark::model::shield m_model_shield;
		};
	}
}
