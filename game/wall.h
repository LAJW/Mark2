#pragma once
#include "unit_base.h"
#include "resource_image.h"

namespace mark {
	class world;
	class sprite;

	namespace unit {
		class wall : public mark::unit::base {
		public:
			wall(mark::world& world, mark::vector<double> pos);
			auto render() const->std::vector<mark::sprite>;
			void tick(double dt) { }
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}