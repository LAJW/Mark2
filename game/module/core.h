#pragma once
#include <stdafx.h>
#include "base.h"
#include <resource_image.h>

namespace mark {
namespace module {
class core final : public base {
public:
	static constexpr const char* type_name = "module_core";

	core(resource::manager&, const YAML::Node&);
	core(resource::manager& resource_manager);
	auto detachable() const -> bool;
	auto describe() const -> std::string override;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
private:
	void tick(tick_context& context) override;

	std::shared_ptr<const resource::image> m_image;
};
}
}
