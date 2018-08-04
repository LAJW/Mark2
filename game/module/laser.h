#pragma once
#include "base.h"
#include <cooldown.h>
#include <lfo.h>
#include <model_animated.h>
#include <resource/image.h>
#include <stdafx.h>
#include <targeting_system.h>

namespace mark {
namespace module {
class laser final : public module::base
{
public:
	static constexpr const char* type_name = "module_cannon";

	laser(resource::manager&, random& random, const YAML::Node&);
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	void command(const mark::command::any&) override;

private:
	void update(update_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	void render(
		update_context& context,
		std::vector<vd> collisions,
		bool is_firing,
		vd) const;

	model::animated m_model;
	resource::image_ptr m_im_ray;
	targeting_system m_targeting_system;
	float m_rotation = 0.f;
	float m_angular_velocity = 90.f;
	lfo m_randomiser;
};
} // namespace module
} // namespace mark
