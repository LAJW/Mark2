#pragma once
#include <interface/item.h>

namespace mark {
namespace item {
class base : public interface::item
{
public:
	auto quantity() const -> size_t override final;
	void stack(interface::item_ptr& item) override final;
	auto can_stack(const interface::item& item) const -> bool override final;
	void serialize(YAML::Emitter& out) const override;

protected:
	base(const YAML::Node& node);

private:
	[[nodiscard]] virtual auto type_equals(const base& other) const -> bool = 0;
	[[nodiscard]] virtual auto max_stack_size() const -> size_t = 0;

protected:
	size_t m_quantity = 1;
};
} // namespace item
} // namespace mark
