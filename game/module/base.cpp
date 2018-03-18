#include <stdafx.h>
#include "base.h"
#include <any>
#include <exception.h>
#include <property_manager.h>
#include <resource_image.h>
#include <resource_manager.h>
#include <sprite.h>
#include <tick_context.h>
#include <unit/modular.h>
#include <world.h>

mark::module::base_ref::base_ref(const YAML::Node& node)
	: m_grid_pos(node["grid_pos"].as<vector<int>>(vector<int>()))
{
}

void mark::module::base_ref::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << Key << "grid_pos" << Value << BeginMap;
	out << Key << "x" << m_grid_pos.x;
	out << Value << "y" << m_grid_pos.y;
	out << EndMap;
}

auto mark::module::base_ref::parent() const -> const unit::modular&
{
	assert(m_parent);
	return *m_parent;
}

auto mark::module::base_ref::parent() -> unit::modular&
{
	assert(m_parent);
	return *m_parent;
}

auto mark::module::base_ref::grid_pos() const noexcept -> vector<int>
{
	return vector<int>(m_grid_pos);
}

constexpr let HEAT_TRANSFER_RATE = 15.f;
constexpr let HEAT_LOSS_RATE = 2.f;

mark::module::base::~base() = default;

void mark::module::base::tick(tick_context& context)
{
	let health_percentage = m_cur_health / m_max_health;
	let pos = this->pos();

	auto neighbours = parent().neighbours_of(*this);
	let total_surface = 2 * (m_size.x + m_size.y);
	for (auto& pair : neighbours) {
		let & [ module, surface ] = pair;
		auto& module_heat = module.get().m_cur_heat;
		let delta_heat = static_cast<float>(surface) /
			static_cast<float>(total_surface) * static_cast<float>(context.dt) *
			HEAT_TRANSFER_RATE;
		if (module_heat - m_cur_heat > delta_heat) {
			m_cur_heat += delta_heat;
			module_heat -= delta_heat;
		}
		else if (module_heat > m_cur_heat) {
			let avg_heat = (module_heat + m_cur_heat) / 2.f;
			m_cur_heat = avg_heat;
			module_heat = avg_heat;
		}
	}
	if (m_cur_heat >= 0.f) {
		m_cur_heat = std::max(
			m_cur_heat - HEAT_LOSS_RATE * static_cast<float>(context.dt), 0.f);
	}
	else {
		m_cur_heat = 0.f;
	}
	if (!this->parent().landed()) {
		if (health_percentage <= 0.5f) {
			context.render([&] {
				tick_context::spray_info _;
				_.image = world().resource_manager().image("glare.png");
				_.lifespan(.3f, 1.f);
				_.direction = -45.f;
				_.cone = 90.f;
				_.color = {200, 200, 200, 25};
				_.velocity(64.f, 128.f);
				_.pos = pos;
				_.diameter(16.f, 32.f);
				_.count = 4;
				_.layer = 5;
				return _;
			}());
		}
		else if (health_percentage <= 0.25f) {
			context.render([&] {
				tick_context::spray_info _;
				_.image = world().resource_manager().image("glare.png");
				_.lifespan(.3f, 1.f);
				_.direction = -45.f;
				_.cone = 90.f;
				_.color = {0, 0, 0, 75};
				_.velocity(64.f, 128.f);
				_.diameter(16.f, 32.f);
				_.pos = pos;
				_.count = 4;
				_.layer = 5;
				return _;
			}());
		}
	}
	context.sprites[0].emplace_back([&] {
		sprite _;
		_.image = m_im_shadow;
		_.pos = pos;
		_.rotation = parent().rotation();
		_.size = module::size * 1.1f * std::max(m_size.y, m_size.x);
		_.color = {255, 255, 255, 100};
		return _;
	}());
	if (m_stunned > 0) {
		m_stun_lfo =
			std::fmod(m_stun_lfo + static_cast<float>(context.dt), 1.f);
		m_stunned = std::max(m_stunned - static_cast<float>(context.dt), 0.f);
		context.sprites[3].emplace_back([&] {
			sprite _;
			_.image = parent().world().image_stun;
			_.pos = pos;
			_.rotation = m_stun_lfo * 360.f;
			_.size = module::size * 2;
			return _;
		}());
	}
}

auto mark::module::base::collide(const segment_t& ray) -> std::optional<
	std::pair<std::reference_wrapper<interface::damageable>, vector<double>>>
{
	let size = this->size();
	// half width
	let hw = static_cast<double>(size.x) / 2.0 * module::size;
	// half height
	let hh = static_cast<double>(size.y) / 2.0 * module::size;
	let pos = this->pos();
	let rotation = parent().rotation();
	const std::array<segment_t, 4> segments{
		segment_t{{-hw, -hh}, {-hw, hh}}, // left
		segment_t{{-hw, hh}, {hw, hh}},   // bottom
		segment_t{{hw, hh}, {hw, -hh}},   // right
		segment_t{{hw, -hh}, {-hw, -hh}}  // side
	};
	std::optional<vector<double>> min;
	double min_length = INFINITY;
	for (let& raw : segments) {
		let segment = std::make_pair(
			rotate(raw.first, rotation) + pos,
			rotate(raw.second, rotation) + pos);
		if (let intersection = intersect(segment, ray)) {
			let length = mark::length(*intersection - pos);
			if (length < min_length) {
				min_length = length;
				min = *intersection;
			}
		}
	}
	if (min) {
		return {{std::ref(static_cast<interface::damageable&>(*this)), *min}};
	}
	return {};
}

auto mark::module::base::neighbours()
	-> std::vector<std::pair<std::reference_wrapper<module::base>, unsigned>>
{
	return parent().neighbours_of(*this);
}

bool mark::module::base::damage(const interface::damageable::info& attr)
{
	if (attr.team != parent().team() && m_cur_health > 0 &&
		attr.damaged->find(this) == attr.damaged->end()) {
		auto& rm = parent().world().resource_manager();
		let critical = rm.random(0.f, 1.f) <= attr.critical_chance;
		let stun = rm.random(0.f, 1.f) <= attr.stun_chance;
		attr.damaged->insert(this);
		if (critical) {
			m_cur_health -= attr.physical * attr.critical_multiplier;
		}
		else {
			m_cur_health -= attr.physical;
		}
		if (stun) {
			m_stunned += attr.stun_duration;
		}
		return true;
	}
	return false;
}

auto mark::module::base::dead() const -> bool { return m_cur_health <= 0.f; }

void mark::module::base::command(const command::any&) {}

void mark::module::base::on_death(tick_context& context)
{
	context.render([&] {
		tick_context::spray_info _;
		_.image = parent().world().resource_manager().image("explosion.png");
		_.pos = pos();
		_.velocity(75.f, 150.f);
		_.lifespan(0.3f);
		_.diameter(24.f);
		_.count = 20;
		return _;
	}());
}

auto mark::module::base::global_modifiers() const -> module::modifiers
{
	return module::modifiers();
}

auto mark::module::base::reserved() const noexcept -> reserved_kind
{
	return reserved_kind::none;
}

auto mark::module::base::heat_color() const -> sf::Color
{
	let intensity = static_cast<uint8_t>((1.f - m_cur_heat / max_heat) * 255.f);
	return {255, intensity, intensity, 255};
}

auto mark::module::base::pos() const -> vector<double>
{
	let pos = (vector<float>(grid_pos()) + vector<float>(this->size()) / 2.f) *
		static_cast<float>(module::size);
	return parent().pos() + vector<double>(rotate(pos, parent().rotation()));
}

auto mark::module::base::world() const -> const mark::world&
{
	return parent().world();
}

auto mark::module::base::thumbnail() const
	-> std::shared_ptr<const resource::image>
{
	return m_thumbnail;
}

auto mark::module::base::size() const -> vector<unsigned> { return m_size; }

// Serialiser / Deserialiser

static auto size_to_image_file_name(const mark::vector<unsigned>& size)
	-> std::string
{
	if (size == mark::vector<unsigned>{2, 2}) {
		return "shadow-2x2.png";
	}
	if (size == mark::vector<unsigned>{4, 2}) {
		return "shadow-4x2.png";
	}
	std::terminate();
}

auto mark::module::base::cur_health() const -> float { return m_cur_health; }

auto mark::module::base::max_health() const -> float { return m_max_health; }

auto mark::module::base::needs_healing() const -> bool
{
	return m_cur_health < m_max_health;
}

void mark::module::base::heal(float amount)
{
	m_cur_health = std::min(m_cur_health + amount, m_max_health);
}

auto mark::module::base::team() const -> size_t { return parent().team(); }

template <typename prop_man, typename T>
void mark::module::base::bind(prop_man& property_manager, T& instance)
{
	MARK_BIND(cur_health);
	MARK_BIND(max_health);
	MARK_BIND(stunned);
	MARK_BIND(cur_heat);
	MARK_BIND(size);
}

mark::module::base::base(resource::manager& rm, const YAML::Node& node)
	: base_ref(node)
	, m_thumbnail(rm.image(node["thumbnail"].as<std::string>("grid.png")))
{
	property_manager property_manager(rm);
	bind(property_manager, *this);
	property_manager.deserialise(node);
	m_im_shadow = rm.image(size_to_image_file_name(m_size));
}

void mark::module::base::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << Key << "id" << Value << reinterpret_cast<size_t>(this);

	property_serialiser serialiser;
	bind(serialiser, *this);
	serialiser.serialise(out);

	base_ref::serialise(out);

	out << Key << "thumbnail" << Value << m_thumbnail->filename();
}

float mark::module::base::parent_rotation() const
{
	return parent().rotation();
}

auto mark::module::base::world() noexcept -> mark::world&
{
	return parent().world();
}
