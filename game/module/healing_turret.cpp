#include <stdafx.h>

#include "healing_turret.h"
#include "impl/healing_turret.h"
#include <algorithm.h>
#include <resource/manager.h>
#include <sprite.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>

namespace mark {

template <typename U, typename T>
optional<T&> module::healing_turret::target(U& self)
{
	if (self.m_stunned || !self.m_target) {
		return {};
	}
	let target = self.parent().module_at(*self.m_target);
	if (!target || !target->needs_healing()) {
		return {};
	}
	return target;
}
} // namespace mark

void mark::module::healing_turret::update(update_context& context)
{
	this->module::base::update(context);
	m_model.update(context.dt);
	if (!target(*this)) {
		if (let target = most_damaged_neighbor_in_range(*this, 100.0)) {
			m_target = target->grid_pos();
		}
	}
	if (let target = this->target(*this)) {
		target->heal(static_cast<float>(10. * context.dt));
	}
	this->render(context);
}

void mark::module::healing_turret::render(update_context& context) const
{
	let pos = this->pos();
	let model_size = std::max(this->size().x, this->size().y) * module::size;
	context.sprites[2].push_back(m_model.render(
		pos, model_size, parent().rotation(), this->heat_color()));
	let target = this->target(*this);
	if (!target) {
		return;
	}
	let collision = target->pos();
	let dir = normalize(collision - pos);
	let rotation = atan(-dir);
	context.render([&] {
		update_context::spray_info _;
		_.image = m_im_ray;
		_.pos = collision;
		_.velocity(25.f, 50.f);
		_.lifespan(1.f);
		_.diameter(8.f);
		_.count = 4;
		_.direction = rotation;
		_.cone = 180.f;
		_.color = sf::Color::Green;
		_.layer = 3;
		return _;
	}());
	let len = static_cast<int>(length(collision - pos) / double(module::size));
	Expects(len >= 1); // self should never be targeted
	transform(range(1, len), back_inserter(context.sprites[3]), [&](let i) {
		let di = static_cast<double>(i);
		let cur_len = module::size * di - module::size * 0.5;
		sprite _;
		_.image = m_im_ray;
		_.pos = collision - dir * cur_len;
		_.size = module::size;
		_.rotation = rotation;
		_.color = sf::Color::Green;
		return _;
	});
}

std::string mark::module::healing_turret::describe() const
{
	return "Healing Turret\nHPS: 100\n" + base::describe();
}

// Serialize / Deserialize

template <typename prop_man, typename T>
void mark::module::healing_turret::bind(prop_man& property_manager, T& instance)
{
	(void)property_manager;
	(void)instance;
}

mark::module::healing_turret::healing_turret(
	resource::manager& rm,
	random& random,
	const YAML::Node& node)
	: module::base(rm, random, node)
	, m_model(rm.image("cannon.png"))
	, m_im_ray(rm.image("ray.png"))
{}

void mark::module::healing_turret::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialize(out);
	out << EndMap;
}

auto mark::module::healing_turret::passive() const noexcept -> bool
{
	return true;
}
