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

#ifndef UNIT_TEST
		class manager {
		public:
			manager();
			auto image(const std::string& filename)->std::shared_ptr<const mark::resource::image>;
			template<typename T>
			T random(T min, T max) {
				static_assert(false, "manager.random supports only numeric types");
			};
		private:
			auto random_int(int min, int max) -> int;
			auto random_double(double min, double max) -> double;
			std::unordered_map<std::string, std::weak_ptr<const mark::resource::image>> m_images;
			std::random_device m_rd;
			std::mt19937_64 m_gen;
		};
#else
		class manager {
		public:
			auto image(const std::string& filename)->std::shared_ptr<const mark::resource::image>;
			template<typename T>
			T random(T min, T max) {
				static_assert(false, "manager.random supports only numeric types");
			};
		private:
			auto random_int(int min, int max) -> int;
			auto random_double(double min, double max) -> double;
		};
#endif
		template<>
		auto manager::random<int>(int min, int max) -> int;
		template<>
		auto manager::random<unsigned>(unsigned min, unsigned max) -> unsigned;
		template<>
		auto manager::random<double>(double min, double max) -> double;
		template<>
		auto manager::random<float>(float min, float max) -> float;
	};
}