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
			auto collides(mark::vector<double> pos, float radius) const -> bool;
			bool damage(const mark::idamageable::attributes&) override;
			auto describe() const->std::string;
		private:
			std::shared_ptr<const mark::resource::image> m_im_generator;
			mark::model::shield m_model_shield;
			float m_cur_shield = 1000.f;
			float m_max_shield = 1000.f;
		};
	}
}
