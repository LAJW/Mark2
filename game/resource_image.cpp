#include "stdafx.h"
#include "resource_image.h"

#ifndef UNIT_TEST

#include <SFML/Graphics/Texture.hpp>

mark::resource::image::image(std::string filename):
	m_filename(filename) {

	auto texture = std::make_unique<sf::Texture>();
	texture->loadFromFile("resource/texture/" + filename);
	m_texture = texture.release();
}

mark::resource::image::~image() {
	delete m_texture;
}

auto mark::resource::image::size() const noexcept -> mark::vector<unsigned> {
	return m_texture->getSize();
}

auto mark::resource::image::filename() const noexcept -> const std::string& {
	return m_filename;
}

auto mark::resource::image::texture() const noexcept -> const sf::Texture& {
	return *m_texture;
}

#else

mark::resource::image::image(std::string filename):
	m_filename(filename) {
	
	m_texture->loadFromFile(filename);
}

mark::resource::image::~image() { /* no-op */ }

auto mark::resource::image::getSize() const noexcept -> mark::vector<unsigned> {
	return { 64, 64 };
}

auto mark::resource::image::filename() const noexcept -> const std::string& {
	return m_filename;
}

auto mark::resource::image::texture() const noexcept -> const sf::Texture& {
	return *static_cast<sf::Texture*>(nullptr);
}

#endif