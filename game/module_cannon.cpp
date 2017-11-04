#include "stdafx.h"
#include "module_cannon.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include "unit_modular.h"

mark::module::cannon::cannon(resource::manager& resource_manager):
	module::base_turret({ 4, 2 }, resource_manager.image("cannon.png")),
	m_model(resource_manager.image("cannon.png")),
	m_im_ray(resource_manager.image("ray.png")) {

}

void mark::module::cannon::tick(tick_context& context) {
	this->module::base::tick(context);
	m_model.tick(context.dt);
	auto pos = this->pos();
	const auto rotation = parent().rotation();
	const auto model_size = std::max(this->size().x, this->size().y)
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
	} else if (const auto target = this->target()) {
		m_rotation = turn(*target - pos, m_rotation, m_angular_velocity, context.dt);
	}
	if (this->can_shoot()) {
		std::unordered_set<interface::damageable*> damaged;

		std::optional<vector<double>> collision_pos;
		int len;
		const auto dir = rotate(vector<double>(1, 0), m_rotation);
		for (len = 1; len < 200; len++) {
			const auto cur_len = module::size * static_cast<double>(len);
			const auto prev = pos + dir * (cur_len - module::size);
			const auto cur = pos + dir * (cur_len + 2.0);
			world::damage_info info;
			info.context = &context;
			info.aoe_radius = 0.f;
			info.piercing = 1;
			info.segment = { prev, cur };
			info.damage.damaged = &damaged;
			info.damage.physical = 100.f * static_cast<float>(context.dt);
			info.damage.team = parent().team();
			if (const auto result = world.damage(info)) {
				collision_pos = result;
				tick_context::spray_info spray;
				spray.image = m_im_ray;
				spray.pos = *collision_pos;
				spray.velocity(25.f, 50.f);
				spray.lifespan(1.f);
				spray.diameter(8.f);
				spray.count = 4;
				spray.direction = m_rotation + 180.f;
				spray.cone = 180.f;
				spray.color = sf::Color::Red;
				// context.render(spray);
				break;
			}
		}
		if (collision_pos) {
			for (int i = 1; i < len; i++) {
				const auto cur_len = module::size * static_cast<double>(i) - module::size * 0.5;
				sprite ray_sprite;
				ray_sprite.image = m_im_ray;
				ray_sprite.pos = *collision_pos - dir * cur_len;
				ray_sprite.size = module::size;
				ray_sprite.rotation = m_rotation;
				ray_sprite.color = sf::Color::Red;
				context.sprites[0].emplace_back(ray_sprite);
			}
		} else {
			for (int i = 1; i < 200; i++) {
				const auto cur_len = module::size * static_cast<double>(len);
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

mark::module::cannon::cannon(resource::manager& rm, const YAML::Node& node):
	module::base_turret(rm, node),
	m_model(rm.image("cannon.png")),
	m_im_ray(rm.image("ray.png")) { }


void mark::module::cannon::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base_turret::serialise(out);
	out << EndMap;
}

auto mark::module::cannon::passive() const noexcept -> bool
{ return false; }
