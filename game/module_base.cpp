#include "stdafx.h"
#include "module_base.h"
#include "exception.h"
#include "tick_context.h"
#include "world.h"
#include "resource_image.h"
#include "unit_modular.h"

constexpr const auto HEAT_TRANSFER_RATE = 15.f;
constexpr const auto HEAT_LOSS_RATE = 2.f;

mark::module::base::base(
	vector<unsigned> size,
	const std::shared_ptr<const resource::image>& thumbnail)
	: m_size(size)
	, m_thumbnail(thumbnail)
{
	assert(size.x <= module::max_dimension);
	assert(size.y <= module::max_dimension);
}

mark::module::base::~base() = default;

void mark::module::base::tick(tick_context & context) {
	const auto health_percentage = m_cur_health / m_max_health;
	const auto pos = this->pos();

	auto attached = parent().attached(*this);
	const auto total_surface = 2 * (m_size.x + m_size.y);
	for (auto& pair : attached) {
		const auto&[module, surface] = pair;
		auto& module_heat = module.get().m_cur_heat;
		const auto delta_heat
			= static_cast<float>(surface) / static_cast<float>(total_surface)
				* static_cast<float>(context.dt) * HEAT_TRANSFER_RATE;
		if (module_heat - m_cur_heat > delta_heat) {
			m_cur_heat += delta_heat;
			module_heat -= delta_heat;
		} else if (module_heat > m_cur_heat) {
			const auto avg_heat = (module_heat + m_cur_heat) / 2.f;
			m_cur_heat = avg_heat;
			module_heat = avg_heat;
		}
	}
	if (m_cur_heat >= 0.f) {
		m_cur_heat = std::max(
			m_cur_heat - HEAT_LOSS_RATE * static_cast<float>(context.dt),
			0.f);
	} else {
		m_cur_heat = 0.f;
	}
	if (!this->parent().landed()) {
		if (health_percentage <= 0.5f) {
			tick_context::spray_info info;
			info.image = parent().world().resource_manager().image("glare.png");
			info.lifespan(.3f, 1.f);
			info.direction = -45.f;
			info.cone = 90.f;
			info.color = { 200, 200, 200, 25 };
			info.velocity(64.f, 128.f);
			info.pos = pos;
			info.diameter(16.f, 32.f);
			info.count = 4;
			info.layer = 5;
			context.render(info);
		} else if (health_percentage <= 0.25f) {
			tick_context::spray_info info;
			info.image = parent().world().resource_manager().image("glare.png");
			info.lifespan(.3f, 1.f);
			info.direction = -45.f;
			info.cone = 90.f;
			info.color = { 0, 0, 0, 75 };
			info.velocity(64.f, 128.f);
			info.diameter(16.f, 32.f);
			info.pos = pos;
			info.count = 4;
			info.layer = 5;
			context.render(info);
		}
	}
	if (m_stunned > 0) {
		m_stun_lfo = std::fmod(m_stun_lfo + static_cast<float>(context.dt), 1.f);
		m_stunned = std::max(m_stunned - static_cast<float>(context.dt), 0.f);
		sprite stun_sprite;
		stun_sprite.image = parent().world().image_stun;
		stun_sprite.pos = pos;
		stun_sprite.rotation = m_stun_lfo * 360.f;
		stun_sprite.size = module::size * 2;
		context.sprites[3].emplace_back(stun_sprite);
	}

}

auto mark::module::base::collide(const segment_t& ray) ->
	std::pair<interface::damageable*, vector<double>> {
	const auto size = this->size();
	// half width
	const auto hw = static_cast<double>(size.x) / 2.0 * module::size;
	// half height
	const auto hh = static_cast<double>(size.y) / 2.0 * module::size;
	const auto pos = this->pos();
	const auto rotation = parent().rotation();
	const std::array<segment_t, 4> segments {
		segment_t{ { -hw, -hh }, { -hw, hh } },  // left
		segment_t{ { -hw, hh },  { hw, hh } },   // bottom
		segment_t{ { hw, hh },   { hw, -hh } },  // right
		segment_t{ { hw, -hh },  { -hw, -hh } }  // side
	};
	auto min = vector<double>(NAN, NAN);
	double min_length = 40000.0;
	for (const auto& raw : segments) {
		const auto segment = std::make_pair(
			rotate(raw.first, rotation) + pos,
			rotate(raw.second, rotation) + pos
		);
		const auto intersection = intersect(segment, ray);
		if (!std::isnan(intersection.x)) {
			const auto length = mark::length(intersection - pos);
			if (length < min_length) {
				min_length = length;
				min = intersection;
			}
		}
	}
	if (!std::isnan(min.x)) {
		return { this, min };
	} else {
		return { nullptr, min };
	}
}

auto mark::module::base::neighbours()
	-> std::vector<std::pair<std::reference_wrapper<module::base>, unsigned>> {
	return parent().attached(*this);
}

auto mark::module::base::grid_pos() const noexcept -> vector<int> {
	return vector<int>(m_grid_pos);
}

bool mark::module::base::damage(const interface::damageable::info& attr) {
	if (attr.team != parent().team() && m_cur_health > 0
		&& attr.damaged->find(this) == attr.damaged->end()) {
		auto& rm = parent().world().resource_manager();
		const auto critical = rm.random(0.f, 1.f) <= attr.critical_chance;
		const auto stun = rm.random(0.f, 1.f) <= attr.stun_chance;
		attr.damaged->insert(this);
		if (critical) {
			m_cur_health -= attr.physical * attr.critical_multiplier;
		} else {
			m_cur_health -= attr.physical;
		}
		if (stun) {
			m_stunned += attr.stun_duration;
		}
		return true;
	}
	return false;
}

auto mark::module::base::dead() const -> bool
{ return m_cur_health <= 0.f; }

void mark::module::base::target(vector<double>)
{}

void mark::module::base::queue(vector<double>, bool)
{}

void mark::module::base::shoot(vector<double>, bool)
{}

void mark::module::base::on_death(tick_context& context) {
	tick_context::spray_info spray;
	spray.image = parent().world().resource_manager().image("explosion.png");
	spray.pos = pos();
	spray.velocity(75.f, 150.f);
	spray.lifespan(0.3f);
	spray.diameter(24.f);
	spray.count = 20;
	context.render(spray);
}

auto mark::module::base::global_modifiers() const->module::modifiers
{
	return module::modifiers();
}

auto mark::module::base::reserved() const noexcept -> reserved_type
{ return reserved_type::none; }

auto mark::module::base::parent() const -> const unit::modular& {
	if (m_parent) {
		return *m_parent;
	} else {
		throw exception("NO_PARENT");
	}
}

auto mark::module::base::parent() -> unit::modular&{
	return const_cast<unit::modular&>(static_cast<const module::base*>(this)->parent());
}

auto mark::module::base::heat_color() const -> sf::Color {
	const auto intensity = static_cast<uint8_t>((1.f - m_cur_heat / max_heat) * 255.f);
	return { 255, intensity, intensity, 255 };
}

auto mark::module::base::pos() const -> vector<double> {
	const auto pos = (vector<float>(grid_pos()) + vector<float>(this->size()) / 2.f)
		* static_cast<float>(module::size);
	return parent().pos() + vector<double>(rotate(pos, parent().rotation()));
}

auto mark::module::base::thumbnail() const -> std::shared_ptr<const resource::image> {
	return m_thumbnail;
}

auto mark::module::base::size() const -> vector<unsigned> {
	return m_size;
}

// Serializer / Deserializer

mark::module::base::base(resource::manager& rm, const YAML::Node& node):
	m_cur_health(node["cur_health"].as<float>()),
	m_max_health(node["max_health"].as<float>()),
	m_stunned(node["stunned"].as<float>()),
	m_cur_heat(node["cur_heat"].as<float>()),
	m_grid_pos(node["grid_pos"].as<vector<int>>()),
	m_size(node["size"].as<vector<unsigned>>()),
	m_thumbnail(rm.image(node["thumbnail"].as<std::string>("grid.png"))) { }

void mark::module::base::serialize_base(YAML::Emitter& out) const {
	using namespace YAML;
	out << Key << "id" << Value << this->id();
	out << Key << "cur_health" << Value << m_cur_health;
	out << Key << "max_health" << Value << m_max_health;
	out << Key << "stunned" << Value << m_stunned;
	out << Key << "cur_heat" << Value << m_cur_heat;

	out << Key << "size" << Value << BeginMap;
	out << Key << "x" << m_size.x;
	out << Value << "y" << m_size.y;
	out << EndMap;

	out << Key << "grid_pos" << Value << BeginMap;
	out << Key << "x" << m_grid_pos.x;
	out << Value << "y" << m_grid_pos.y;
	out << EndMap;

	out << Key << "thumbnail" << Value << m_thumbnail->filename();
}

float mark::module::base::parent_rotation() const
{ return parent().rotation(); }

auto mark::module::base::world() noexcept -> mark::world &
{ return parent().world(); }
