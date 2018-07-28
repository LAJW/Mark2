#include "base.h"

mark::item::base::base(const YAML::Node& node)
	: m_quantity(node["quantity"].as<size_t>(1))
{}

void mark::item::base::stack(interface::item_ptr& item)
{
	let constexpr max_stack_quantity = 20;
	if (let other = dynamic_cast<base*>(item.get())) {
		let total = m_quantity + other->quantity();
		if (total > max_stack_quantity) {
			other->m_quantity = total - max_stack_quantity;
			m_quantity = max_stack_quantity;
		} else {
			item.reset();
			m_quantity = total;
		}
	}
}

void mark::item::base::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << Key << "quantity" << Value << m_quantity;
}

auto mark::item::base::can_stack(const interface::item& other) const -> bool
{
	if (let item = dynamic_cast<const base*>(&other)) {
		if (this->type_equals(*item)) {
			return m_quantity < 20;
		}
	}
	return false;
}

auto mark::item::base::quantity() const -> size_t { return m_quantity; }
