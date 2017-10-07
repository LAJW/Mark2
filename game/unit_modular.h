#pragma once
#include "stdafx.h"
#include "unit_damageable.h"
#include "command.h"
#include "ihas_bindings.h"

namespace mark {

namespace resource {
class image;
}

namespace module {
class base;
class core;
class cargo;
struct modifiers;
};
class world;

namespace unit {
class modular final:
	public mark::unit::damageable,
	public mark::ihas_bindings {
public:
	using find_result = std::vector<std::reference_wrapper<mark::module::base>>;
	using const_find_result = std::vector<std::reference_wrapper<const mark::module::base>>;

	static constexpr const char* type_name = "unit_modular";
	static constexpr unsigned max_size = 40;

	modular(mark::world& world, const YAML::Node&);
	modular(mark::world& world, mark::vector<double> pos = { 0, 0 }, float rotation = 0.0f);
	void command(const mark::command& command) override;
	// Attaches module to the modular
	// Throws mark::exception if object cannot be attached
	// Doesn't destroy passed in module on error
	void attach(std::unique_ptr<module::base>& module, mark::vector<int> pos);
	auto can_attach(const module::base& module, mark::vector<int> pos) const -> bool;
	auto attached(const mark::module::base&) ->
		std::vector<std::pair<std::reference_wrapper<mark::module::base>, unsigned>>;
	auto attached(const mark::module::base&) const ->
		std::vector<std::pair<std::reference_wrapper<const mark::module::base>, unsigned>>;
	auto core() -> mark::module::core&;
	auto core() const -> const mark::module::core&;

	void tick(mark::tick_context& context) override;
	inline auto rotation() const { return m_rotation; }
	auto dead() const -> bool override;
	void on_death(mark::tick_context& context) override;
	inline auto world() -> mark::world& { return m_world; }
	void activate(const std::shared_ptr<mark::unit::base>& by) override;
	auto containers() ->
		std::vector<std::reference_wrapper<mark::module::cargo>>;
	auto containers() const ->
		std::vector<std::reference_wrapper<const mark::module::cargo>>;
	auto detach(mark::vector<int> pos)->std::unique_ptr<mark::module::base>;
	auto module(mark::vector<int> pos) const -> const mark::module::base*;
	auto module(mark::vector<int> pos)->mark::module::base*;
	auto damage(const mark::idamageable::info&) -> bool override;
	auto collide(const mark::segment_t&) ->
		std::pair<mark::idamageable*, mark::vector<double>> override;
	auto collide(mark::vector<double> center, float radius) ->
		std::vector<std::reference_wrapper<mark::idamageable>> override;
	auto lookat() const noexcept -> mark::vector<double>;
	// bind module at position to command
	void toggle_bind(enum class mark::command::type, mark::vector<int> pos);
	auto bindings() const -> bindings_t;
	void serialize(YAML::Emitter&) const;
	// get module at position. Returns null if out of bounds or no module present.
	auto at(mark::vector<int8_t> pos) noexcept -> mark::module::base*;
	auto at(mark::vector<int8_t> pos) const noexcept-> const mark::module::base*;
	// is module resting on the landing pad
	auto landed() const noexcept -> bool;
	auto p_reserved(mark::vector<int8_t> pos) const noexcept -> bool;
private:
	// Attach without checking structure of the ship
	void p_attach(std::unique_ptr<module::base> module, mark::vector<int> pos);
	// Check whether module can be attached without checking structure of the ship
	auto p_can_attach(const module::base& module, mark::vector<int> pos) const -> bool;
	void remove_dead(mark::tick_context&);
	void pick_up(mark::tick_context&);
	// Remove module from module bindings
	void unbind(const mark::module::base& module);
	auto modifiers() const -> mark::module::modifiers;
	void tick_modules(mark::tick_context& context);
	void tick_movement(double dt, const mark::module::modifiers& mods);
	void tick_ai();
	auto p_connected_to_core(const mark::module::base&) const -> bool;
	// get pointer in reference to the center of the grid
	auto p_at(mark::vector<int8_t> pos) noexcept -> mark::module::base*&;
	auto p_at(mark::vector<int8_t> pos) const noexcept -> const mark::module::base*;
	auto p_reserved(mark::vector<int8_t> pos) noexcept -> bool&;
	// get pointer in reference to the top left corner of the grid
	auto p_grid(mark::vector<uint8_t> pos) noexcept -> std::pair<mark::module::base*, bool>&;
	auto p_grid(mark::vector<uint8_t> pos) const noexcept -> const std::pair<mark::module::base*, bool>&;

	std::vector<std::unique_ptr<mark::module::base>> m_modules;
	std::array<std::pair<mark::module::base*, bool>, max_size * max_size>
		m_grid = { std::pair<mark::module::base*, bool>(nullptr, false) };
	mark::module::core* m_core = nullptr;
	float m_rotation = 0.f;
	mark::vector<double> m_moveto;
	mark::vector<double> m_lookat;
	bool m_ai = false;
	bool m_move = false;
	std::unordered_multimap<enum class mark::command::type, std::reference_wrapper<mark::module::base>> m_bindings;
	std::vector<mark::vector<double>> m_path_cache; // path cache
	float m_path_age = 0.f;
	double m_velocity = 0;
};
}
}