#include "laser.h"
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>
#include <unit/modular.h>
#include <world.h>

void mark::module::laser::update(update_context& context)
{
	this->module::base::update(context);
	m_randomiser.update(context.dt);
	m_model.update(context.dt);
	auto pos = this->pos();
	m_targeting_system.update(context);
	if (m_angular_velocity == 0.f) {
		m_rotation = parent().rotation();
	} else if (let target = m_targeting_system.target()) {
		m_rotation =
			turn(*target - pos, m_rotation, m_angular_velocity, context.dt);
	}
	let dir = rotate(vd(1, 0), m_rotation + m_randomiser.get());
	let is_firing = !m_stunned && m_targeting_system.can_shoot();
	if (!is_firing) {
		this->render(context, {}, is_firing, dir);
		return;
	}
	std::unordered_set<gsl::not_null<interface::damageable*>> damaged;
	auto damage_result = parent().world().damage([&] {
		world::damage_info _;
		_.context = &context;
		_.aoe_radius = 0.f;
		_.piercing = 1;
		let prev = pos;
		let cur = pos + dir * static_cast<double>(module::size * 200);
		_.segment = { prev, cur };
		_.damage.damaged = &damaged;
		_.damage.physical = 100.f * static_cast<float>(context.dt);
		_.damage.team = parent().team();
		return _;
	}());
	this->render(context, move(damage_result.collisions), is_firing, dir);
}

void mark::module::laser::render(
	update_context& context,
	std::vector<vd> collisions,
	bool is_firing,
	const vd dir) const
{
	let pos = this->pos();
	let model_size = std::max(this->size().x, this->size().y) * module::size;
	context.sprites[2].push_back(m_model.render(
		pos, model_size, parent().rotation(), this->heat_color()));
	if (!is_firing) {
		return;
	}
	if (!collisions.empty()) {
		let collision = collisions.back();
		let len = int(length(collision - pos) / double(module::size));
		context.lights.push_back({ collision, sf::Color::Red });
		if (len <= 1) {
			return;
		}
		for (let i : range(1, len)) {
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
		return;
	}
	for (let& collision : collisions) {
		context.render([&] {
			update_context::spray_info _;
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
	for (let i : range(1, 200)) {
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

std::string mark::module::laser::describe() const
{
	return "Laser Cannon\n"
		   "DPS: 60\n";
}

// Serialize / Deserialize

template <typename prop_man, typename T>
void mark::module::laser::bind(prop_man& property_manager, T& instance)
{
	// No properties to serialize yet
	(void)property_manager;
	(void)instance;
}

mark::module::laser::laser(resource::manager& rm, const YAML::Node& node)
	: module::base(rm, node)
	, m_targeting_system(*this)
	, m_model(rm.image("cannon.png"))
	, m_im_ray(rm.image("ray.png"))
	, m_randomiser(rm.random(1.f, 1.2f), rm.random(0.f, 1.f))
{}

void mark::module::laser::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialize(out);
	out << EndMap;
}

auto mark::module::laser::passive() const noexcept -> bool { return false; }

void mark::module::laser::command(const command::any& any)
{
	m_targeting_system.command(any);
}
