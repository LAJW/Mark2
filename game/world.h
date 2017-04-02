#pragma once
#include <memory>
#include <vector>
#include "iserializable.h"

namespace mark {
	class sprite;
	namespace resource {
		class manager;
	}
	namespace unit {
		class base;
	}

	class world {
	public:
		world(mark::resource::manager& resource_manager);
		auto render() const->std::vector<mark::sprite>;
		auto resource_manager() -> mark::resource::manager&;
		void tick(double dt);
	private:
		std::vector<std::shared_ptr<mark::unit::base>> m_units;
		mark::resource::manager& m_resource_manager;
	};
}