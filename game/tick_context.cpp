#include "tick_context.h"
#include <string>

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


void mark::print(std::shared_ptr<const mark::resource::image> font, std::vector<mark::sprite>& out, mark::vector<double> pos, mark::vector<double> box, float size, sf::Color color, std::string text) {
	auto offset = mark::vector<double>(size, size) / 2.0;
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
			out.push_back(mark::sprite(font, pos + offset + mark::vector<double>(0, offset_y(ch)), size, 0.f, frame, color));
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