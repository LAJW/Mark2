#pragma once
#include <command.h>
#include <interface/damageable.h>
#include <interface/item.h>
#include <interface/world_object.h>
#include <stdafx.h>

namespace mark {

class property_manager;

namespace module {

// grid size - width/height of an 1x1 module
static constexpr float size = 16.f;

// Maximum module width and height
const unsigned max_dimension = 4;

// stats altering behavior of parent/adjacent modules
struct modifiers
{
	float velocity = 0.f;
	float mass = 0.f;
	float armor = 0.f;
};

auto deserialize(resource::manager&, random& random, const YAML::Node&)
	-> module::base_ptr;

enum class reserved_kind
{
	none,
	front,
	back
};

// Part of the base modifiable by the modular and cargo
class base_ref
{
public:
	friend unit::modular;
	friend module::cargo;

	auto grid_pos() const noexcept -> vi32;
	auto parent() const -> const unit::modular&;

protected:
	base_ref() = default;
	base_ref(const YAML::Node& node);
	auto parent() -> unit::modular&;
	void serialize(YAML::Emitter& out) const;
	// Check if has parent
	// Only to be used by the description function. All other functions assume
	// that they won't be called if the module is attached to the modular or is
	// being serialized, so no interaction with the base is allowed.
	auto has_parent() const -> bool;
	~base_ref() = default;

private:
	virtual void update(update_context& context) = 0;
	unit::modular* m_parent = nullptr;
	vector<int8_t> m_grid_pos;
};

class base
	: public base_ref
	, public interface::damageable
	, public interface::world_object
	, public interface::item
{
public:
	// serialize module::base properties
	virtual void serialize(YAML::Emitter&) const;

	static constexpr auto max_heat = 100.f;

	virtual ~base();

	// Module's position in the world
	auto pos() const -> vd override;

	auto world() const -> const mark::world& override;

	// Module's image in the inventory
	auto thumbnail() const -> resource::image_ptr;

	// Size in grid units
	auto size() const -> vu32;

	// Describes whether the module is dead or not
	virtual auto dead() const -> bool;

	// Can module be detached from the vessel
	virtual auto detachable() const -> bool { return true; }

	virtual void command(const command::any&);

	// Find collision point, return pointer to damaged module
	virtual auto collide(const segment_t&)
		-> std::optional<std::pair<ref<interface::damageable>, vd>>;

	// UI text describing module's properties
	virtual auto describe() const -> std::string;

	// Obtain energy from the module
	virtual auto harvest_energy(double) -> float { return 0.f; }

	// Current / Maximum energy stored in the module
	virtual auto energy_ratio() const -> float { return 0.f; }

	// Neighbour modules
	auto neighbors() -> std::vector<std::pair<ref<module::base>, unsigned>>;

	// Default damage handling
	auto damage(const interface::damageable::info& attr) -> bool override;

	// called on module's death
	virtual void on_death(update_context& context);

	// get modifiers for parent modular ship
	virtual auto global_modifiers() const -> module::modifiers;

	// get modifiers for surrounding modules
	virtual auto local_modifiers() const -> module::modifiers;

	// Specifies whether space around the module should be reserved
	// For example behind engines and in front of locked turrets
	virtual auto reserved() const noexcept -> reserved_kind;

	// Specifiy whether module has no abilities
	virtual auto passive() const noexcept -> bool = 0;

	// Get module's cur health
	auto cur_health() const -> float;

	// Get module's cur health
	auto max_health() const -> float;

	auto needs_healing() const -> bool;

	// Replenish module's health
	void heal(float amount);

	// Get team ID, to satisfy world object interface
	auto team() const -> size_t override final;

	// Randomise all random properties
	[[nodiscard]] auto randomise(
		const std::unordered_map<std::string, YAML::Node>& blueprints,
		random& random) -> std::error_code;

	// No-op - modules are not stackable
	void stack(interface::item_ptr&) override final {}

	auto can_stack(const interface::item&) const -> bool override final
	{
		return false;
	}

	auto quantity() const -> size_t override { return 1; }

protected:
	base(resource::manager&, random& random, const YAML::Node&);

	void update(update_context& context) override;

	auto heat_color() const -> sf::Color;

	// Get parent rotation (reduces compile time)
	float parent_rotation() const;

	auto world() noexcept -> mark::world&;

	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);

	/// Calculate amount of damage to deal. Helper implementation function to
	/// share damage calculation logic between base, shield and alike
	struct damage_result
	{
		float health;
		float heat;
		bool stun;
	};
	auto damage_impl(const interface::damageable::info& attr) const
		-> damage_result;

protected:
	float m_cur_health = 100.f;
	float m_max_health = 100.f;
	float m_armor = 0.f;
	/// Resistance to antimatter damage. 0 represents 0% resistance 1 - 100%
	/// Values can range from -2 to .75
	float m_antimatter_resistance = 0.f;
	/// Resistance to heat
	float m_heat_resistance = 0.f;
	/// Resistance to energy damage. Minimum 0 maximum 1
	float m_energy_resistance = 0.f;
	float m_stunned = 0.f;
	float m_cur_heat = 0.f;
	bool m_burning = false;

private:
	resource::image_ptr m_thumbnail;
	resource::image_ptr m_im_shadow;
	vu32 m_size = { 2, 2 };
	float m_stun_lfo = 0.f;
	std::string m_blueprint_id;
};
} // namespace module
} // namespace mark
