#include "healing_turret.h"
#include <algorithm.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>

void mark::module::healing_turret::update(update_context& context)
{
	this->module::base::update(context);
	m_model.update(context.dt);
	let pos = this->pos();
	let model_size = std::max(this->size().x, this->size().y) * module::size;
	context.sprites[2].push_back(m_model.render(
		pos, model_size, parent().rotation(), this->heat_color()));
	if (!this->target()) {
		let neighbours = [&] {
			std::unordered_set<gsl::not_null<mark::module::base*>> neighbours;
			const auto m_radius = 100.f;
			let radius = static_cast<int>(std::round(m_radius / 16.f));
			for (let offset : mark::range<vector<int>>(
					 { -radius, -radius }, { radius + 1, radius + 1 })) {
				let module =
					this->parent().module_at(this->grid_pos() + offset);
				if (module && module != this
					&& length(module->pos() - this->pos()) < m_radius) {
					neighbours.insert(module);
				}
			}
			return neighbours;
		}();
		let min_health_neighbour = min_element_v(neighbours, [](let a, let b) {
			return a->cur_health() / a->max_health()
				< b->cur_health() / b->max_health();
		});
		if (min_health_neighbour && (*min_health_neighbour)->needs_healing()) {
			m_target = (*min_health_neighbour)->grid_pos();
		}
	}
	let target = this->target();
	if (!target)
		return;
	target->heal(static_cast<float>(10. * context.dt));
	let collision = target->pos();
	let dir = normalize(collision - pos);
	let rotation = atan(dir);
	context.render([&] {
		update_context::spray_info _;
		_.image = m_im_ray;
		_.pos = collision;
		_.velocity(25.f, 50.f);
		_.lifespan(1.f);
		_.diameter(8.f);
		_.count = 4;
		_.direction = -rotation;
		_.cone = 180.f;
		_.color = sf::Color::Green;
		_.layer = 3;
		return _;
	}());
	let len = static_cast<int>(length(collision - pos) / double(module::size));
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

auto mark::module::healing_turret::target() -> mark::module::base*
{
	if (m_stunned || !m_target) {
		return nullptr;
	}
	let target = parent().module_at(*m_target);
	if (!target || !target->needs_healing()) {
		return nullptr;
	}
	return target;
}

std::string mark::module::healing_turret::describe() const
{
	return "Healing Turret\n"
		   "HPS: 100\n";
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
	const YAML::Node& node)
	: module::base(rm, node)
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
