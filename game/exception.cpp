#include "stdafx.h"
#include "exception.h"

class custom_category_t : public std::error_category {
public:
	virtual const char* name() const noexcept { return "custom"; }
	virtual std::string message(int ev) const noexcept
	{
		using namespace mark::error;
		switch (static_cast<mark::error::code>(ev)) {
		case code::success: return "Success";
		case code::bad_pos: return "Invalid module position";
		case code::occupied: return "Space occupied";
		}
		assert(false);
		return "UNREACHABLE";
	}
} custom_category;

namespace std {
std::error_code make_error_code(mark::error::code e)
{ return std::error_code(static_cast<int>(e), custom_category); }
}
