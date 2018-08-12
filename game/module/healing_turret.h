#pragma once
#include "base.h"
#include <cooldown.h>
#include <lfo.h>
#include <model_animated.h>
#include <resource/image.h>
#include <stdafx.h>
#include <add_const_if.h>

namespace mark {
namespace module {
class healing_turret final : public module::base
{
public:
	static constexpr const char* type_name = "module_healing_turret";

	healing_turret(resource::manager&, random& random, const YAML::Node&);
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

private:
	void update(update_context& context) override;
	void render(update_context& context) const;
	template <typename U, typename T = add_const_if_t<base, std::is_const_v<U>>>
	[[nodiscard]] optional<T&> static target(U& self);
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);

	model::animated m_model;
	resource::image_ptr m_im_ray;
	std::optional<vi32> m_target;
};
} // namespace module
} // namespace mark
