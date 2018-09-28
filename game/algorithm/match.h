#pragma once
#include <variant>

namespace mark {
template <typename return_type, typename T>
optional<return_type> maybe_call(const T&)
{
	return {};
}

template <typename return_type, typename U, typename T, typename... Ts>
auto maybe_call(U&& arg, [[maybe_unused]] T func, [[maybe_unused]] Ts&&... ts)
{
	if constexpr (std::is_invocable<T, U>::value) {
		return optional<return_type>(func(arg));
	}
	// No else, because [[maybe_unused]] doesn't work on a variadics in VS
	return maybe_call<return_type>(
		std::forward<U>(arg), std::forward<Ts>(ts)...);
}

template <typename return_type, typename U, typename... Ts>
auto maybe_match(U&& arg, Ts... ts)
{
	return std::visit(
		[&](auto&& arg) {
			return maybe_call<return_type>(arg, std::forward<Ts>(ts)...);
		},
		std::forward<U>(arg));
}

template <typename... Base>
struct visitor : Base...
{
	using Base::operator()...;
};

template <typename... T>
visitor(T...)->visitor<T...>;

template <typename T, typename... Ts>
auto match(T&& value, Ts&&... ts)
{
	return std::visit(
		visitor{ std::forward<Ts>(ts)... }, std::forward<T>(value));
}
} // namespace mark
