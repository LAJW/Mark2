#pragma once

namespace mark {

template <typename T, bool = false>
struct add_const_if final
{
	using type = T;
};

template <typename T>
struct add_const_if<T, true> final
{
	using type = std::add_const_t<T>;
};

template <typename T, bool cond>
using add_const_if_t = typename add_const_if<T, cond>::type;

} // namespace mark
