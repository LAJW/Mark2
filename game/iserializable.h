#pragma once
#include <memory>
#include "yaml-cpp/yaml.h"

namespace mark {
	class iserializable {
	public:
		inline virtual void serialize(YAML::Emitter& out) const {
			out << "Serializer not defined for this object";
		};
		inline auto id() const -> uint64_t {
			return reinterpret_cast<uint64_t>(this);
		}
	protected:
		virtual ~iserializable() = default;
	};
}