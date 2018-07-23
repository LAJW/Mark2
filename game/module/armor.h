#pragma once
#include "base.h"
#include <resource_image.h>
#include <stdafx.h>

namespace mark {
namespace module {
class armor final : public base
{
public:
	static constexpr const char* type_name = "module_armor";

	armor(resource::manager&, random& random, const YAML::Node&);
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	auto local_modifiers() const -> module::modifiers override;

private:
	void update(update_context& context) override;

	resource::image_ptr m_image;
};
} // namespace module
} // namespace mark
