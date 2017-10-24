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
	using find_result = std::vector<std::reference_wrapper<module::base>>;
	using const_find_result = std::vector<std::reference_wrapper<const module::base>>;

	static constexpr const char* type_name = "unit_modular";
	static constexpr unsigned max_size = 40;

	modular(mark::world& world, const YAML::Node&);
	modular(mark::world& world, vector<double> pos = { 0, 0 }, float rotation = 0.0f);
	void command(const mark::command& command) override;
	[[nodiscard]] auto attach(
		const vector<int>& pos, std::unique_ptr<module::base>& module)
		-> std::error_code override;
	auto can_attach(
		const vector<int>& pos, const module::base& module) const
		-> bool override;
	auto at(const vector<int>& pos) noexcept
		-> module::base* override;
	auto at(const vector<int>& pos) const noexcept
		-> const module::base* override;
	auto detach(const vector<int>& pos)
		-> std::unique_ptr<module::base> override;

	// TODO: Rename to neighbour_of
	auto attached(const module::base&) ->
		std::vector<std::pair<std::reference_wrapper<module::base>, unsigned>>;
	auto attached(const module::base&) const ->
		std::vector<std::pair<std::reference_wrapper<const module::base>, unsigned>>;

	void tick(tick_context& context) override;
	inline auto rotation() const { return m_rotation; }
	auto dead() const -> bool override;
	void on_death(tick_context& context) override;
	void activate(const std::shared_ptr<unit::base>& by) override;
	auto containers() ->
		std::vector<std::reference_wrapper<module::cargo>>;
	auto containers() const ->
		std::vector<std::reference_wrapper<const module::cargo>>;
	auto damage(const interface::damageable::info&) -> bool override;
	auto collide(const segment_t&) ->
		std::pair<interface::damageable*, vector<double>> override;
	auto collide(vector<double> center, float radius) ->
		std::vector<std::reference_wrapper<interface::damageable>> override;
	auto lookat() const noexcept -> vector<double>;
	// bind module at position to command
	void toggle_bind(enum class command::type, vector<int> pos);
	auto bindings() const -> bindings_t;
	// Get vector of commands controlling this module
	auto binding(vector<int> pos) const
		-> std::vector<enum class command::type>;
	void serialize(YAML::Emitter&) const;
	// is module resting on the landing pad
	auto landed() const noexcept -> bool;
	auto p_reserved(vector<int8_t> pos) const noexcept -> bool;
	void ai(bool);
private:
	// Attach without checking structure of the ship
	[[nodiscard]] auto p_attach(
		const vector<int>& pos, std::unique_ptr<module::base>& module)
		-> std::error_code;
	// Check whether module can be attached without checking structure of the ship
	auto p_can_attach(const module::base& module, vector<int> pos) const -> bool;
	void remove_dead(tick_context&);
	void pick_up(tick_context&);
	// Remove module from module bindings
	void unbind(const module::base& module);
	auto modifiers() const -> module::modifiers;
	void tick_modules(tick_context& context);
	void tick_movement(double dt, const module::modifiers& mods);
	void tick_ai();
	auto p_connected_to_core(const module::base&) const -> bool;
	// get pointer in reference to the center of the grid
	auto p_at(vector<int8_t> pos) noexcept -> module::base*&;
	auto p_at(vector<int8_t> pos) const noexcept -> const module::base*;
	auto p_reserved(vector<int8_t> pos) noexcept -> bool&;
	// get pointer in reference to the top left corner of the grid
	auto p_grid(vector<uint8_t> pos) noexcept -> std::pair<module::base*, bool>&;
	auto p_grid(vector<uint8_t> pos) const noexcept -> const std::pair<module::base*, bool>&;

	std::vector<std::unique_ptr<module::base>> m_modules;
	std::array<std::pair<module::base*, bool>, max_size * max_size>
		m_grid = { std::pair<module::base*, bool>(nullptr, false) };
	module::core* m_core = nullptr;
	float m_rotation = 0.f;
	vector<double> m_moveto;
	vector<double> m_lookat;
	bool m_ai = false;
	std::unordered_multimap<enum class command::type, std::reference_wrapper<module::base>> m_bindings;
	std::vector<vector<double>> m_path_cache; // path cache
	float m_path_age = 0.f;
	double m_velocity = 0;
};
}
}