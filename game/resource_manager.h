#pragma once
#include "stdafx.h"

namespace mark {
	namespace resource {
		class image;

		class manager {
		public:
			virtual auto image(const std::string& filename) ->
				std::shared_ptr<const resource::image> = 0;
			template<typename T>
			T random(T min, T max) {
				static_assert(false, "manager.random supports only numeric types");
			};
		protected:
			virtual auto random_int(int min, int max) -> int = 0;
			virtual auto random_double(double min, double max) -> double = 0;
		};
		template<>
		auto manager::random<bool>(bool min, bool max) -> bool;
		template<>
		auto manager::random<int>(int min, int max) -> int;
		template<>
		auto manager::random<unsigned>(unsigned min, unsigned max) -> unsigned;
		template<>
		auto manager::random<size_t>(size_t min, size_t max) -> size_t;
		template<>
		auto manager::random<double>(double min, double max) -> double;
		template<>
		auto manager::random<float>(float min, float max) -> float;

		class manager_impl final : public manager {
		public:
			manager_impl();
			auto image(const std::string& filename)->
				std::shared_ptr<const resource::image> override;
		protected:
			auto random_int(int min, int max) -> int override;
			auto random_double(double min, double max) -> double override;
		private:
			std::unordered_map<std::string, std::weak_ptr<const resource::image>> m_images;
			std::random_device m_rd;
			std::mt19937_64 m_gen;
		};

		class manager_stub final : public manager {
		public:
			auto image(const std::string& filename)->
				std::shared_ptr<const resource::image> override;
		protected:
			auto random_int(int min, int max) -> int override;
			auto random_double(double min, double max) -> double override;
		};
	};
}