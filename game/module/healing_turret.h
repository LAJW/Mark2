#pragma once
#include "base.h"
#include <cooldown.h>
#include <lfo.h>
#include <model_animated.h>
#include <resource_image.h>
#include <stdafx.h>

namespace mark {
namespace module {
class healing_turret final : public module::base
{
public:
	static constexpr const char* type_name = "module_healing_turret";

	healing_turret(resource::manager&, const YAML::Node&);
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

private:
	void update(update_context& context) override;
	void render(update_context& context) const;
	auto target() const -> const module::base*;
	auto target() -> module::base*;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);

	model::animated m_model;
	std::shared_ptr<const resource::image> m_im_ray;
	std::optional<vector<int>> m_target;
};
} // namespace module
} // namespace mark
