#pragma once
#include "base.h"
#include <cooldown.h>
#include <lfo.h>
#include <model_animated.h>
#include <resource_image.h>
#include <stdafx.h>
#include <targeting_system.h>

namespace mark {
namespace module {
class cannon final : public module::base {
public:
	static constexpr const char* type_name = "module_cannon";

	cannon(resource::manager&, const YAML::Node&);
	auto describe() const -> std::string override;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

private:
	void tick(tick_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	void bind(mark::property_manager&) override;

	model::animated m_model;
	std::shared_ptr<const resource::image> m_im_ray;
	targeting_system m_targeting_system;
	float m_rotation = 0.f;
	float m_angular_velocity = 90.f;
	lfo m_randomiser;
};
} // namespace module
} // namespace mark