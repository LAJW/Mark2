#include "healing_turret.h"
#include <algorithm.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>

namespace mark {
static auto in_range(const module::base& a, const module::base& b, double range)
{
	return length(a.pos() - b.pos()) < range;
}

static auto neighbor_at_pos_in_range(
	const module::base& root,
	vector<int> pos,
	double range)
{
	let module = root.parent().module_at(pos);
	return module && module != &root && in_range(*module, root, range)
		? module
		: nullptr;
}

static auto neighbors_in_radius(const module::base& root, double radius)
{
	std::unordered_set<gsl::not_null<const module::base*>> neighbors;
	let center = root.grid_pos() + vector<int>(root.size()) / 2;
	let bound = static_cast<int>(std::ceil(radius / 16.));
	for (let offset :
		 range<vector<int>>({ -bound, -bound }, { bound + 1, bound + 1 })) {
		let module = neighbor_at_pos_in_range(root, center + offset, radius);
		if (module) {
			neighbors.insert(module);
		}
	}
	return neighbors;
}

static auto
most_damaged_neighbor_in_range(const module::base& root, double range)
{
	let neighbors = neighbors_in_radius(root, range);
	let min_health_neighbour = min_element_v(neighbors, [](let a, let b) {
		return a->cur_health() / a->max_health()
			< b->cur_health() / b->max_health();
	});
	return min_health_neighbour && (*min_health_neighbour)->needs_healing()
		? min_health_neighbour->get()
		: nullptr;
}
} // namespace mark

void mark::module::healing_turret::update(update_context& context)
{
	this->module::base::update(context);
	m_model.update(context.dt);
	if (!this->target()) {
		if (let target = most_damaged_neighbor_in_range(*this, 100.0)) {
			m_target = target->grid_pos();
		}
	}
	if (let target = this->target()) {
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
	let target = this->target();
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

auto mark::module::healing_turret::target() const -> const mark::module::base*
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

auto mark::module::healing_turret::target() -> mark::module::base*
{
	let& self = *this;
	return const_cast<module::base*>(self.target());
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
