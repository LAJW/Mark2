#pragma once
#include "mobile.h"
#include <array2d.h>
#include <command.h>
#include <interface/container.h>
#include <interface/has_bindings.h>
#include <ref.h>
#include <stdafx.h>

namespace mark {
namespace unit {

class modular final
	: public unit::mobile
	, public interface::container
	, public interface::has_bindings
{
	// Types and static properties

public:
	using find_result = std::vector<std::reference_wrapper<module::base>>;
	using const_find_result =
		std::vector<std::reference_wrapper<const module::base>>;

	/// Type of modular's constructor parameter
	struct info final : mobile::info
	{
		/// Angular orientation of the ship
		float rotation = 0.f;
	};

	static constexpr const char* type_name = "unit_modular";
	static constexpr unsigned max_size = 40;

private:
	struct grid_element final
	{
		optional<module::base&> module;
		std::unordered_set<not_null<const module::base*>> reserved;
	};

public:
	modular(ref<mark::world> world, ref<random> random, const YAML::Node&);
	explicit modular(info info);
	~modular();

	// Inherited overriden functions

	void command(const command::any& command) override;
	[[nodiscard]] auto attach(vi32 pos, interface::item_ptr&& item)
		-> std::error_code override;
	[[nodiscard]] auto can_attach(vi32 pos, const interface::item& item) const
		-> bool override;
	[[nodiscard]] auto at(vi32 pos) noexcept
		-> optional<interface::item&> override;
	[[nodiscard]] auto at(vi32 pos) const noexcept
		-> optional<const interface::item&> override;
	[[nodiscard]] auto pos_at(vi32 pos) const noexcept
		-> std::optional<vi32> override;
	[[nodiscard]] auto detach(vi32 pos) -> interface::item_ptr override;
	[[nodiscard]] auto can_detach(vi32 pos) const noexcept -> bool override;
	[[nodiscard]] auto dead() const -> bool override;
	[[nodiscard]] auto damage(const interface::damageable::info&)
		-> bool override;
	[[nodiscard]] auto collide(const segment_t&)
		-> std::optional<std::pair<interface::damageable&, vd>> override;
	[[nodiscard]] auto collide(vd center, double radius)
		-> std::vector<std::reference_wrapper<interface::damageable>> override;
	[[nodiscard]] auto bindings() const -> bindings_t override;
	void serialize(YAML::Emitter&) const override;
	// Set velocity (and acceleration) of this vessel to zero
	[[nodiscard]] auto radius() const -> double override;

private:
	void update(update_context& context) override;
	void on_death(update_context& context) override;

	// Non-virtual non-static public functions
public:
	[[nodiscard]] auto module_at(vi32 pos) noexcept -> optional<module::base&>;
	[[nodiscard]] auto module_at(vi32 pos) const noexcept
		-> optional<const module::base&>;
	[[nodiscard]] auto rotation() const { return m_rotation; }

	[[nodiscard]] auto containers()
		-> std::vector<std::reference_wrapper<module::cargo>>;
	[[nodiscard]] auto containers() const
		-> std::vector<std::reference_wrapper<const module::cargo>>;
	void knockback(
		std::unordered_set<not_null<interface::damageable*>>& knocked,
		float angle,
		double distance);
	// bind module at position to command
	void toggle_bind(int8_t, vi32 pos);
	// Get vector of commands controlling this module
	[[nodiscard]] auto binding(vi32 pos) const -> std::vector<int8_t>;
	void ai(bool);
	// Get a reference to modular's targeting system
	[[nodiscard]] auto targeting_system() const
		-> const mark::targeting_system&;
	[[nodiscard]] auto targeting_system() -> mark::targeting_system&;

private:
	// Attach without checking structure of the ship
	[[nodiscard]] auto p_attach(vi32 pos, module::base_ptr&& module)
		-> std::error_code;
	// Check whether module can be attached without checking structure of the
	// ship
	[[nodiscard]] auto p_can_attach(const module::base& module, vi32 pos) const
		-> bool;
	void remove_dead(ref<update_context>);
	void pick_up();
	// Remove module from module bindings
	void unbind(const module::base& module);
	[[nodiscard]] auto modifiers() const -> module::modifiers;
	void update_modules(ref<update_context> context);
	// Generate a list of commands for self
	[[nodiscard]] auto update_ai() const -> std::vector<command::any>;
	[[nodiscard]] auto p_connected_to_core(
		const module::base&,
		std::pair<vi8, vi8> area_to_ignore) const -> bool;

	/// Get module at position without checking bounds
	[[nodiscard]] auto p_at(vector<int8_t> pos) noexcept -> grid_element&;
	[[nodiscard]] auto p_at(vector<int8_t> pos) const noexcept
		-> const grid_element&;

	/// Generalized implementation function of module_at
	template <
		typename T,
		typename U = add_const_if_t<module::base, std::is_const_v<T>>>
	[[nodiscard]] static optional<U&> module_at_impl(T& self, vi32 pos);

	std::vector<module::base_ptr> m_modules;
	array2d<grid_element, max_size, max_size> m_grid;
	unique_ptr<mark::targeting_system> m_targeting_system;
	module::core* m_core = nullptr;
	float m_rotation = 0.f;
	bool m_ai = false;
	vd m_lookat;
	std::unordered_multimap<int8_t, std::reference_wrapper<module::base>>
		m_bindings;
	double m_velocity = 0;
	double m_radius = 0.;
	std::vector<vd> m_knockback_path;
	double m_initial_knockback_path_length;
};

// Drop module into the modular's containers if possible
[[nodiscard]] auto push(modular& modular, interface::item_ptr &&)
	-> std::error_code;

} // namespace unit

[[nodiscard]] auto neighbors_of(module::base&)
	-> std::vector<std::pair<module::base&, unsigned>>;
[[nodiscard]] auto neighbors_of(const module::base&)
	-> std::vector<std::pair<const module::base&, unsigned>>;

} // namespace mark
