#pragma once
#include "stdafx.h"

namespace mark {
	namespace resource {
		class image;

		class manager {
		public:
			manager();
			virtual auto image(const std::string& filename) ->
				std::shared_ptr<const mark::resource::image>;
			template<typename T>
			T random(T min, T max) {
				static_assert(false, "manager.random supports only numeric types");
			};
		protected:
			virtual auto random_int(int min, int max) -> int;
			virtual auto random_double(double min, double max) -> double;
		private:
			std::unordered_map<std::string, std::weak_ptr<const mark::resource::image>> m_images;
			std::random_device m_rd;
			std::mt19937_64 m_gen;
		};
		template<>
		auto manager::random<int>(int min, int max) -> int;
		template<>
		auto manager::random<unsigned>(unsigned min, unsigned max) -> unsigned;
		template<>
		auto manager::random<double>(double min, double max) -> double;
		template<>
		auto manager::random<float>(float min, float max) -> float;

		class manager_stub final : public manager {
		public:
			auto image(const std::string& filename)->
				std::shared_ptr<const mark::resource::image> override;
		protected:
			auto random_int(int min, int max) -> int override;
			auto random_double(double min, double max) -> double override;
		};
	};
}