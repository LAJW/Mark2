#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "resource_image.h"

namespace mark {
	namespace resource {
		class manager {
		public:
			auto image(const std::string& filename)->std::shared_ptr<const mark::resource::image>;
		private:
			std::unordered_map<std::string, std::weak_ptr<const mark::resource::image>> m_images;
		};
	};
}