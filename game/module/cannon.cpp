#include <stdafx.h>
#include "cannon.h"
#include <resource_manager.h>
#include <sprite.h>
#include <tick_context.h>
#include <unit/modular.h>
#include <world.h>

void mark::module::cannon::tick(tick_context& context)
{
	this->module::base::tick(context);
	m_randomiser.tick(context.dt);
	m_model.tick(context.dt);
	auto pos = this->pos();
	let rotation = parent().rotation();
	let model_size = std::max(this->size().x, this->size().y) * module::size;
	context.sprites[2].push_back(
		m_model.render(pos, model_size, rotation, this->heat_color()));
	auto& world = parent().world();
	m_targeting_system.tick();
	if (m_angular_velocity == 0.f) {
		m_rotation = rotation;
	}
	else if (let target = m_targeting_system.target()) {
		m_rotation =
			turn(*target - pos, m_rotation, m_angular_velocity, context.dt);
	}
	if (!m_stunned && m_targeting_system.request_charge()) {
		let dir = rotate(vector<double>(1, 0), m_rotation + m_randomiser.get());
		std::unordered_set<interface::damageable*> damaged;
		let collisions =
			world
				.damage([&] {
					world::damage_info _;
					_.context = &context;
					_.aoe_radius = 0.f;
					_.piercing = 1;
					let prev = pos;
					let cur =
						pos + dir * static_cast<double>(module::size * 200);
					_.segment = {prev, cur};
					_.damage.damaged = &damaged;
					_.damage.physical = 100.f * static_cast<float>(context.dt);
					_.damage.team = parent().team();
					return _;
				}())
				.first;
		for (let& collision : collisions) {
			context.render([&] {
				tick_context::spray_info _;
				_.image = m_im_ray;
				_.pos = collision;
				_.velocity(25.f, 50.f);
				_.lifespan(1.f);
				_.diameter(8.f);
				_.count = 4;
				_.direction = m_rotation + 180.f;
				_.cone = 180.f;
				_.color = sf::Color::Red;
				return _;
			}());
		}
		if (!collisions.empty()) {
			let collision = collisions.back();
			let len = int(length(collision - pos) / double(module::size));
			context.lights.push_back({collision, sf::Color::Red});
			for (int i = 1; i < len; i++) {
				let cur_len =
					module::size * static_cast<double>(i) - module::size * 0.5;
				context.sprites[0].emplace_back([&] {
					sprite _;
					_.image = m_im_ray;
					_.pos = collision - dir * cur_len;
					_.size = module::size;
					_.rotation = m_rotation;
					_.color = sf::Color::Red;
					return _;
				}());
			}
		}
		else {
			for (int i = 1; i < 200; i++) {
				let cur_len = module::size * static_cast<double>(200);
				context.sprites[0].emplace_back([&] {
					sprite _;
					_.image = m_im_ray;
					_.pos = pos + dir * (cur_len + 2.0);
					_.size = module::size;
					_.rotation = m_rotation;
					_.color = sf::Color::Red;
					return _;
				}());
			}
		}
	}
}

std::string mark::module::cannon::describe() const
{
	return "Laser Cannon\n"
		   "DPS: 60\n";
}

// Serialize / Deserialize

mark::module::cannon::cannon(resource::manager& rm, const YAML::Node& node)
	: module::base(rm, node)
	, m_targeting_system(*this)
	, m_model(rm.image("cannon.png"))
	, m_im_ray(rm.image("ray.png"))
	, m_randomiser(rm.random(1.f, 1.2f), rm.random(0.f, 1.f))
{
}

void mark::module::cannon::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << EndMap;
}

auto mark::module::cannon::passive() const noexcept -> bool { return false; }
