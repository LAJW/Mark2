#pragma once
#include <type_traits>
#include <stdafx.h>

namespace mark {

// Base class for random number generator
// Instantiate using one of the two available implementations - random_impl
// and random_stub. Random_impl is the real random number generator, while
// random_stub is a stub for use in unit tests when deterministc values are
// desirable
class random {
public:
	template <typename T>
	T operator()(T min, T max)
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
private:
	virtual auto random_signed(int64_t min, int64_t max) -> int64_t = 0;
	virtual auto random_unsigned(uint64_t min, uint64_t max) -> uint64_t = 0;
	virtual auto random_double(double min, double max) -> double = 0;
};

class random_impl final : public random {
	auto random_signed(int64_t min, int64_t max) -> int64_t override;
	auto random_unsigned(uint64_t min, uint64_t max) -> uint64_t override;
	auto random_double(double min, double max) -> double override;

	std::random_device m_rd;
	std::mt19937_64 m_gen;
};

class random_stub final : public random {
	auto random_signed(int64_t min, int64_t max) -> int64_t override;
	auto random_unsigned(uint64_t min, uint64_t max) -> uint64_t override;
	auto random_double(double min, double max) -> double override;
};

}