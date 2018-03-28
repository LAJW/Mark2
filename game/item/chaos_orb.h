#pragma once
#include <stdafx.h>
#include <interface/item.h>

namespace mark {
namespace item {
class chaos_orb final : public interface::item {
public:
	static constexpr const char* type_name = "item_chaos_orb";

	chaos_orb(resource::manager& rm, const YAML::Node& node);
	auto use_on(
		resource::manager& rm,
		const std::unordered_map<std::string, YAML::Node>& blueprints,
		module::base& item) -> use_on_result;
	auto describe() const -> std::string override;
	void serialise(YAML::Emitter&) const override;
	auto size() const -> vector<unsigned> override;
	auto thumbnail() const -> std::shared_ptr<const resource::image> override;
	void stack(std::unique_ptr<interface::item>& item) override;
	auto can_stack(const interface::item& item) const -> bool override;
private:
	std::shared_ptr<const resource::image> m_thumbnail;
	size_t m_quantity = 1;
};
}
}