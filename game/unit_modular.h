#pragma once
#include "stdafx.h"
#include "unit_damageable.h"
#include "command.h"
#include "interface_has_bindings.h"
#include "interface_container.h"

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
	public unit::damageable,
	public interface::container,
	public interface::has_bindings {
public:
	using find_result = std::vector<std::reference_wrapper<mark::module::base>>;
	using const_find_result = std::vector<std::reference_wrapper<const mark::module::base>>;

	static constexpr const char* type_name = "unit_modular";
	static constexpr unsigned max_size = 40;

	modular(mark::world& world, const YAML::Node&);
	modular(mark::world& world, mark::vector<double> pos = { 0, 0 }, float rotation = 0.0f);
	void command(const mark::command& command) override;
	[[nodiscard]] auto attach(
		const vector<int>& pos, std::unique_ptr<module::base>& module)
		-> error::code override;
	auto can_attach(
		const vector<int>& pos, const module::base& module) const
		-> bool override;
	auto at(const vector<int>& pos) noexcept
		-> mark::module::base* override;
	auto at(const vector<int>& pos) const noexcept
		-> const mark::module::base* override;
	auto detach(const vector<int>& pos)
		-> std::unique_ptr<mark::module::base> override;

	// TODO: Rename to neighbour_of
	auto attached(const mark::module::base&) ->
		std::vector<std::pair<std::reference_wrapper<mark::module::base>, unsigned>>;
	auto attached(const mark::module::base&) const ->
		std::vector<std::pair<std::reference_wrapper<const mark::module::base>, unsigned>>;

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
	auto damage(const interface::damageable::info&) -> bool override;
	auto collide(const mark::segment_t&) ->
		std::pair<interface::damageable*, mark::vector<double>> override;
	auto collide(mark::vector<double> center, float radius) ->
		std::vector<std::reference_wrapper<interface::damageable>> override;
	auto lookat() const noexcept -> mark::vector<double>;
	// bind module at position to command
	void toggle_bind(enum class mark::command::type, mark::vector<int> pos);
	auto bindings() const -> bindings_t;
	// Get vector of commands controlling this module
	auto binding(mark::vector<int> pos) const
		-> std::vector<enum class mark::command::type>;
	void serialize(YAML::Emitter&) const;
	// is module resting on the landing pad
	auto landed() const noexcept -> bool;
	auto p_reserved(mark::vector<int8_t> pos) const noexcept -> bool;
	void ai(bool);
private:
	// Attach without checking structure of the ship
	[[nodiscard]] auto p_attach(
		const vector<int>& pos, std::unique_ptr<module::base>& module) -> error::code;
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