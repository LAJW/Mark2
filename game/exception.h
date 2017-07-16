#pragma once
#include "stdafx.h"
#include <stdexcept>

namespace mark {
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
	namespace error {
		class base : public mark::exception {
		public:
			base() : mark::exception("mark_error") { };
		};
	}
}