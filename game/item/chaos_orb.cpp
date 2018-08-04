#include "chaos_orb.h"
#include <exception.h>
#include <module/base.h>
#include <resource/manager.h>
#include <stdafx.h>

mark::item::chaos_orb::chaos_orb(
	mark::resource::manager& rm,
	const YAML::Node& node)
	: base(node)
	, m_thumbnail(rm.image("chaos-orb.png"))
{}

auto mark::item::chaos_orb::use_on(
	mark::random& random,
	const std::unordered_map<std::string, YAML::Node>& blueprints,
	module::base& item) -> use_on_result
{
	Expects(this->quantity() != 0);
	let error = item.randomise(blueprints, random);
	if (error == error::code::success) {
		--m_quantity;
	}
	use_on_result result;
	result.error = error;
	result.consumed = this->quantity() == 0;
	return result;
}

auto mark::item::chaos_orb::describe() const -> std::string
{
	std::ostringstream os;
	os << "Chaos Orb" << std::endl;
	os << "Randomises all random properties of an item." << std::endl;
	os << "Quantity: " << this->quantity() << std::endl;
	return os.str();
}

void mark::item::chaos_orb::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialize(out);
	out << EndMap;
}

auto mark::item::chaos_orb::size() const -> vu32 { return { 2, 2 }; }

auto mark::item::chaos_orb::thumbnail() const -> resource::image_ptr
{
	return m_thumbnail;
}

auto mark::item::chaos_orb::type_equals(const base& item) const -> bool
{
	return dynamic_cast<const chaos_orb*>(&item) != nullptr;
}

auto mark::item::chaos_orb::max_stack_size() const -> size_t { return 20; }
