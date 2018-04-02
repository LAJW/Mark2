﻿#pragma once
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
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

private:
	void tick(tick_context& context) override;
	void bind(mark::property_manager&) override{};

	std::shared_ptr<const resource::image> m_image;
};
} // namespace module
} // namespace mark
