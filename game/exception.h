#pragma once
#include "stdafx.h"
#include <stdexcept>
#include <system_error>

namespace mark {
namespace error {
enum class code {
	success,
	bad_pos,
	occupied
};
}

class exception : public std::runtime_error {
public:
	inline exception(const char* message) : std::runtime_error(message) {}
	inline exception(const std::string& message) : std::runtime_error(message) {}
};

class user_error : public exception {
public:
	inline user_error(const char* message) : exception(message) {}
	inline user_error(const std::string& message) : exception(message) {}
};
}

namespace std {
template<> struct is_error_code_enum<mark::error::code> : public true_type {};
std::error_code make_error_code(mark::error::code e);
}

