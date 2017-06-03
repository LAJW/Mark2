#pragma once
#include <memory>
#include "yaml-cpp/yaml.h"

namespace mark {
	class iserializable {
	public:
		virtual void serialize(YAML::Emitter& out) const = 0;
		inline auto id() const -> uint64_t {
			return reinterpret_cast<uint64_t>(this);
		}
	protected:
		virtual ~iserializable() = default;
	};
}