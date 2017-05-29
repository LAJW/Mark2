#pragma once
#include "module_base.h"
#include "resource_image.h"
#include "cooldown.h"
#include "model_animated.h"

namespace mark {
	namespace resource {
		class manager;
	}
	namespace module {
		class cannon : public mark::module::base{
		public:
			cannon(mark::resource::manager&, const YAML::Node&);
			cannon(mark::resource::manager& manager);
			void tick(mark::tick_context& context) override;
			void shoot(mark::vector<double> pos, bool release) override;
			auto describe() const -> std::string override;
			void serialize(YAML::Emitter&) const override;
		private:
			mark::model::animated m_model;
			std::shared_ptr<const mark::resource::image> m_im_ray;
			bool m_shoot = false;
		};
	}
}