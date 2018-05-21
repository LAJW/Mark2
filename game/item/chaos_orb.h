#pragma once
#include <interface/item.h>
#include <stdafx.h>

namespace mark {
namespace item {
class chaos_orb final : public interface::item
{
public:
	static constexpr const char* type_name = "item_chaos_orb";

	chaos_orb(resource::manager& rm, const YAML::Node& node);
	auto use_on(
		resource::manager& rm,
		const std::unordered_map<std::string, YAML::Node>& blueprints,
		module::base& item) -> use_on_result override;
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto size() const -> vu32 override;
	auto thumbnail() const -> resource::image_ptr override;
	void stack(interface::item_ptr& item) override;
	auto can_stack(const interface::item& item) const -> bool override;

private:
	resource::image_ptr m_thumbnail;
	size_t m_quantity = 1;
};
} // namespace item
} // namespace mark
