#pragma once
#include <resource/manager.h>
#include "stdafx.h"

namespace mark {
struct update_context
{
	update_context(resource::manager&, random& random);
	~update_context();
	double dt;
	std::map<int, std::vector<renderable>> sprites;
	std::map<int, std::vector<sprite>> normals;
	std::vector<std::pair<vd, sf::Color>> lights;
	std::vector<not_null<shared_ptr<unit::base>>> units;
	std::vector<particle> particles;
	enum class bar_kind
	{
		health,
		energy,
		shield
	};
	struct bar_info
	{
		resource::image_ptr image;
		bar_kind type;
		vd pos;
		float percentage;
	};
	void render(const bar_info& info);
	mark::random &random;
	struct spray_info
	{
		resource::image_ptr image;
		vd pos;
		double min_velocity = NAN;
		double max_velocity = NAN;
		float min_lifespan = NAN;
		float max_lifespan = NAN;
		float min_diameter = NAN;
		float max_diameter = NAN;
		size_t count = 1;
		double step = 0.f;
		float direction = 0.f;
		float cone = 360.f;
		sf::Color color = sf::Color::White;
		int layer = 0;

		void velocity(double min, double max = NAN);
		void lifespan(float min, float max = NAN);
		void diameter(float min, float max = NAN);
	};
	void render(const spray_info&);
	bool crit = false;

private:
	resource::manager& m_resource_manager;
};

struct text_info
{
	int layer = 0;
	resource::image_ptr font;
	std::variant<vd, vi32> pos;
	vd box;
	float size = 14.f;
	sf::Color color = sf::Color::White;
	std::string text;
	bool centred = false;
};

void render(std::vector<renderable>& layer, const text_info&);

} // namespace mark
