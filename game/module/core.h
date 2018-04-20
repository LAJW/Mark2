#pragma once
#include "base.h"
#include <resource_image.h>
#include <stdafx.h>

namespace mark {
namespace module {
class core final : public base
{
public:
	static constexpr const char* type_name = "module_core";

	core(resource::manager&, const YAML::Node&);
	auto detachable() const -> bool override;
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

private:
	void update(update_context& context) override;

	std::shared_ptr<const resource::image> m_image;
};
} // namespace module
} // namespace mark
