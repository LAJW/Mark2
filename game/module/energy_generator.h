#pragma once
#include <stdafx.h>
#include "base.h"

namespace mark {
namespace module {
class energy_generator final : public module::base {
public:
	static constexpr const char* type_name = "module_energy_generator";

	energy_generator(resource::manager&, const YAML::Node&);
	energy_generator(resource::manager& manager);
	auto describe() const->std::string override;
	auto harvest_energy(double dt) -> float override;
	auto energy_ratio() const -> float override;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
private:
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image_base;
	std::shared_ptr<const resource::image> m_image_bar;
	float m_cur_energy = 0.f;
	const float m_max_energy = 100.f;
	const float m_energy_regen = 30.f;
};
}
}