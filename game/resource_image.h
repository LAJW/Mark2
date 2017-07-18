#pragma once
#include "stdafx.h"
#include "vector.h"
#include <SFML/Graphics/Texture.hpp>

namespace mark {
	namespace resource {
		class image {
		public:
			virtual ~image() = default;
			virtual auto size() const noexcept -> mark::vector<unsigned>;
			virtual auto filename() const noexcept -> const std::string&;
			virtual auto texture() const noexcept -> const sf::Texture&;
		};

		class image_impl final : public image {
		public:
			image_impl(std::string filename);
			auto size() const noexcept -> mark::vector<unsigned> override;
			auto filename() const noexcept -> const std::string& override;
			auto texture() const noexcept -> const sf::Texture& override;
		private:
			std::string m_filename;
			std::unique_ptr<sf::Texture> m_texture;
		};
	};
}
