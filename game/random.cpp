#include <stdafx.h>
#include "random.h"

auto mark::random_impl::random_signed(int64_t min, int64_t max)
	-> int64_t
{
	std::uniform_int_distribution<int64_t> dist(min, max);
	return dist(m_gen);
}

auto mark::random_impl::random_unsigned(uint64_t min, uint64_t max)
	-> uint64_t
{
	std::uniform_int_distribution<uint64_t> dist(min, max);
	return dist(m_gen);
}

auto mark::random_impl::random_double(double min, double max)
	-> double
{
	std::uniform_real_distribution<> dist(min, max);
	return dist(m_gen);
}

auto mark::random_stub::random_signed(int64_t min, int64_t max)
	-> int64_t
{
	return (max - min) / 2 + min;
}

auto mark::random_stub::random_unsigned(uint64_t min, uint64_t max)
	-> uint64_t
{
	return (max - min) / 2 + min;
}

auto mark::random_stub::random_double(double min, double max)
	-> double
{
	return (max - min) / 2 + min;
}
