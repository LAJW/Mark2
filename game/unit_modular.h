#pragma once
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>
#include "unit_base.h"
#include "command.h"

namespace mark {

	namespace module {
		class base;
		class core;
	};
	class world;

	namespace unit {
		class modular : public base {
		public:
			class socket {
			public:
				socket(modular& parent, std::unique_ptr<module::base>, mark::vector<int> pos);
				socket(mark::unit::modular::socket&& other);
				~socket();
				auto get_attached()->std::vector<std::reference_wrapper<module::base>>;
				inline auto size() const -> mark::vector<unsigned>;
				inline auto pos() const { return m_pos; }
				auto relative_pos() const->mark::vector<double>;
				auto render() const->std::vector<mark::sprite>;
				auto rotation() const -> float;
			private:
				const mark::vector<int> m_pos;
				std::unique_ptr<module::base> m_module;
				modular& m_parent;
			};
		public:
			modular(mark::world& world, mark::vector<double> pos = { 0, 0 }, float rotation = 0.0f);
			void command(const mark::command& command) override;
			void attach(std::unique_ptr<module::base> module, mark::vector<int> pos);
			auto get_attached(const socket&, mark::vector<int> pos)->std::vector<std::reference_wrapper<socket>>;
			auto get_core()->mark::module::core&;
			auto render() const->std::vector<mark::sprite> override;
			void tick(double dt) override;
			inline auto rotation() const { return m_rotation; }
		private:
			std::vector<socket> m_sockets;
			mark::module::core* m_core = nullptr;
			float m_rotation = 0.f;
			mark::command m_command;
			std::vector<mark::vector<double>> m_path;
		};
	}
}