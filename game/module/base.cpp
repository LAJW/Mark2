#include "base.h"
#include <algorithm/accumulate.h>
#include <any>
#include <exception.h>
#include <property_manager.h>
#include <resource_image.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <unit/modular.h>
#include <unit/landing_pad.h>
#include <update_context.h>
#include <world.h>

let constexpr min_resistance = -2.f;
let constexpr max_resistance = .75f;

template <typename prop_man, typename T>
void mark::module::base::bind(prop_man& property_manager, T& instance)
{
	MARK_BIND(cur_health);
	MARK_BIND(max_health);
	MARK_BIND(armor);
	MARK_BIND(antimatter_resistance);
	MARK_BIND(heat_resistance);
	MARK_BIND(energy_resistance)
	MARK_BIND(stunned);
	MARK_BIND(cur_heat);
	MARK_BIND(size);
	MARK_BIND(blueprint_id)
}

static std::string to_percent(float value)
{
	std::ostringstream os;
	os.precision(1);
	os << value * 100.f;
	return os.str();
}

auto mark::module::base::describe() const -> std::string
{
	std::ostringstream os;
	os.precision(1);
	os << std::fixed;
	os << "Health: " << m_cur_health << " of " << m_max_health << std::endl;
	os << "Heat: " << m_cur_heat << " of " << 100.f << std::endl;
	module::modifiers total_modifiers;
	if (this->has_parent()) {
		for (let& neighbor : parent().neighbors_of(*this)) {
			let local_modifiers = neighbor.first.local_modifiers();
			total_modifiers.armor += local_modifiers.armor;
		}
	}
	let armor = m_armor + total_modifiers.armor;
	if (armor > 0.f) {
		os << "Armor: " << armor << std::endl;
	}
	if (m_antimatter_resistance != 0.f) {
		os << "Antimatter resistance: " << to_percent(m_antimatter_resistance)
		   << std::endl;
	}
	if (m_heat_resistance != 0.f) {
		os << "Heat resistance " << to_percent(m_heat_resistance) << std::endl;
	}
	if (m_energy_resistance != 0.f) {
		os << "Energy resistance " << to_percent(m_energy_resistance)
		   << std::endl;
	}
	return os.str();
}

mark::module::base_ref::base_ref(const YAML::Node& node)
	: m_grid_pos(node["grid_pos"].as<vi32>(vi32()))
{}

void mark::module::base_ref::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << Key << "grid_pos" << Value << BeginMap;
	out << Key << "x" << m_grid_pos.x;
	out << Value << "y" << m_grid_pos.y;
	out << EndMap;
}

auto mark::module::base_ref::parent() const -> const unit::modular&
{
	Expects(m_parent);
	return *m_parent;
}

auto mark::module::base_ref::parent() -> unit::modular&
{
	Expects(m_parent);
	return *m_parent;
}

auto mark::module::base_ref::grid_pos() const noexcept -> vi32
{
	return vi32(m_grid_pos);
}

auto mark::module::base_ref::has_parent() const -> bool
{
	return m_parent;
}

constexpr let HEAT_TRANSFER_RATE = 15.f;
constexpr let HEAT_LOSS_RATE = 2.f;

mark::module::base::~base() = default;

static auto is_landed(const mark::unit::modular& modular) -> bool
{
	if (let landing_pad =
		std::dynamic_pointer_cast<const mark::unit::landing_pad>(
				modular.world().target())) {
		return landing_pad->ship().get() == &modular;
	}
	return false;
}

void mark::module::base::update(update_context& context)
{
	let health_percentage = m_cur_health / m_max_health;
	let pos = this->pos();
	let dtf = static_cast<float>(context.dt);
	if (m_cur_heat > 90.f) {
		m_burning = true;
	} else if (m_cur_heat < 50.f) {
		m_burning = false;
	}
	if (m_burning) {
		m_cur_health -= dtf * 2.f;
	}

	auto neighbors = parent().neighbors_of(*this);
	let total_surface = 2 * (m_size.x + m_size.y);
	for (auto& pair : neighbors) {
		let & [ module, surface ] = pair;
		auto& module_heat = module.m_cur_heat;
		let delta_heat = static_cast<float>(surface)
			/ static_cast<float>(total_surface) * dtf * HEAT_TRANSFER_RATE;
		if (module_heat - m_cur_heat > delta_heat) {
			m_cur_heat += delta_heat;
			module_heat -= delta_heat;
		} else if (module_heat > m_cur_heat) {
			let avg_heat = (module_heat + m_cur_heat) / 2.f;
			m_cur_heat = avg_heat;
			module_heat = avg_heat;
		}
	}
	if (m_cur_heat >= 0.f) {
		m_cur_heat = std::max(
			m_cur_heat - HEAT_LOSS_RATE * static_cast<float>(context.dt), 0.f);
	} else {
		m_cur_heat = 0.f;
	}
	if (is_landed(this->parent())) {
		if (health_percentage <= 0.5f) {
			context.render([&] {
				update_context::spray_info _;
				_.image = world().resource_manager().image("glare.png");
				_.lifespan(.3f, 1.f);
				_.direction = -45.f;
				_.cone = 90.f;
				_.color = { 200, 200, 200, 25 };
				_.velocity(64.f, 128.f);
				_.pos = pos;
				_.diameter(16.f, 32.f);
				_.count = 4;
				_.layer = 5;
				return _;
			}());
		} else if (health_percentage <= 0.25f) {
			context.render([&] {
				update_context::spray_info _;
				_.image = world().resource_manager().image("glare.png");
				_.lifespan(.3f, 1.f);
				_.direction = -45.f;
				_.cone = 90.f;
				_.color = { 0, 0, 0, 75 };
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
		_.color = { 255, 255, 255, 100 };
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
	if (m_burning) {
		context.render([&] {
			update_context::spray_info _;
			_.image = world().resource_manager().image("explosion.png");
			_.lifespan(.3f, 1.f);
			_.direction = -45.f;
			_.cone = 90.f;
			_.velocity(64.f, 128.f);
			_.diameter(8.f, 16.f);
			_.pos = pos;
			_.count = 4;
			_.layer = 5;
			return _;
		}());
	}
}

auto mark::module::base::collide(const segment_t& ray)
	-> std::optional<std::pair<ref<interface::damageable>, vd>>
{
	let size = this->size();
	// half width
	let hw = static_cast<double>(size.x) / 2.0 * module::size;
	// half height
	let hh = static_cast<double>(size.y) / 2.0 * module::size;
	let pos = this->pos();
	let rotation = parent().rotation();
	const std::array<segment_t, 4> segments{
		segment_t{ { -hw, -hh }, { -hw, hh } }, // left
		segment_t{ { -hw, hh }, { hw, hh } },   // bottom
		segment_t{ { hw, hh }, { hw, -hh } },   // right
		segment_t{ { hw, -hh }, { -hw, -hh } }  // side
	};
	std::optional<vd> min;
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
		return { { std::ref(static_cast<interface::damageable&>(*this)),
				   *min } };
	}
	return {};
}

auto mark::module::base::neighbors()
	-> std::vector<std::pair<module::base&, unsigned>>
{
	return parent().neighbors_of(*this);
}

static auto resistance_to_multiplier(float resistance)
{
	return 1.f - std::clamp(resistance, min_resistance, max_resistance);
}

auto mark::module::base::damage(const interface::damageable::info& attr) -> bool
{
	if (attr.team == parent().team() || m_cur_health <= 0
		|| !attr.damaged->insert(this).second) {
		return false;
	}
	let[health, heat, stun] = damage_impl(attr);
	m_cur_health = std::max(0.f, m_cur_health - health);
	m_cur_heat = std::min(100.f, m_cur_heat + heat * 2.f);
	if (stun) {
		m_stunned += attr.stun_duration;
	}
	let direction = atan(parent().pos() - attr.pos);
	parent().knockback(*attr.knocked, direction, attr.knockback);
	return true;
}

auto mark::module::base::dead() const -> bool { return m_cur_health <= 0.f; }

void mark::module::base::command(const command::any&) {}

void mark::module::base::on_death(update_context& context)
{
	context.render([&] {
		update_context::spray_info _;
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
	auto modifiers = module::modifiers();
	modifiers.mass = 1.f;
	return modifiers;
}

auto mark::module::base::local_modifiers() const -> module::modifiers
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
	return { 255, intensity, intensity, 255 };
}

auto mark::module::base::pos() const -> vd
{
	let pos = (vector<float>(grid_pos()) + vector<float>(this->size()) / 2.f)
		* static_cast<float>(module::size);
	return parent().pos() + vd(rotate(pos, parent().rotation()));
}

auto mark::module::base::world() const -> const mark::world&
{
	return parent().world();
}

auto mark::module::base::thumbnail() const -> resource::image_ptr
{
	return m_thumbnail;
}

auto mark::module::base::size() const -> vu32 { return m_size; }

// Serialiser / Deserializer

static auto size_to_image_file_name(const mark::vu32& size) -> std::string
{
	if (size == mark::vu32{ 1, 1 }) {
		return "shadow-2x2.png";
	}
	if (size == mark::vu32{ 2, 2 }) {
		return "shadow-2x2.png";
	}
	if (size == mark::vu32{ 4, 2 }) {
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

auto mark::module::base::randomise(
	const std::unordered_map<std::string, YAML::Node>& blueprints,
	random& random) -> std::error_code
{
	if (m_blueprint_id.empty()) {
		return error::code::module_not_random;
	}
	property_manager property_manager(random);
	bind(property_manager, *this);
	return property_manager.randomise(blueprints.at(m_blueprint_id));
}

mark::module::base::base(
	resource::manager& rm,
	random& random,
	const YAML::Node& node)
	: base_ref(node)
	, m_thumbnail(rm.image(node["thumbnail"].as<std::string>("grid.png")))
{
	property_manager property_manager(random);
	bind(property_manager, *this);
	if (property_manager.deserialize(node)) {
		throw std::runtime_error("Could not deserialize module::base");
	}
	m_im_shadow = rm.image(size_to_image_file_name(m_size));
}

void mark::module::base::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << Key << "id" << Value << reinterpret_cast<size_t>(this);

	property_serializer serializer;
	bind(serializer, *this);
	serializer.serialize(out);

	base_ref::serialize(out);

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

auto mark::module::base::damage_impl(
	const interface::damageable::info& attr) const -> damage_result
{
	auto& random = *attr.random;
	let critical = random(0.f, 1.f) <= attr.critical_chance;
	let critical_multiplier = critical ? attr.critical_multiplier : 1.f;
	let stun = random(0.f, 1.f) <= attr.stun_chance;
	let heat_multiplier = resistance_to_multiplier(m_heat_resistance);
	let antimatter_multiplier =
		resistance_to_multiplier(m_antimatter_resistance);
	let heat_damage = attr.heat * critical_multiplier * heat_multiplier;
	let neighbors = parent().neighbors_of(*this);
	let armor =
		accumulate(neighbors, m_armor, [&](const float sum, const auto& pair) {
			return sum + pair.first.local_modifiers().armor;
		});
	let physical_damage =
		std::max(0.f, attr.physical * critical_multiplier - armor);
	let anitmatter_damage =
		attr.antimatter * critical_multiplier * antimatter_multiplier;
	let damage =
		std::max(1.f, physical_damage + anitmatter_damage + heat_damage);
	let heat_fraction = heat_damage / m_max_health * 100.f;
	return { damage, heat_fraction, stun };
}
