#ifndef MODULE_HPP
#define MODULE_HPP

#include "Config.hpp"

class Organism;

class Module
{
	public:

		Module(b2World& world, sf::Vector2i position, int type);

		void Update(sf::RenderWindow& window);

		void SetOrganism(std::weak_ptr<Organism> organism) {m_organism = organism;}
		void SetPosition(sf::Vector2i pos) {m_position = pos;}
		void SetComRelPosition(sf::Vector2f pos) {m_comRelPosition = pos;}


		sf::Vector2f GetComRelPosition() {return m_comRelPosition;}
		sf::Vector2i GetPosition() {return m_position;}
		int GetType() {return m_type;}


	private:

		//This module position (in units of modules)
		sf::Vector2i m_position;

		//Position relative to center of mass (in units of modules)
		sf::Vector2f m_comRelPosition;

		bool m_anchorActive;

		//Module type
		int m_type;

		std::weak_ptr<Organism> m_organism;

};

#endif