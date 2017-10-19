#pragma once
#include "stdafx.h"
#include "module_base.h"

namespace mark {
namespace resource {
class manager;
class image;
}
namespace module {
class energy_generator : public module::base {
public:
	static constexpr const char* type_name = "module_energy_generator";

	energy_generator(resource::manager&, const YAML::Node&);
	energy_generator(resource::manager& manager);
	void tick(tick_context& context) override;
	auto describe() const->std::string override;
	auto harvest_energy() -> float override;
	auto energy_ratio() const -> float override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
private:
	std::shared_ptr<const resource::image> m_image_base;
	std::shared_ptr<const resource::image> m_image_bar;
	float m_cur_energy = 0.f;
	const float m_max_energy = 100.f;
	const float m_energy_regen = 30.f;
};
}
}