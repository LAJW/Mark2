#pragma once
#include <vector>
#include <memory>
#include "iserializable.h"
#include "vector.h"

namespace mark {
	namespace resource {
		class manager;
	}
	namespace terrain {
		class base;
	};
	class sprite;
	class world;

	class map final : public mark::iserializable {
	public:
		map(mark::resource::manager& resource_manager);
		auto traversable(mark::vector<double> pos, double radius = 0.0) const -> bool;
		auto render(mark::vector<double> top_left, mark::vector<double> bottom_right) const->std::vector<mark::sprite>;
		auto find_path(
			mark::vector<double> start,
			mark::vector<double> end,
			double radius = 0.0) const->std::vector<mark::vector<double>>;
	private:
		auto size() const -> mark::vector<int>;
		auto traversable(mark::vector<int> pos, int radius) const -> bool;
		std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> m_terrain;
	};
}