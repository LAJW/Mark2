#pragma once
#include "stdafx.h"
#include "lfo.h"
#include "adsr.h"
#include "vector.h"

namespace mark {
	struct tick_context;
	namespace resource {
		class manager;
		class image;
	}
	namespace model {
		class shield {
		public:
			shield(mark::resource::manager& resource_manager, float radius);
			void tick(mark::tick_context& context, mark::vector<double> pos);
			void trigger(mark::vector<double> pos);
		private:
			mark::lfo m_lfo;
			mark::adsr m_adsr;
			const std::shared_ptr<const mark::resource::image> m_image_shield;
			const std::shared_ptr<const mark::resource::image> m_image_reflection;
			const float m_radius;
			mark::vector<double> m_trigger_pos;
		};
	}
}