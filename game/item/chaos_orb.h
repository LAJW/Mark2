#pragma once
#include <stdafx.h>
#include <interface/item.h>

namespace mark {
namespace item {
class chaos_orb : public item {
	static constexpr const char* type_name = "item_chaos_orb";

	chaos_orb(const YAML::Node& node);
	auto mark::item::chaos_orb::use_on(
		resource::manager& rm,
		const std::shared_ptr<std::string, YAML::Node>& blueprints,
		module::base& item)->std::error_code override;
	auto describe() const -> std::string override;
	void serialise(YAML::Emitter&) const override;
};
}
}