#include "tick_context.h"
#include <string>

void mark::print(std::shared_ptr<const mark::resource::image> font, std::vector<mark::sprite>& out, mark::vector<double> pos, mark::vector<double> box, float size, sf::Color color, std::string text) {
	auto offset = mark::vector<double>(0, 0);
	for (size_t i = 0; i < text.size(); i++) {
		if (offset.x < box.x) {
			offset.x += size;
		} else {
			offset.y += size;
			offset.x = 0.0;
		}
		const auto ch = text[i];
		if (isalnum(ch)) {
			unsigned frame = 0;
			if (ch >= 'A' && ch <= 'Z') {
				frame = ch - 'A';
			} else if (ch >= 'a' && ch <= 'z') {
				frame = ch - 'a';
			} else if (ch >= '1' && ch <= '0') {
				frame = ch - '1' + 'Z' - 'A';
			}
			out.push_back(mark::sprite(font, pos + offset, size, 0.f, frame, color));
		}
	}
}