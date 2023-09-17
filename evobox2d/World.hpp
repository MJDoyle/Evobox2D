#ifndef WORLD_HPP
#define WORLD_HPP

#include "Organism.hpp"

//A basic light source
struct LightSource
{
	LightSource(b2Vec2 position, float intensity, float radius)
	{
		m_position = position;
		m_intensity = intensity;
		m_radius = radius;
	}

	b2Vec2 m_position;

	float m_intensity;

	float m_radius;
};

//A merge struct, this is created when it is determined that two interacting organisms can merge and is then used by the MergeOrganisms function

struct Merge
{
	Merge(std::weak_ptr<Organism> org1, std::weak_ptr<Organism> org2, std::weak_ptr<Module> mod1, std::weak_ptr<Module> mod2, int side1, int side2)
	{
		m_org1 = org1;
		m_org2 = org2;

		m_mod1 = mod1;
		m_mod2 = mod2;

		m_side1 = side1;
		m_side2 = side2;
	}

	std::weak_ptr<Organism> m_org1;
	std::weak_ptr<Organism> m_org2;

	std::weak_ptr<Module> m_mod1;
	std::weak_ptr<Module> m_mod2;

	int m_side1;
	int m_side2;
};

//This is just used to indicate where to draw notifications

struct Notification
{
	Notification(sf::Vector2f position, std::string text, sf::Font& font) 
	{	
		m_counter = 0;

		m_text.setColor(sf::Color::Black);
		m_text.setCharacterSize(16);
		m_text.setPosition(position);
		m_text.setString(text);
		m_text.setFont(font); 
	}

	//Text
	sf::Text m_text;

	//Counter
	int m_counter;

	bool Update()
	{
		m_counter ++;

		if (m_counter >= TEXT_DRAW_TIMER)
			return false;

		return true;
	}

	void Draw(sf::RenderWindow& window)
	{
		//std::cout << "Start drawing" << std::endl;

		window.draw(m_text);

		//std::cout << "Finished drawing" << std::endl;
	}
};

class World
{
	public:

		World(unsigned long seed, char * outputName);

		bool Update();

		void Draw();

		void HandleEvents();

		void SetupWindow();

	private:

		//Counter for organisms created via abiogenesis
		long long m_createdOrganisms;

		//Counter for organisms created via reproduction
		long long m_reproducedOrganisms;

		//Box2D ground body, used for friction
		b2Body* m_groundBody;

		//window
		sf::RenderWindow m_window;

		//Main clock
		sf::Clock m_mainClock;

		//FPS
		int m_FPS;

		sf::Text m_FPSText;

		sf::Text m_simTimeText;

		sf::Text m_miscText;

		//Font for text
		sf::Font m_font;

		//Text
		sf::Text m_selectedOrgInfo;

		//List of notifications to be drawn
		std::vector<std::shared_ptr<Notification>> m_notifications;
		
		//Organism that has been selected
		std::weak_ptr<Organism> m_selectedOrganism;

		bool m_paused;

		b2World* m_world;

		b2Body* m_walls[4];

		void WriteState();

		char * m_outputName;

		//State output file
		std::ofstream m_stateOutputFile;

		//Event output file
		std::ofstream m_eventOutputFile;

		//State output file name
		std::stringstream m_filename;

		//Event output file name
		std::stringstream m_filename2;

		int m_writeCounter;

		void SetupWalls();

		void SetupOrganisms();

		void SetupLightSources();

		void UpdateLighting();

		void InteractOrganisms();

		void MergeOrganisms(std::weak_ptr<Organism> org1, std::weak_ptr<Organism> org2, std::weak_ptr<Module> mod1, std::weak_ptr<Module> mod2, int side1, int side2);

		std::vector<std::shared_ptr<Organism>>::iterator KillOrganism(std::weak_ptr<Organism> org);

		std::vector<std::shared_ptr<Organism>>::iterator ReproduceOrganism(std::weak_ptr<Organism> org);

		//Vector of merges
		std::vector<std::shared_ptr<Merge>> m_merges;

		std::vector<std::shared_ptr<Organism>> m_organisms;

		std::vector<std::weak_ptr<Module>> m_modules;

		//Light sources
		std::vector<std::shared_ptr<LightSource>> m_lightSources;

		//Light timer 
		long double m_lightTimer;

		int m_lightingType;

		long double m_mainCounter;

		bool m_lightsOn;

		float m_lightRotationAngle;

		void DrawWalls(sf::RenderWindow& window);

		void DrawLightSources(sf::RenderWindow& window);

		void AddModule(sf::Vector2f pos);

		void AddSingleModuleOrganism(b2Vec2 pos, float rot, b2Vec2 vel, float angVel, int type);

		void AddCustomOrganism(std::map<std::pair<int, int>, std::shared_ptr<Module>> moduleMap, b2Vec2 position, float angle);

		time_t m_startTime;

		std::map<std::tuple<int, int, int, int, int>, int> m_species;

		std::stringstream eventWriteString;


};


#endif