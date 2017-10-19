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

	shield_generator(resource::manager&, const YAML::Node&);
	shield_generator(resource::manager& resource_manager);
	void tick(tick_context& context) override;
	bool damage(const interface::damageable::info&) override;
	auto describe() const->std::string;
	virtual auto collide(const segment_t&) ->
		std::pair<interface::damageable*, vector<double>> override;
	auto shield() const noexcept -> float;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
private:
	std::shared_ptr<const resource::image> m_im_generator;
	model::shield m_model_shield;
	float m_cur_shield = 1000.f;
	float m_max_shield = 1000.f;
};
}
}
