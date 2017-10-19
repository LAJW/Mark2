#pragma once
#include "stdafx.h"
#include "module_base.h"

namespace mark {
namespace resource {
class manager;
class image;
}
namespace module {
class flamethrower : public module::base{
public:
	static constexpr const char* type_name = "module_flamethrower";

	flamethrower(resource::manager&, const YAML::Node&);
	flamethrower(resource::manager& manager);
	void tick(tick_context& context) override;
	void shoot(vector<double> pos, bool release) override;
	auto describe() const -> std::string override;
	auto harvest_energy() -> float override;
	auto energy_ratio() const -> float override;
	void serialize(YAML::Emitter&) const override;
	auto reserved() const noexcept -> reserved_type override;
	auto passive() const noexcept -> bool override;
private:
	std::shared_ptr<const resource::image> m_image_base;
	bool m_shoot = false;
};
}
}
