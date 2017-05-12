#pragma once
#include <memory>
#include <vector>
#include <functional>
#include "unit_base.h"

namespace mark {

	namespace module {
		class base;
		class core;
		class cargo;
	};
	class world;

	namespace unit {
		class landing_pad;
		class modular final : public base {
		public:
			using find_result = std::vector<std::reference_wrapper<mark::module::base>>;
			using const_find_result = std::vector<std::reference_wrapper<const mark::module::base>>;
			static constexpr unsigned max_size = 40;
		public:
			modular(mark::world& world, mark::vector<double> pos = { 0, 0 }, float rotation = 0.0f);
			void command(const mark::command& command) override;
			void attach(std::unique_ptr<module::base> module, mark::vector<int> pos);
			auto can_attach(const std::unique_ptr<module::base>& module, mark::vector<int> pos) const -> bool;
			auto get_attached(mark::module::base&)->std::vector<std::reference_wrapper<mark::module::base>>;
			auto get_core()->mark::module::core&;
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
		private:
			auto attached(
				mark::vector<int8_t> pos,
				mark::vector<uint8_t> size) const ->
				std::vector<std::reference_wrapper<const mark::module::base>>;
			void remove_dead(mark::tick_context&);
			void pick_up(mark::tick_context&);
			std::vector<std::unique_ptr<mark::module::base>> m_modules;
			mark::module::core* m_core = nullptr;
			float m_rotation = 0.f;
			mark::vector<double> m_moveto;
			mark::vector<double> m_lookat;
			bool m_ai = false;
			bool m_move = false;
		};
	}
}