#include "stdafx.h"
#include "tick_context.h"

namespace {
	static float width(char ch) {
		if (ch >= '0' && ch <= '9') {
			return 6.f / 7.f;
		} else if (ch == 'I' || ch == '1' || ch == 't') {
			return 4.f / 7.f;
		} else if (ch == 'f' || ch == 'j' || ch == 'r' || ch == ';' || ch == ',') {
			return 3.f / 7.f;
		} else if (ch == 'i' || ch == 'l' || ch == '.' || ch == ':') {
			return 2.f / 7.f;
		} else if (ch >= 'a' && ch <= 'z' || ch == 'Y') {
			return 5.f / 7.f;
		} else {
			return 1;
		}
	}
	static float offset_y(char ch) {
		if (ch >= 'a' && ch <= 'z' || ch == ':' || ch == ';' || ch == '.' || ch == ',') {
			return 4.f;
		} else {
			return 0.f;
		}
	}
}


void mark::print(
	std::shared_ptr<const resource::image> font,
	std::vector<sprite>& out,
	vector<double> pos,
	vector<double> box,
	float size,
	sf::Color color,
	std::string text) {

	auto offset = vector<double>(size, size) / 2.0;
	for (size_t i = 0; i < text.size(); i++) {
		const auto ch = text[i];
		char frame = -1;
		const auto end = ('Z' - 'A' + 1) * 2 + '9' - '0' + 1;
		if (ch >= 'A' && ch <= 'Z') {
			frame = ch - 'A';
		} else if (ch >= 'a' && ch <= 'z') {
			frame = ch - 'a' + ('Z' - 'A') + 1;
		} else if (ch >= '0' && ch <= '9') {
			frame = ch - '0' + 2 * ('Z' - 'A' + 1);
		} else if (ch == ':') {
			frame = end;
		} else if (ch == ';') {
			frame = end + 1;
		} else if (ch == '.') {
			frame = end + 2;
		} else if (ch == ',') {
			frame = end + 3;
		}
		if (frame >= 0) {
			sprite args;
			args.image = font;
			args.pos = pos + offset + vector<double>(0, offset_y(ch));
			args.size = size;
			args.frame = frame;
			args.color = color;
			out.emplace_back(args);
		}
		if (ch != '\n') {
			// find next non-alnum, if goes over the screen - indent
			double word_width = 0;
			for (size_t j = i + 1; j <= text.size(); j++) {
				if (isalnum(text[j])) {
					word_width += width(text[j]) * size;
				} else {
					if (offset.x + word_width > box.x) {
						offset.x = -size / 2.0;
						offset.y += size * 1.5;
					}
					break;
				}
			}
		}
		if (offset.x + size < box.x && ch != '\n') {
			offset.x += width(ch) * size;
		} else {
			offset.y += size * 1.5f;
			offset.x = size / 2.0;
		}
	}
}

mark::tick_context::tick_context(resource::manager& rm):
	m_resource_manager(rm) {
}

void mark::tick_context::render(const bar_info & info) {
	auto& image = info.image;
	auto type = info.type;
	auto pos = info.pos;
	auto percentage = info.percentage;
	const auto percent = 100.f * std::max(0.f, percentage);
	const auto edge = static_cast<uint8_t>(std::floor(percent / 10.f));
	for (int i = 0; i < 10; i++) {
		const auto offset_x = 7.f * static_cast<float>(i - 5);
		uint8_t opacity = 255;
		uint8_t frame = 0;
		// render gray background
		if (i >= edge) {
			sprite args;
			args.image = image;
			args.pos = pos + vector<double>(offset_x, 0);
			args.size = 8.f;
			args.frame = 6;
			this->sprites[50].emplace_back(args);
		}
		// calculate edge opacity
		if (i == edge) {
			opacity = static_cast<uint8_t>(std::fmod(percent, 10) / 10.f * 255.f);
		}
		if (i <= edge) {
			// choose color
			if (type == tick_context::bar_type::shield) {
				frame = 5;
			} else if (type == tick_context::bar_type::energy) {
				frame = 4;
			} else if (type == tick_context::bar_type::health) {
				if (percent > 75.f) {
					frame = 3;
				} else if (percent > 50.f) {
					frame = 2;
				} else if (percent > 25.f) {
					frame = 1;
				} else {
					frame = 0;
				}
			}
			sprite args;
			args.image = image;
			args.pos = pos + vector<double>(offset_x, 0);
			args.size = 8.f;
			args.frame = frame;
			args.color = sf::Color(255, 255, 255, opacity);
			this->sprites[50].emplace_back(args);
		}
	}

}

void mark::tick_context::render(const tick_context::spray_info& info) {
	for (size_t i = 0; i < info.count; i++) {
		const auto tmp_velocity = !std::isnan(info.max_velocity)
			? this->random(info.min_velocity, info.max_velocity)
			: info.min_velocity;
		const auto tmp_lifespan = !std::isnan(info.max_lifespan)
			? this->random(info.min_lifespan, info.max_lifespan)
			: info.min_lifespan;
		const auto tmp_diameter = !std::isnan(info.max_diameter)
			? this->random(info.min_diameter, info.max_diameter)
			: info.min_diameter;
		const auto tmp_pos = info.pos + rotate(
			vector<double>(1, 0), info.direction) * (info.step * static_cast<double>(i) / static_cast<double>(info.count));
		const auto rotation = info.direction + this->random(0.f, info.cone) - info.cone / 2.f;
		particle::info attr;
		attr.image = info.image;
		attr.color = info.color;
		attr.pos = tmp_pos;
		attr.velocity = tmp_velocity;
		attr.direction = rotation;
		attr.lifespan = tmp_lifespan;
		attr.color = info.color;
		attr.size = tmp_diameter;
		attr.layer = info.layer;
		this->particles.emplace_back(attr);
	}
}

void mark::tick_context::spray_info::velocity(float min, float max) {
	min_velocity = min;
	max_velocity = max;
}

void mark::tick_context::spray_info::lifespan(float min, float max) {
	min_lifespan = min;
	max_lifespan = max;
}

void mark::tick_context::spray_info::diameter(float min, float max) {
	min_diameter = min;
	max_diameter = max;
}
