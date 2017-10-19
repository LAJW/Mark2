#include "stdafx.h"
#include "resource_image.h"
#include <SFML/Graphics/Texture.hpp>

mark::resource::image_impl::image_impl(std::string filename):
	m_filename(filename),
	m_texture(std::make_unique<sf::Texture>()) {
	m_texture->loadFromFile("resource/texture/" + filename);
}

auto mark::resource::image_impl::size() const noexcept -> vector<unsigned> {
	return m_texture->getSize();
}

auto mark::resource::image_impl::filename() const noexcept -> const std::string& {
	return m_filename;
}

auto mark::resource::image_impl::texture() const noexcept -> const sf::Texture& {
	return *m_texture;
}

auto mark::resource::image_stub::size() const noexcept -> vector<unsigned> {
	return { 64, 64 };
}

auto mark::resource::image_stub::filename() const noexcept -> const std::string& {
	static const std::string file_name("file.png");
	return file_name;
}

auto mark::resource::image_stub::texture() const noexcept -> const sf::Texture& {
	return *static_cast<sf::Texture*>(nullptr);
}