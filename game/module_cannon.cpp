#include "stdafx.h"
#include "module_cannon.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include "particle.h"

mark::module::cannon::cannon(mark::resource::manager& resource_manager):
	mark::module::base({ 4, 2 }, resource_manager.image("cannon.png")),
	m_model(resource_manager.image("cannon.png")),
	m_im_ray(resource_manager.image("ray.png")) {

}

void mark::module::cannon::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	m_model.tick(context.dt);
	auto pos = this->pos();
	const auto rotation = parent().rotation();
	const auto model_size = std::max(this->size().x, this->size().y)
		* mark::module::size;
	context.sprites[2].push_back(m_model.render(
		pos,
		model_size,
		rotation,
		this->heat_color()));
	auto& world = parent().world();
	mark::tick_context::bar_info bar;
	if (m_angular_velocity == 0.f) {
		m_rotation = rotation;
	} else {
		// TODO Respect angular velocity here
		m_rotation = mark::turn(m_target - pos, m_rotation, m_angular_velocity, context.dt);
	}
	if (m_shoot) {
		std::unordered_set<mark::idamageable*> damaged;
		for (int i = 1; i < 200; i++) {
			const auto cur_len = mark::module::size * static_cast<double>(i);
			const auto cur_dir = mark::rotate(mark::vector<double>(1, 0), m_rotation);
			const auto prev = pos + cur_dir * (cur_len - mark::module::size);
			const auto cur = pos + cur_dir * (cur_len + 2.0);
			mark::world::damage_info info;
			info.context = &context;
			info.aoe_radius = 0.f;
			info.piercing = 1;
			info.segment = { prev, cur };
			info.damage.damaged = &damaged;
			info.damage.physical = 100.f * static_cast<float>(context.dt);
			info.damage.team = parent().team();
			const auto [ maybe_pos, died ] = world.damage(info);
			if (died) {
				mark::tick_context::spray_info spray;
				spray.image = m_im_ray;
				spray.pos = maybe_pos.value();
				spray.velocity(25.f, 50.f);
				spray.lifespan(1.f);
				spray.diameter(8.f);
				spray.count = 4;
				spray.direction = m_rotation + 180.f;
				spray.cone = 180.f;
				spray.color = sf::Color::Red;
				context.render(spray);
				break;
			}
			{
				mark::sprite::info info;
				info.image = m_im_ray;
				info.pos = cur;
				info.size = mark::module::size;
				info.rotation = m_rotation;
				info.color = sf::Color::Red;
				context.sprites[0].emplace_back(info);
			}
		}
	}
}

void mark::module::cannon::target(mark::vector<double> pos) {
	m_target = pos;
}

void mark::module::cannon::shoot(mark::vector<double> pos, bool release) {
	m_target = pos;
	m_shoot = !release;
}

std::string mark::module::cannon::describe() const {
	return "Laser Cannon\n"
		"DPS: 60\n";
}

// Serialize / Deserialize

mark::module::cannon::cannon(mark::resource::manager& rm, const YAML::Node& node):
	mark::module::base(rm, node),
	m_model(rm.image("cannon.png")),
	m_im_ray(rm.image("ray.png")) { }


void mark::module::cannon::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	this->serialize_base(out);
	out << EndMap;
}
