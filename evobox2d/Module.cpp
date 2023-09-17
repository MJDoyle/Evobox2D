#include "Module.hpp"

#include "Organism.hpp"

Module::Module(b2World& world, sf::Vector2i position, int type)
{
	m_position = position;

	m_anchorActive = false;

	m_type = type;
}

void Module::Update(sf::RenderWindow& window)
{
}