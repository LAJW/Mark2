#pragma once
#include "stdafx.h"
#include <stdexcept>
#include <system_error>

namespace mark {

inline bool success(std::error_code error_code) { return !error_code; }

inline bool failure(std::error_code error_code)
{
	return static_cast<bool>(error_code);
}

namespace error {
enum class code
{
	// Success
	success,
	// (Success state) Module has been stacked
	stacked,
	// Invalid function input
	bad_input,
	// Module position out of range
	bad_pos,
	// Module is unique and cannot be randomized
	module_not_random,
	// Item position in the grid is already taken
	occupied,
	// Can't find property to randomize (if applied on a wrong item)
	property_not_found,
	// Cannot randomize specific property (for using items randomizing specific
	// properties)
	property_not_random,
	// Trying to put window into a window
	ui_cycle,
	// Invalid "before" argument for insertion
	ui_bad_before,
};

std::error_code make_error_code(code e);
} // namespace error

class exception : public std::runtime_error
{
public:
	inline exception(const char* message)
		: std::runtime_error(message)
	{}
	inline exception(const std::string& message)
		: std::runtime_error(message)
	{}
};

class user_error : public exception
{
public:
	inline user_error(const char* message)
		: exception(message)
	{}
	inline user_error(const std::string& message)
		: exception(message)
	{}
};
} // namespace mark

namespace std {
template <>
struct is_error_code_enum<mark::error::code> : public true_type
{};
} // namespace std
