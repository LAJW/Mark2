#pragma once
#include <memory>
#include <vector>
#include "iserializable.h"
#include "property.h"
#include "vector.h"

namespace mark {
	class sprite;
	namespace resource {
		class manager;
	}
	namespace unit {
		class base;
	}
	namespace terrain {
		class base;
	}

	class world {
	public:
		world(mark::resource::manager& resource_manager);
		auto render() const->std::vector<mark::sprite>;
		auto resource_manager() -> mark::resource::manager&;
		void tick(double dt);
		Property<mark::vector<float>> direction = mark::vector<float>(0, 0);
		auto map() const->const std::vector<std::vector<std::shared_ptr<mark::terrain::base>>>&;
		auto camera() const->mark::vector<double> { return m_camera; }
	private:
		std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> m_map;
		std::vector<std::shared_ptr<mark::unit::base>> m_units;
		mark::resource::manager& m_resource_manager;
		std::weak_ptr<mark::unit::base> m_camera_target;
		mark::vector<double> m_camera;
	};
}