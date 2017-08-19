#pragma once
#include "stdafx.h"
#include <stdexcept>
#include <atomic>

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

	class user_error : public mark::exception {
	public:
		inline user_error(const char* message) : exception(message) {}
		inline user_error(const std::string& message) : exception(message) {}
	};
}