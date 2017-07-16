#pragma once
#include "stdafx.h"
#include "unit_base.h"
#include "command.h"

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
		class landing_pad;
		class modular final : public base {
		public:
			using find_result = std::vector<std::reference_wrapper<mark::module::base>>;
			using const_find_result = std::vector<std::reference_wrapper<const mark::module::base>>;

			static constexpr const char* type_name = "unit_modular";
			static constexpr unsigned max_size = 40;

			struct bound_status {
				std::shared_ptr<const mark::resource::image> thumbnail;
				uint16_t total = 0;
				uint16_t ready = 0;
				float cooldown = 0.f;
			};
		public:
			modular(mark::world& world, const YAML::Node&);
			modular(mark::world& world, mark::vector<double> pos = { 0, 0 }, float rotation = 0.0f);
			void command(const mark::command& command) override;
			void attach(std::unique_ptr<module::base> module, mark::vector<int> pos);
			// deprecated
			auto can_attach(const std::unique_ptr<module::base>& module, mark::vector<int> pos) const -> bool;
			// deprecated
			auto get_attached(const mark::module::base&) ->
				std::vector<std::reference_wrapper<mark::module::base>>;
			auto attached(const mark::module::base&) ->
				std::vector<std::reference_wrapper<mark::module::base>>;
			auto attached(const mark::module::base&) const ->
				std::vector<std::reference_wrapper<const mark::module::base>>;
			// deprecated
			auto get_core()->mark::module::core&;
			auto core() -> mark::module::core&;
			auto core() const -> const mark::module::core&;

			void tick(mark::tick_context& context) override;
			inline auto rotation() const { return m_rotation; }
			auto dead() const -> bool override;
			void on_death(mark::tick_context& context) override;
			inline auto world() -> mark::world& { return m_world; }
			auto invincible() const -> bool override;
			void activate(const std::shared_ptr<mark::unit::base>& by) override;
			auto containers()->std::vector<std::reference_wrapper<mark::module::cargo>>;
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
			auto bound_status() const -> std::array<bound_status, 11>;
			void serialize(YAML::Emitter&) const;
			// get module at position. Returns null if out of bounds or no module present.
			auto at(mark::vector<int8_t> pos) noexcept -> mark::module::base*;
			auto at(mark::vector<int8_t> pos) const noexcept-> const mark::module::base*;
		private:
			// attach without performing integrity check
			void p_attach(std::unique_ptr<module::base> module, mark::vector<int> pos);
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
			// get pointer in reference to the top left corner of the grid
			auto p_grid(mark::vector<uint8_t> pos) noexcept -> mark::module::base*&;
			auto p_grid(mark::vector<uint8_t> pos) const noexcept -> const mark::module::base*;


			std::vector<std::unique_ptr<mark::module::base>> m_modules;
			std::array<mark::module::base*, max_size * max_size> m_grid = { nullptr };
			mark::module::core* m_core = nullptr;
			float m_rotation = 0.f;
			mark::vector<double> m_moveto;
			mark::vector<double> m_lookat;
			bool m_ai = false;
			bool m_move = false;
			std::unordered_multimap<enum class mark::command::type, std::reference_wrapper<mark::module::base>> m_bindings;
			std::vector<mark::vector<double>> m_path_cache; // path cache
			float m_path_age = 0.f;
		};
	}
}