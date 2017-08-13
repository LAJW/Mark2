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

		// Terminates the program if error wasn't checked/obtained
		// Should become an alias for code in the final build
		class guard {
		public:
			guard(enum class code error_code) :
				m_error_code(error_code) { }
			auto get() const noexcept -> enum class code {
				m_checked = true;
				return m_error_code;
			}
			~guard() {
				if (!m_checked) {
					std::terminate();
				}
			}
		private:
			enum class code m_error_code = code::success;
			mutable bool m_checked = false;
		};

		inline bool operator==(const error::guard& guard, error::code other) noexcept {
			return guard.get() == other;
		}

		inline bool operator!=(const error::guard& guard, error::code other) noexcept {
			return guard.get() != other;
		}
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