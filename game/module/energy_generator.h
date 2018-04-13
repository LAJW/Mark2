#pragma once
#include "base.h"
#include <stdafx.h>

namespace mark {
namespace module {
class energy_generator final : public module::base
{
public:
	static constexpr const char* type_name = "module_energy_generator";

	energy_generator(resource::manager&, const YAML::Node&);
	auto describe() const -> std::string override;
	auto harvest_energy(double dt) -> float override;
	auto energy_ratio() const -> float override;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

private:
	void update(update_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	void bind(mark::property_manager&) override;

	std::shared_ptr<const resource::image> m_image_base;
	std::shared_ptr<const resource::image> m_image_bar;
	float m_cur_energy = 0.f;
	float m_max_energy = 100.f;
	float m_energy_regen = 30.f;
};
} // namespace module
} // namespace mark