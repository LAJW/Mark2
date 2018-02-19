#pragma once
#include "stdafx.h"
#include "module_base.h"

namespace mark {
namespace module {
class warp_drive final : public module::base {
public:
	static constexpr const char* type_name = "module_warp_drive";

	warp_drive(resource::manager&, const YAML::Node&);
	warp_drive(resource::manager& manager);
	auto describe() const->std::string override;
	void command(const command::any&) override;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
private:
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image_base;
};
}
}
