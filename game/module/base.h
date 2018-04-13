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
	float mass = 1.f;
};

auto deserialize(resource::manager&, const YAML::Node&)
	-> std::unique_ptr<interface::item>;

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

	auto grid_pos() const noexcept -> vector<int>;

protected:
	base_ref() = default;
	base_ref(const YAML::Node& node);
	auto parent() const -> const unit::modular&;
	auto parent() -> unit::modular&;
	void serialize(YAML::Emitter& out) const;
	// Position on the grid
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
	auto pos() const -> vector<double> override;

	auto world() const -> const mark::world& override;

	// Module's image in the inventory
	auto thumbnail() const -> std::shared_ptr<const resource::image>;

	// Size in grid units
	auto size() const -> vector<unsigned>;

	// Describes whether the module is dead or not
	virtual auto dead() const -> bool;

	// Can module be detached from the vessel
	virtual auto detachable() const -> bool { return true; }

	virtual void command(const command::any&);

	// Find collision point, return pointer to damaged module
	virtual auto collide(const segment_t&) -> std::optional<std::pair<
		std::reference_wrapper<interface::damageable>,
		vector<double>>>;

	// UI text describing module's properties
	virtual auto describe() const -> std::string = 0;

	// Obtain energy from the module
	virtual auto harvest_energy(double) -> float { return 0.f; }

	// Current / Maximum energy stored in the module
	virtual auto energy_ratio() const -> float { return 0.f; }

	// Neighbour modules
	auto neighbours() -> std::vector<
		std::pair<std::reference_wrapper<module::base>, unsigned>>;

	// Default damage handling
	auto damage(const interface::damageable::info& attr) -> bool override;

	// called on module's death
	virtual void on_death(update_context& context);

	// get modifiers for parent modular ship
	virtual auto global_modifiers() const -> module::modifiers;

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
		resource::manager& resource_manager) -> std::error_code;

	// No-op - modules are not stackable
	void stack(std::unique_ptr<interface::item>&) override final {}

	auto can_stack(const interface::item&) const -> bool override final
	{
		return false;
	}

protected:
	base(resource::manager&, const YAML::Node&);

	void update(update_context& context) override;

	auto heat_color() const -> sf::Color;

	// Get parent rotation (reduces compile time)
	float parent_rotation() const;

	auto world() noexcept -> mark::world&;

	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	virtual void bind(mark::property_manager&);

protected:
	float m_cur_health = 100.f;
	float m_max_health = 100.f;
	float m_stunned = 0.f;
	float m_cur_heat = 0.f;

private:
	std::shared_ptr<const resource::image> m_thumbnail;
	std::shared_ptr<const resource::image> m_im_shadow;
	vector<unsigned> m_size = { 2, 2 };
	float m_stun_lfo = 0.f;
	std::string m_blueprint_id;
};
} // namespace module
} // namespace mark
