#pragma once
#include "stdafx.h"
#include "cooldown.h"
#include "model_animated.h"
#include "module_base.h"
#include "module_base_turret.h"
#include "resource_image.h"

namespace mark {
namespace resource {
class manager;
}
namespace module {
class cannon : public module::base_turret {
public:
	static constexpr const char* type_name = "module_cannon";

	cannon(resource::manager&, const YAML::Node&);
	cannon(resource::manager& manager);
	void tick(tick_context& context) override;
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
private:
	model::animated m_model;
	std::shared_ptr<const resource::image> m_im_ray;
	float m_rotation = 0.f;
	float m_angular_velocity = 90.f;
};
}
}