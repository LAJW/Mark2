#include "stdafx.h"
#include "module_cannon.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include "unit_modular.h"

mark::module::cannon::cannon(resource::manager& rm)
	: module::base_turret({ 4, 2 }, rm.image("cannon.png"))
	, m_model(rm.image("cannon.png"))
	, m_im_ray(rm.image("ray.png"))
	, m_randomiser(rm.random(1.f, 1.2f), rm.random(0.f, 1.f)) { }

void mark::module::cannon::tick(tick_context& context) {
	this->module::base::tick(context);
	m_randomiser.tick(context.dt);
	m_model.tick(context.dt);
	auto pos = this->pos();
	let rotation = parent().rotation();
	let model_size = std::max(this->size().x, this->size().y)
		* module::size;
	context.sprites[2].push_back(m_model.render(
		pos,
		model_size,
		rotation,
		this->heat_color()));
	auto& world = parent().world();
	base_turret::tick();
	if (m_angular_velocity == 0.f) {
		m_rotation = rotation;
	} else if (let target = this->target()) {
		m_rotation = turn(*target - pos, m_rotation, m_angular_velocity, context.dt);
	}
	if (this->can_shoot()) {
		std::unordered_set<interface::damageable*> damaged;

		let dir = rotate(vector<double>(1, 0), m_rotation + m_randomiser.get() );
		let prev = pos;
		let cur = pos + dir * static_cast<double>(module::size * 200);
		world::damage_info info;
		info.context = &context;
		info.aoe_radius = 0.f;
		info.piercing = 1;
		info.segment = { prev, cur };
		info.damage.damaged = &damaged;
		info.damage.physical = 100.f * static_cast<float>(context.dt);
		info.damage.team = parent().team();
		let collisions = world.damage(info).first;
		for (let& collision : collisions) {
			tick_context::spray_info spray;
			spray.image = m_im_ray;
			spray.pos = collision;
			spray.velocity(25.f, 50.f);
			spray.lifespan(1.f);
			spray.diameter(8.f);
			spray.count = 4;
			spray.direction = m_rotation + 180.f;
			spray.cone = 180.f;
			spray.color = sf::Color::Red;
			context.render(spray);
		}
		if (!collisions.empty()) {
			let collision = collisions.back();
			let len = int(length(collision - pos) / double(module::size));
			context.lights.push_back({ collision, sf::Color::Red });
			for (int i = 1; i < len; i++) {
				let cur_len = module::size * static_cast<double>(i) - module::size * 0.5;
				sprite ray_sprite;
				ray_sprite.image = m_im_ray;
				ray_sprite.pos = collision - dir * cur_len;
				ray_sprite.size = module::size;
				ray_sprite.rotation = m_rotation;
				ray_sprite.color = sf::Color::Red;
				context.sprites[0].emplace_back(ray_sprite);
			}
		} else {
			for (int i = 1; i < 200; i++) {
				let cur_len = module::size * static_cast<double>(200);
				sprite ray_sprite;
				ray_sprite.image = m_im_ray;
				ray_sprite.pos = pos + dir * (cur_len + 2.0);
				ray_sprite.size = module::size;
				ray_sprite.rotation = m_rotation;
				ray_sprite.color = sf::Color::Red;
				context.sprites[0].emplace_back(ray_sprite);
			}
		}
	}
}

std::string mark::module::cannon::describe() const {
	return "Laser Cannon\n"
		"DPS: 60\n";
}

// Serialize / Deserialize

mark::module::cannon::cannon(resource::manager& rm, const YAML::Node& node)
	: module::base_turret(rm, node)
	, m_model(rm.image("cannon.png"))
	, m_im_ray(rm.image("ray.png"))
	, m_randomiser(rm.random(1.f, 1.2f), rm.random(0.f, 1.f)) { }


void mark::module::cannon::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base_turret::serialise(out);
	out << EndMap;
}

auto mark::module::cannon::passive() const noexcept -> bool
{ return false; }
