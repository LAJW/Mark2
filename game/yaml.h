#pragma once
#pragma warning(push, 3)
#include <yaml-cpp\yaml.h>
#pragma warning(pop)

namespace YAML {
	template<typename T>
	struct convert<mark::vector<T>> {
		static Node encode(const mark::vector<T>& rhs) {
			Node node;
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			return node;
		}

		static bool decode(const Node& node, mark::vector<T>& rhs) {
			if (!node.IsMap() || node.size() != 2) {
				return false;
			}

			rhs.x = node["x"].as<T>();
			rhs.y = node["y"].as<T>();
			return true;
		}
	};
}
