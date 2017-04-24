#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <random>

namespace sf {
	class Texture;
}

namespace mark {
	namespace resource {
		using image = sf::Texture;
		class manager {
		public:
			manager();
			auto image(const std::string& filename)->std::shared_ptr<const mark::resource::image>;
			auto random_int(int min, int max) -> int;
			auto random_double(double min, double max) -> double;
		private:
			std::unordered_map<std::string, std::weak_ptr<const mark::resource::image>> m_images;
			std::random_device m_rd;
			std::mt19937_64 m_gen;
		};
	};
}