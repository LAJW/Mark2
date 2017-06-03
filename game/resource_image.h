#pragma once
#include <string>
#include "vector.h"

namespace sf {
	class Texture;
}

namespace mark {
	namespace resource {
		class image {
		public:
			image(std::string filename);
			~image();
			auto size() const noexcept -> mark::vector<unsigned>;
			auto filename() const noexcept -> const std::string&;
			auto texture() const noexcept -> const sf::Texture&;
		private:
			std::string m_filename;
			sf::Texture* m_texture;
		};
	};
}
