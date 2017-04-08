#pragma once
#include <memory>
#include "json.h"

namespace mark {
	class iserializable {
	public:
		static std::unique_ptr<iserializable> deserialize(const json&);
		json serialize();
	protected:
		virtual ~iserializable() = default;
	};
}