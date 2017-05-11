#pragma once
#include "resource_manager.h"
#include <SFML/Graphics.hpp>
#include "vector.h"

namespace mark {
	class sprite;
	class app final {
	public:
		app(const int argc, const char* argv[]);
		app(std::vector<std::string> arguments);
		void main();
	private:
		void render(const mark::sprite&, const mark::vector<double>& camera, sf::RenderTexture& buffer);
		

		sf::RenderWindow m_window;
		sf::RenderTexture m_buffer;
		sf::RenderTexture m_buffer2;
		sf::Shader m_fragment_shader;
		mark::resource::manager m_resource_manager;
	};
}
