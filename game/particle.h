#pragma once
#include "resource_image.h"
#include "vector.h"
#include "resource_image.h"
#include <map>
#include <memory>
#include <vector>

namespace mark {
	class sprite;
	class particle {
	public:
		particle(
			std::shared_ptr<const mark::resource::image>& image,
			mark::vector<double> pos,
			float velocity,
			float direction,
			float lifespan);
		void tick(double dt, std::map<int, std::vector<mark::sprite>>& m_pos);
		inline bool dead() const { return m_cur_lifespan <= 0; }
	private:
		std::shared_ptr<const mark::resource::image> m_image;
		mark::vector<double> m_pos;
		float m_lifespan;
		float m_cur_lifespan;
		float m_velocity;
		float m_direction;
	};
}