#pragma once
#include "stdafx.h"
#include <type_traits>

namespace mark {
namespace resource {
class image;

class manager
{
public:
	virtual auto image(const std::string& filename)
		-> std::shared_ptr<const resource::image> = 0;
	template <typename T>
	T random(T min, T max)
	{
		static_assert(
			std::is_arithmetic_v<T>,
			"Cannot generate random value for non-arithmetic type");
		// else statements are required - early retur doesn't work well with
		// constexpr if
		if constexpr (std::is_integral_v<T>) {
			if constexpr (std::is_signed_v<T>) {
				return static_cast<T>(random_signed(min, max));
			} else {
				return static_cast<T>(random_unsigned(min, max));
			}
		} else {
			return static_cast<T>(random_double(min, max));
		}
	}

protected:
	virtual auto random_signed(int64_t min, int64_t max) -> int64_t = 0;
	virtual auto random_unsigned(uint64_t min, uint64_t max) -> uint64_t = 0;
	virtual auto random_double(double min, double max) -> double = 0;
};

class manager_impl final : public manager
{
public:
	manager_impl();
	auto image(const std::string& filename)
		-> std::shared_ptr<const resource::image> override;

protected:
	auto random_signed(int64_t min, int64_t max) -> int64_t override;
	auto random_unsigned(uint64_t min, uint64_t max) -> uint64_t override;
	auto random_double(double min, double max) -> double override;

private:
	std::unordered_map<std::string, std::weak_ptr<const resource::image>>
		m_images;
	std::random_device m_rd;
	std::mt19937_64 m_gen;
};

class manager_stub final : public manager
{
public:
	auto image(const std::string& filename)
		-> std::shared_ptr<const resource::image> override;

protected:
	auto random_signed(int64_t min, int64_t max) -> int64_t override;
	auto random_unsigned(uint64_t min, uint64_t max) -> uint64_t override;
	auto random_double(double min, double max) -> double override;
};
}; // namespace resource
} // namespace mark
