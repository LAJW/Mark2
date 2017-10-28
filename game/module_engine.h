#pragma once
#include "stdafx.h"
#include "module_base.h"

namespace mark {
namespace resource {
class manager;
class image;
}
namespace module {
class engine final : public module::base {
public:
	static constexpr const char* type_name = "module_engine";

	engine(resource::manager&, const YAML::Node&);
	engine(resource::manager& manager);
	auto describe() const->std::string override;
	auto global_modifiers() const->module::modifiers override;
	void shoot(vector<double> pos, bool release) override;
	void serialize(YAML::Emitter&) const override;
	auto reserved() const noexcept -> reserved_type override;
	auto passive() const noexcept -> bool override;
private:
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image_base;
	bool m_active = false;
};
}
}