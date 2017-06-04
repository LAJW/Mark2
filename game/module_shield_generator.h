#pragma once
#include "stdafx.h"
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
			static constexpr const char* type_name = "shield_generator";

			shield_generator(mark::resource::manager&, const YAML::Node&);
			shield_generator(mark::resource::manager& resource_manager);
			void tick(mark::tick_context& context) override;
			bool damage(const mark::idamageable::info&) override;
			auto describe() const->std::string;
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
			auto shield() const noexcept -> float;
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const mark::resource::image> m_im_generator;
			mark::model::shield m_model_shield;
			float m_cur_shield = 1000.f;
			float m_max_shield = 1000.f;
		};
	}
}
