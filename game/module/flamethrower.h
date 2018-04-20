#pragma once
#include "base.h"
#include <stdafx.h>

namespace mark {
namespace module {
class flamethrower final : public module::base
{
public:
	static constexpr const char* type_name = "module_flamethrower";

	flamethrower(resource::manager&, const YAML::Node&);
	void command(const command::any& any) override;
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto reserved() const noexcept -> reserved_kind override;
	auto passive() const noexcept -> bool override;

private:
	void update(update_context& context) override;
	void render(update_context& context) const;
	// Check if can shoot and target is in range
	auto can_shoot() const -> bool;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);

	std::shared_ptr<const resource::image> m_image_base;
	std::shared_ptr<const resource::image> m_image_fire;
	float m_cone_of_fire = 30.f;
	float m_dps = 200.f;
	double m_range = 300.;
};
} // namespace module
} // namespace mark
