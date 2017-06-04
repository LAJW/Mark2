#pragma once
#include "stdafx.h"
#include "resource_manager.h"
#include <SFML/Graphics.hpp>

namespace mark {
	class sprite;
	class app final {
	public:
		app(const int argc, const char* argv[]);
		app(std::vector<std::string> arguments);
		void main();
	private:
		sf::RenderWindow m_window;
		mark::resource::manager m_resource_manager;
	};
}
