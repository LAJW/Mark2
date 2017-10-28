#pragma once
#include "stdafx.h"
#include "module_base.h"
#include "lfo.h"

namespace mark {
namespace resource {
class manager;
class image;
}
namespace module {
class battery final : public module::base {
public:
	static constexpr const char* type_name = "module_battery";

	battery(resource::manager&, const YAML::Node&);
	battery(resource::manager& manager);
	auto describe() const -> std::string override;
	auto harvest_energy() -> float override;
	auto energy_ratio() const -> float override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
private:
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image_base;
	std::shared_ptr<const resource::image> m_image_bar;
	float m_cur_energy = 0.f;
	float m_max_energy = 1000.f;
};
}
}