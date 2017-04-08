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
		auto traversable(mark::vector<double> pos) const -> bool;
		auto render() const->std::vector<mark::sprite>;
		auto find_path(mark::vector<double> start, mark::vector<double> end) const->std::vector<mark::vector<double>>;
	private:
		auto size() const -> mark::vector<int>;
		auto traversable(mark::vector<int> pos) const -> bool;
		std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> m_terrain;
	};
}