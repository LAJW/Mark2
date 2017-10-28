#pragma once
#include "stdafx.h"
#include "interface_damageable.h"

namespace sf {
	class Color;
}

namespace mark {
namespace resource {
class image;
class manager;
}
struct sprite;
namespace unit {
class modular;
}
struct tick_context;
class world;

namespace module {
// grid size - width/height of an 1x1 module
static constexpr float size = 16.f;

// Maximum module width and height
const unsigned max_dimension = 4;

// stats altering behavior of parent/adjacent modules
struct modifiers {
	float velocity = 0.f;
};

class base;

auto deserialize(resource::manager&, const YAML::Node&) ->
	std::unique_ptr<module::base>;

enum class reserved_type {
	none,
	front,
	back
};

class cargo;

// Part of the base modifiable by the modular and cargo
class base_ref {
public:
	friend unit::modular;
	friend module::cargo;

	auto grid_pos() const noexcept -> vector<int>;
protected:
	base_ref() = default;
	base_ref(const YAML::Node& node);
	auto parent() const -> const unit::modular&;
	auto parent() -> unit::modular&;
	void serialize(YAML::Emitter& out) const;
	// Position on the grid
	~base_ref()=default;
private:
	unit::modular* m_parent = nullptr;
	vector<int8_t> m_grid_pos;
};

class base:
	public base_ref,
	public interface::damageable {
public:
	virtual void serialize(YAML::Emitter&) const;

	static constexpr auto max_heat = 100.f;

	virtual ~base();

	virtual void tick(tick_context& context);

	// Module's position in the world
	auto pos() const -> vector<double>;

	// Module's image in the inventory
	auto thumbnail() const -> std::shared_ptr<const resource::image>;

	// Size in grid units
	auto size() const -> vector<unsigned>;

	// Describes whether the module is dead or not
	virtual auto dead() const -> bool;

	// Can module be detached from the vessel
	virtual auto detachable() const -> bool { return true; }

	// Target/look at specific position in the world
	virtual void target(vector<double> pos);

	// Select target to shoot at
	virtual void shoot(vector<double> pos, bool release);

	// Add target to shooting queue
	virtual void queue(vector<double> pos, bool release);

	// Find collision point, return pointer to damaged module
	virtual auto collide(const segment_t&) ->
		std::optional<std::pair<
			std::reference_wrapper<interface::damageable>,
			vector<double>>>;

	// UI text describing module's properties
	virtual auto describe() const -> std::string = 0;

	// Obtain energy from the module
	virtual auto harvest_energy() -> float { return 0.f; }

	// Current / Maximum energy stored in the module
	virtual auto energy_ratio() const -> float { return 0.f; }

	// Neighbour modules
	auto neighbours() -> std::vector<std::pair<std::reference_wrapper<module::base>, unsigned>>;

	// Default damage handling
	auto damage(const interface::damageable::info& attr) -> bool override;

	// called on module's death
	virtual void on_death(tick_context& context);

	// get modifiers for parent modular ship
	virtual auto global_modifiers() const -> module::modifiers;

	// Specifies whether space around the module should be reserved
	// For example behind engines and in front of locked turrets
	virtual auto reserved() const noexcept -> reserved_type;

	// Specifiy whether module has no abilities
	virtual auto passive() const noexcept -> bool = 0;

protected:
	base(resource::manager&, const YAML::Node&);
	base(vector<unsigned> size,
		const std::shared_ptr<const resource::image>& thumbnail);

	auto heat_color() const -> sf::Color;
	// serialize module::base properties, call only from module serializers

	// Get parent rotation (reduces compile time)
	float parent_rotation() const;

	auto world() noexcept -> mark::world&;

	float m_cur_health = 100.f;
	float m_max_health = 100.f;
	float m_stunned = 0.f;
	float m_cur_heat = 0.f;
private:
	std::shared_ptr<const resource::image> m_thumbnail;
	const vector<unsigned> m_size;
	float m_stun_lfo;
};
}
}
