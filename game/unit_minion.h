#pragma once
#include "unit_base.h"
#include "resource_image.h"

namespace mark {
	class world;
	namespace unit {
		class minion : public mark::unit::base {
		public:
			minion(mark::world& world, mark::vector<double> pos);
			auto render() const->std::vector<mark::sprite>;
			void tick(double dt);
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}