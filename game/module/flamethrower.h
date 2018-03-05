#pragma once
#include <stdafx.h>
#include "base.h"

namespace mark {
namespace module {
class flamethrower final : public module::base {
public:
	static constexpr const char* type_name = "module_flamethrower";

	flamethrower(resource::manager&, const YAML::Node&);
	flamethrower(resource::manager& manager);
	void command(const command::any& any) override;
	auto describe() const -> std::string override;
	void serialise(YAML::Emitter&) const override;
	auto reserved() const noexcept -> reserved_type override;
	auto passive() const noexcept -> bool override;
private:
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image_base;
	bool m_shoot = false;
};
}
}
