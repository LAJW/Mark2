#pragma once
#include <memory>
#include <vector>
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
			class socket final {
			public:
				socket(modular& parent, std::unique_ptr<module::base>, mark::vector<int> pos);
				socket(mark::unit::modular::socket&& other);
				mark::unit::modular::socket& operator=(mark::unit::modular::socket&& socket);
				~socket();
				auto get_attached()->std::vector<std::reference_wrapper<module::base>>;
				inline auto size() const -> mark::vector<unsigned>;
				inline auto pos() const { return m_pos; }
				auto relative_pos() const->mark::vector<double>;
				void tick(mark::tick_context& context);
				auto rotation() const -> float;
				auto dead() const -> bool;
				inline auto world() -> mark::world& { return m_parent.world(); }
				inline auto team() const -> int { return m_parent.team(); }
				auto module() -> mark::module::base&;
				auto module() const -> const mark::module::base&;
				std::unique_ptr<mark::module::base> detach();
				auto collides(mark::vector<double> pos, float radius) const -> bool;
			private:
				mark::vector<int> m_pos;
				std::unique_ptr<mark::module::base> m_module;
				mark::unit::modular& m_parent;
			};
		public:
			modular(mark::world& world, mark::vector<double> pos = { 0, 0 }, float rotation = 0.0f);
			void command(const mark::command& command) override;
			void attach(std::unique_ptr<module::base> module, mark::vector<int> pos);
			auto can_attach(const std::unique_ptr<module::base>& module, mark::vector<int> pos) const -> bool;
			auto get_attached(const socket&, mark::vector<int> pos)->std::vector<std::reference_wrapper<socket>>;
			auto get_core()->mark::module::core&;
			void tick(mark::tick_context& context) override;
			inline auto rotation() const { return m_rotation; }
			auto dead() const -> bool override;
			inline auto world() -> mark::world& { return m_world; }
			void damage(unsigned amount, mark::vector<double> pos) override;
			auto invincible() const -> bool override;
			void activate(const std::shared_ptr<mark::unit::base>& by) override;
			auto containers()->std::vector<std::reference_wrapper<mark::module::cargo>>;
			auto detach(mark::vector<int> pos)->std::unique_ptr<mark::module::base>;
			auto collides(mark::vector<double> pos, float radius) const -> bool override;
			auto module(mark::vector<double> pos, float radius) const -> const mark::module::base*;
			auto module(mark::vector<double> pos, float radius) -> mark::module::base*;
			auto module(mark::vector<int> pos) const -> const mark::module::base*;
			auto module(mark::vector<int> pos)->mark::module::base*;
		private:
			std::vector<socket> m_sockets;
			mark::module::core* m_core = nullptr;
			float m_rotation = 0.f;
			mark::vector<double> m_moveto;
			mark::vector<double> m_lookat;
			bool m_ai = false;
		};
	}
}