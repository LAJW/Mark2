#pragma once
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "iserializable.h"
#include "property.h"
#include "vector.h"
#include "map.h"
#include "particle.h"

namespace mark {
	class sprite;
	class command;
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
		auto resource_manager() -> mark::resource::manager&;
		auto tick(double dt, mark::vector<double> screen_size) -> std::map<int, std::vector<mark::sprite>>;
		auto map() const->const mark::map&;
		auto camera() const->mark::vector<double> { return m_camera; }
		auto find(mark::vector<double> pos, double radius)->std::vector<std::shared_ptr<mark::unit::base>>;
		auto find(mark::vector<double> pos, double radius, const std::function<bool(const mark::unit::base&)>& pred)
			->std::vector<std::shared_ptr<mark::unit::base>>;
		auto find_one(mark::vector<double> pos, double radius, const std::function<bool(const mark::unit::base&)>& pred)
			->std::shared_ptr<mark::unit::base>;
		void command(const mark::command& command);
	private:
		mark::map m_map;
		std::vector<std::shared_ptr<mark::unit::base>> m_units;
		mark::resource::manager& m_resource_manager;
		std::weak_ptr<mark::unit::base> m_camera_target;
		mark::vector<double> m_camera;
		std::vector<mark::particle> m_particles;
	};
}