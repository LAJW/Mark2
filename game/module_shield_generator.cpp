#include "stdafx.h"
#include "module_shield_generator.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include <sstream>

mark::module::shield_generator::shield_generator(mark::resource::manager& rm, const YAML::Node& node):
	mark::module::base(rm, node),
	m_im_generator(rm.image("shield-generator.png")),
	m_model_shield(rm, 128.f) { }

mark::module::shield_generator::shield_generator(mark::resource::manager& resource_manager):
	base({ 2, 2 }, resource_manager.image("shield-generator.png")),
	m_im_generator(resource_manager.image("shield-generator.png")),
	m_model_shield(resource_manager, 128.f) {

}

void mark::module::shield_generator::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	const auto pos = this->pos();
	if (m_cur_shield > 0) {
		m_model_shield.tick(context, pos);
	}
	mark::sprite::info info;
	info.image = m_im_generator;
	info.pos = pos;
	info.size = mark::module::size * 2.f;
	info.rotation = parent().rotation();
	info.color = this->heat_color();
	context.sprites[2].emplace_back(info);

	mark::tick_context::bar_info shield_bar;
	shield_bar.image = parent().world().resource_manager().image("bar.png");
	shield_bar.pos = pos + mark::vector<double>(0, -mark::module::size * 2);
	shield_bar.type = mark::tick_context::bar_type::shield;
	shield_bar.percentage = m_cur_shield / m_max_shield;
	context.render(shield_bar);
}

bool mark::module::shield_generator::damage(const mark::idamageable::info& attr) {
	if (attr.damaged->find(this) == attr.damaged->end() && attr.team != parent().team()) {
		if (m_cur_shield > 0.f) {
			attr.damaged->insert(this);
			m_model_shield.trigger(attr.pos);
			m_cur_shield -= attr.physical;
			return true;
		} else {
			attr.damaged->insert(this);
			m_cur_health -= attr.physical;
			return true;
		}
	}
	return false;
}

auto mark::module::shield_generator::describe() const -> std::string {
	std::ostringstream os;
	os << "Shield Generator Module" << std::endl;
	os << "Health: " << static_cast<int>(std::ceil(m_cur_health)) << " of " << static_cast<int>(std::ceil(m_max_health)) << std::endl;
	os << "Shields: " << static_cast<int>(std::ceil(m_cur_shield)) << " of " << static_cast<int>(std::ceil(m_max_shield)) << std::endl;
	return os.str();
}

auto mark::module::shield_generator::collide(const mark::segment_t& ray) ->
	std::pair<mark::idamageable*, mark::vector<double>> {
	if (m_cur_shield > 0.f) {
		const auto shield_size = 64.f;
		const auto intersection = mark::intersect(ray, pos(), shield_size);
		if (!std::isnan(intersection.x)) {
			return { this, intersection };
		} else {
			return { nullptr, { NAN, NAN } };
		}
	} else {
		return mark::module::base::collide(ray);
	}
}

auto mark::module::shield_generator::shield() const noexcept -> float {
	return m_cur_shield;
}

void mark::module::shield_generator::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	this->serialize_base(out);
	out << Key << "cur_shield" << Value << m_cur_shield;
	out << Key << "max_shield" << Value << m_max_shield;
	out << EndMap;
}

auto mark::module::shield_generator::passive() const noexcept -> bool
{ return true; }
