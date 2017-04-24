#pragma once
#ifndef UNIT_TEST
#include <SFML/Graphics/Texture.hpp>
#else
#include "vector.h"
namespace sf {
	class Texture {
	public:
		inline mark::vector<unsigned> getSize() const { return { 64, 64 }; }
	};
}
#endif

namespace mark {
	namespace resource {
		using image = sf::Texture;
	};
}