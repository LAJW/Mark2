#pragma once
#include <item/base.h>
#include <stdafx.h>

namespace mark {
namespace item {
class chaos_orb final : public item::base
{
public:
	static constexpr const char* type_name = "item_chaos_orb";

	chaos_orb(resource::manager& rm, const YAML::Node& node);
	auto use_on(
		mark::random& random,
		const std::unordered_map<std::string, YAML::Node>& blueprints,
		module::base& item) -> use_on_result override;
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto size() const -> vu32 override;
	auto thumbnail() const -> resource::image_ptr override;

private:
	auto type_equals(const base &other) const -> bool override;
	auto max_stack_size() const -> size_t override;

	resource::image_ptr m_thumbnail;
};
} // namespace item
} // namespace mark
