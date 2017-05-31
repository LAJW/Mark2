#include <array>
#include <assert.h>
#include "module_base.h"
#include "exception.h"
#include "tick_context.h"
#include "world.h"

mark::module::base::base(mark::vector<unsigned> size, const std::shared_ptr<const mark::resource::image>& thumbnail):
	m_size(size),
	m_thumbnail(thumbnail) {
	assert(size.x <= mark::module::max_dimension);
	assert(size.y <= mark::module::max_dimension);
}

void mark::module::base::tick(mark::tick_context & context) {
	const auto health_percentage = m_cur_health / m_max_health;
	const auto pos = this->pos();

	auto attached = parent().get_attached(*this);
	for (auto& module : attached) {
		auto& module_heat = module.get().m_cur_heat;
		if (module_heat - m_cur_heat > 1.f) {
			m_cur_heat += 0.5f;
			module_heat -= 0.5f;
		} else if (module_heat > m_cur_heat) {
			const auto avg_heat = (module_heat + m_cur_heat) / 2.f;
			m_cur_heat = avg_heat;
			module_heat = avg_heat;
		}
	}
	if (m_cur_heat >= 0.f) {
		m_cur_heat = std::max(m_cur_heat - 2.f * static_cast<float>(context.dt), 0.f);
	}
	if (health_percentage <= 0.5f) {
		mark::tick_context::spray_info info;
		info.image = parent().world().resource_manager().image("glare.png");
		info.lifespan(.3f, 1.f);
		info.direction = -45.f;
		info.cone = 90.f;
		info.color = { 200, 200, 200, 25 };
		info.velocity(64.f, 128.f);
		info.pos = pos;
		info.diameter(16.f, 32.f);
		info.count = 4;
		context.render(info);
	} else if (health_percentage <= 0.25f) {
		mark::tick_context::spray_info info;
		info.image = parent().world().resource_manager().image("glare.png");
		info.lifespan(.3f, 1.f);
		info.direction = -45.f;
		info.cone = 90.f;
		info.color = { 0, 0, 0, 75 };
		info.velocity(64.f, 128.f);
		info.diameter(16.f, 32.f);
		info.pos = pos;
		info.count = 4;
		context.render(info);
	}
	if (m_stunned > 0) {
		m_stun_lfo = std::fmod(m_stun_lfo + static_cast<float>(context.dt), 1.f);
		m_stunned = std::max(m_stunned - static_cast<float>(context.dt), 0.f);
		mark::sprite::info stun_sprite;
		stun_sprite.image = parent().world().image_stun;
		stun_sprite.pos = pos;
		stun_sprite.rotation = m_stun_lfo * 360.f;
		stun_sprite.size = mark::module::size * 2;
		context.sprites[3].emplace_back(stun_sprite);
	}

}

auto mark::module::base::collide(const mark::segment_t& ray) ->
	std::pair<mark::idamageable*, mark::vector<double>> {
	const auto size = this->size();
	// half width
	const auto hw = static_cast<double>(size.x) / 2.0 * mark::module::size;
	// half height
	const auto hh = static_cast<double>(size.y) / 2.0 * mark::module::size;
	const auto pos = this->pos();
	const auto rotation = parent().rotation();
	const std::array<segment_t, 4> segments {
		mark::segment_t{ { -hw, -hh }, { -hw, hh } },  // left
		mark::segment_t{ { -hw, hh },  { hw, hh } },   // bottom
		mark::segment_t{ { hw, hh },   { hw, -hh } },  // right
		mark::segment_t{ { hw, -hh },  { -hw, -hh } }  // side
	};
	auto min = mark::vector<double>(NAN, NAN);
	double min_length = 40000.0;
	for (const auto& raw : segments) {
		const auto segment = std::make_pair(
			mark::rotate(raw.first, rotation) + pos,
			mark::rotate(raw.second, rotation) + pos
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

auto mark::module::base::neighbours() -> std::vector<std::reference_wrapper<mark::module::base>> {
	return parent().get_attached(*this);
}

auto mark::module::base::grid_pos() const noexcept -> mark::vector<int> {
	return mark::vector<int>(m_grid_pos);
}

bool mark::module::base::damage(const mark::idamageable::info & attr) {
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

auto mark::module::base::dead() const -> bool {
	return m_cur_health <= 0.f;
}

void mark::module::base::on_death(mark::tick_context& context) {
	mark::tick_context::spray_info spray;
	spray.image = parent().world().resource_manager().image("explosion.png");
	spray.pos = pos();
	spray.velocity(75.f, 150.f);
	spray.lifespan(0.3f);
	spray.diameter(24.f);
	spray.count = 20;
	context.render(spray);
}

auto mark::module::base::global_modifiers() const->mark::module::modifiers
{
	return mark::module::modifiers();
}

auto mark::module::base::parent() const -> const mark::unit::modular& {
	if (m_parent) {
		return *m_parent;
	} else {
		throw mark::exception("NO_PARENT");
	}
}

auto mark::module::base::parent() -> mark::unit::modular&{
	return const_cast<mark::unit::modular&>(static_cast<const mark::module::base*>(this)->parent());
}

auto mark::module::base::heat_color() const -> sf::Color {
	const auto intensity = static_cast<uint8_t>((1.f - m_cur_heat / max_heat) * 255.f);
	return { 255, intensity, intensity, 255 };
}

mark::module::base::base(mark::resource::manager& rm, const YAML::Node& node):
	m_cur_health(node["cur_health"].as<float>()),
	m_max_health(node["max_health"].as<float>()),
	m_stunned(node["stunned"].as<float>()),
	m_cur_heat(node["cur_heat"].as<float>()),
	m_grid_pos(node["grid_pos"]["x"].as<int>(), node["grid_pos"]["y"].as<int>()),
	m_size(node["size"]["x"].as<unsigned>(), node["size"]["y"].as<unsigned>()) { }

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
}

auto mark::module::base::pos() const -> mark::vector<double> {
	const auto pos = (mark::vector<float>(grid_pos()) + mark::vector<float>(this->size()) / 2.f)
		* static_cast<float>(mark::module::size);
	return parent().pos() + mark::vector<double>(rotate(pos, parent().rotation()));
}

auto mark::module::base::thumbnail() const -> std::shared_ptr<const mark::resource::image> {
	return m_thumbnail;
}

auto mark::module::base::size() const -> mark::vector<unsigned> {
	return m_size;
}
