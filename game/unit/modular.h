#pragma once
#include "mobile.h"
#include <array2d.h>
#include <command.h>
#include <interface/container.h>
#include <interface/has_bindings.h>
#include <stdafx.h>

namespace mark {
namespace unit {

class modular final
	: public unit::mobile
	, public interface::container
	, public interface::has_bindings
{
public:
	using find_result = std::vector<ref<module::base>>;
	using const_find_result = std::vector<cref<module::base>>;

	static constexpr const char* type_name = "unit_modular";
	static constexpr unsigned max_size = 40;

	modular(mark::world& world, const YAML::Node&);
	struct info : mobile::info
	{
		float rotation = 0.f;
	};
	modular(info info);
	~modular();
	void command(const command::any& command) override;
	[[nodiscard]] auto attach(vi32 pos, interface::item_ptr& item)
		-> std::error_code override;
	auto can_attach(vi32 pos, const interface::item& item) const
		-> bool override;
	auto at(vi32 pos) noexcept -> interface::item* override;
	auto at(vi32 pos) const noexcept -> const interface::item* override;
	auto pos_at(vi32 pos) const noexcept -> std::optional<vi32> override;
	auto module_at(vi32 pos) noexcept -> module::base*;
	auto module_at(vi32 pos) const noexcept -> const module::base*;
	auto detach(vi32 pos) -> interface::item_ptr override;
	auto can_detach(vi32 pos) const noexcept -> bool;

	auto neighbors_of(const module::base&)
		-> std::vector<std::pair<ref<module::base>, unsigned>>;
	auto neighbors_of(const module::base&) const
		-> std::vector<std::pair<cref<module::base>, unsigned>>;

	auto rotation() const { return m_rotation; }
	auto dead() const -> bool override;
	auto containers() -> std::vector<ref<module::cargo>>;
	auto containers() const -> std::vector<cref<module::cargo>>;
	auto damage(const interface::damageable::info&) -> bool override;
	void knockback(
		std::unordered_set<not_null<interface::damageable*>>& knocked,
		float angle,
		double distance);
	auto collide(const segment_t&)
		-> std::optional<std::pair<ref<interface::damageable>, vd>> override;
	auto collide(vd center, double radius)
		-> std::vector<ref<interface::damageable>> override;
	// bind module at position to command
	void toggle_bind(int8_t, vi32 pos);
	auto bindings() const -> bindings_t override;
	// Get vector of commands controlling this module
	auto binding(vi32 pos) const -> std::vector<int8_t>;
	void serialize(YAML::Emitter&) const override;
	// is module resting on the landing pad
	auto landed() const noexcept -> bool;
	void ai(bool);
	// Set velocity (and acceleration) of this vessel to zero
	auto radius() const -> double override;
	// Get a reference to modular's targeting system
	auto targeting_system() const -> const mark::targeting_system&;
	auto targeting_system() -> mark::targeting_system&;

private:
	void update(update_context& context) override;
	void on_death(update_context& context) override;

	// Attach without checking structure of the ship
	[[nodiscard]] auto p_attach(vi32 pos, module::base_ptr& module)
		-> std::error_code;
	// Check whether module can be attached without checking structure of the
	// ship
	auto p_can_attach(const module::base& module, vi32 pos) const -> bool;
	void remove_dead(update_context&);
	void pick_up();
	// Remove module from module bindings
	void unbind(const module::base& module);
	auto modifiers() const -> module::modifiers;
	void update_modules(update_context& context);
	// Generate a list of commands for self
	std::vector<command::any> update_ai() const;
	auto p_connected_to_core(
		const module::base&,
		std::pair<vi8, vi8> area_to_ignore) const -> bool;

	struct grid_element
	{
		module::base* module = nullptr;
		bool reserved = false;
	};
	auto p_at(vector<int8_t> pos) noexcept -> grid_element&;
	auto p_at(vector<int8_t> pos) const noexcept -> const grid_element&;

	std::vector<module::base_ptr> m_modules;
	array2d<grid_element, max_size, max_size> m_grid;
	unique_ptr<mark::targeting_system> m_targeting_system;
	module::core* m_core = nullptr;
	float m_rotation = 0.f;
	bool m_ai = false;
	vd m_lookat;
	std::unordered_multimap<int8_t, ref<module::base>> m_bindings;
	double m_velocity = 0;
	double m_radius = 0.;
	std::vector<vd> m_knockback_path;
	double m_initial_knockback_path_length;
};

// Drop module into the modular's containers if possible
[[nodiscard]] auto push(modular& modular, interface::item_ptr&)
	-> std::error_code;
;
} // namespace unit
} // namespace mark
