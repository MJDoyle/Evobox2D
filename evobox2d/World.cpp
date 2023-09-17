#include "World.hpp"


World::World(unsigned long seed, char * outputName)
{
	//Set up world
	m_world = new b2World(b2Vec2(0, 0));

	m_outputName = outputName;

	m_createdOrganisms = 0;

	m_reproducedOrganisms = 0;

	m_lightTimer = 0;

	m_paused = false;

	m_writeCounter = 0;

	m_lightingType = BOTH;

	m_lightRotationAngle = 0;

	m_lightsOn = true;

	m_mainCounter = 0;

	b2BodyDef bodyDef;

	m_groundBody = m_world->CreateBody(&bodyDef);

	//Load font

	m_font.loadFromFile("Fonts/DejaVuSans.ttf");

	m_selectedOrgInfo.setFont(m_font);

	m_selectedOrgInfo.setColor(sf::Color::Black);

	m_selectedOrgInfo.setCharacterSize(16);

	m_selectedOrgInfo.setPosition(25, 25);

	m_FPSText.setFont(m_font);;

	m_FPSText.setColor(sf::Color::Black);

	m_FPSText.setCharacterSize(12);

	m_FPSText.setPosition(400, 25);


	m_simTimeText.setFont(m_font);

	m_simTimeText.setColor(sf::Color::Black);

	m_simTimeText.setCharacterSize(12);

	m_simTimeText.setPosition(550, 25);


	m_miscText.setFont(m_font);

	m_miscText.setColor(sf::Color::Black);

	m_miscText.setCharacterSize(12);

	m_miscText.setPosition(700, 25);



	//Set up output folder

	_mkdir(outputName);

	//Set up output files

	m_startTime = time(0);   // get time now
	struct tm * now = localtime( & m_startTime );

	m_filename << outputName;
	m_filename << "/State";

	m_filename << (now->tm_year + 1900) << '-' 
		<< (now->tm_mon + 1) << '-'
		<<  now->tm_mday << '-'
		<< now->tm_hour << '-'
		<< now->tm_min << '-'
		<< now->tm_sec
		<< ".csv";

	m_stateOutputFile.clear();

	m_filename2 << outputName;
	m_filename2 << "/Events";

	m_filename2 << (now->tm_year + 1900) << '-' 
		<< (now->tm_mon + 1) << '-'
		<<  now->tm_mday << '-'
		<< now->tm_hour << '-'
		<< now->tm_min << '-'
		<< now->tm_sec
		<< ".csv"; 

	m_eventOutputFile.clear();

	//Config file
	std::ofstream configFile;

	std::stringstream configFilename;

	configFilename << outputName << "/Config.txt";

	configFile.open(configFilename.str());

	configFile << "SEED: " << seed << std::endl;
	configFile << "CHG_RATE: " << CHARGING_RATE << std::endl;
	configFile << "PWR_DRAW: " << MODULE_POWER_DRAW << std::endl;
	configFile << "PRD_RATE: " << PREDATOR_RATE << std::endl;
	configFile << "MUT_RATE: " << MUTATION_RATE << std::endl;
	configFile << "PERPETUAL_ABIOGENESIS: " << PERPETUAL_ABIOGENESIS << std::endl;
	configFile << "STARTING MODULES: " << NUM_STARTING_ORGANISMS << std::endl;
	configFile << "PRED_PREY: " << PRED_PREY << std::endl;
	configFile << "BACKGROUND_LIGHT: " << BACKGROUND_LIGHT_INTENSITY << std::endl;
	configFile << "SOURCE_LIGHT: " << SOURCE_LIGHT_INTENSITY << std::endl;
	if (SOURCE_LIGHT_INTENSITY > 0)
	{
		configFile << "NUM_SOURCES: " << NUM_LIGHT_SOURCES << std::endl;
		configFile << "SOURCE_RADIUS: " << MAX_LIGHT_RADIUS << std::endl;
	}
	configFile << "WORLD_SIZE: " << WORLD_SIZE << std::endl;

	configFile.close();

	SetupWalls();

	SeedRand(seed);

	SetupOrganisms();

	if (NUM_LIGHT_SOURCES)
		SetupLightSources();

	m_FPS = 600;

	m_selectedOrganism = m_organisms.back();

	m_organisms.back()->SetSelected(true);

	m_mainClock.restart();

	std::stringstream ss;
}

void World::SetupWindow()
{
	m_window.create(sf::VideoMode(SCREEN_SIZE.x, SCREEN_SIZE.y), "EvoSimulator");

	sf::View view = m_window.getDefaultView();				//Center the camera and set the zoom
	view.setCenter(0, 0);
	view.zoom(2);
	m_window.setView(view);
}

void World::SetupOrganisms()
{
	if (!PRED_PREY)
	{
		for (int i = 0; i != NUM_STARTING_ORGANISMS; i ++)
		{
			int modType;

			if (i < NUM_STARTING_ORGANISMS / 3)
				modType = SOLAR;

			else if (i < 2 * NUM_STARTING_ORGANISMS / 3)
				modType = ANCHOR;

			else
				modType = BOUNDARY;

			int x = ((RandFloat() - 0.5) * (WORLD_SIZE - MODULE_SIZE));
			int y = ((RandFloat() - 0.5) * (WORLD_SIZE - MODULE_SIZE));

			AddSingleModuleOrganism(b2Vec2(x, y), 0, b2Vec2(0, 0), 0, modType);
		}
	}

	else
	{
		for (int i = 0; i != NUM_STARTING_ORGANISMS; i ++)
		{
			int modType;

			if (i < NUM_STARTING_ORGANISMS / 5)
				modType = SOLAR;

			else if (i < 2 * NUM_STARTING_ORGANISMS / 5)
				modType = ANCHOR;

			else if (i < 3 * NUM_STARTING_ORGANISMS / 5)
				modType = PREDATOR;

			else if (i < 4 * NUM_STARTING_ORGANISMS / 5)
				modType = ARMOUR;

			else
				modType = BOUNDARY;

			int x = ((RandFloat() - 0.5) * (WORLD_SIZE - MODULE_SIZE));
			int y = ((RandFloat() - 0.5) * (WORLD_SIZE - MODULE_SIZE));

			AddSingleModuleOrganism(b2Vec2(x, y), 0, b2Vec2(0, 0), 0, modType);
		}
	}



	/*AddSingleModuleOrganism(b2Vec2(-60, 0), 0, b2Vec2(1, 0), 0, BOUNDARY);

	AddSingleModuleOrganism(b2Vec2(-40, 0), 0, b2Vec2(1, 0), 0, SOLAR);

	AddSingleModuleOrganism(b2Vec2(-20, 0), 0, b2Vec2(0, 0), 0, ARMOUR);



	AddSingleModuleOrganism(b2Vec2(60, 0), 0, b2Vec2(-1, 0), 0, BOUNDARY);
	AddSingleModuleOrganism(b2Vec2(40, 0), 0, b2Vec2(0, 0), 0, PREDATOR);
*/




	/*AddSingleModuleOrganism(b2Vec2(-40, -20), 0, b2Vec2(1, 1), 0, BOUNDARY);

	AddSingleModuleOrganism(b2Vec2(-40, 20), 0, b2Vec2(1, -1), 0, PREDATOR);





	AddSingleModuleOrganism(b2Vec2(40, 1), 0, b2Vec2(-1, 0), 0, BOUNDARY);

	AddSingleModuleOrganism(b2Vec2(60, 1), 0, b2Vec2(-2, 0), 0, SOLAR);*/



	//AddSingleModuleOrganism(b2Vec2(40, -20), 0, b2Vec2(-10, 10), 0, BOUNDARY);

	//AddSingleModuleOrganism(b2Vec2(40, 20), 0, b2Vec2(-10, -10), 0, SOLAR);








	/*AddSingleModuleOrganism(b2Vec2(0, 0), 0, b2Vec2(0, 1), 0, BOUNDARY);

	AddSingleModuleOrganism(b2Vec2(0, 10), 0, b2Vec2(0, -1), 0, PREDATOR);

	AddSingleModuleOrganism(b2Vec2(10, -20), 0, b2Vec2(0, 1), 0, BOUNDARY);

	AddSingleModuleOrganism(b2Vec2(10, 0), 0, b2Vec2(0, 0), 0, SOLAR);

	AddSingleModuleOrganism(b2Vec2(10, 10), 0, b2Vec2(0, -1), 0, ARMOUR);*/


	/*AddSingleModuleOrganism(b2Vec2(0, 0), 3 * PI / 2, b2Vec2(0, 0), 0, SOLAR);

	AddSingleModuleOrganism(b2Vec2(0, 20), 0, b2Vec2(0, -0.2), 0, ANCHOR);

	AddSingleModuleOrganism(b2Vec2(0, -20), PI / 2, b2Vec2(0, 0), 0, BOUNDARY);*/

}

void World::SetupLightSources()
{
	for (int i = 0; i != NUM_LIGHT_SOURCES; i++)
	{
		////Random position
		b2Vec2 position;

		position.x = (RandFloat() * (WORLD_SIZE - 2 * MAX_LIGHT_RADIUS)) - (WORLD_SIZE / 2 - MAX_LIGHT_RADIUS);
		position.y = (RandFloat() * (WORLD_SIZE - 2 * MAX_LIGHT_RADIUS)) - (WORLD_SIZE / 2 - MAX_LIGHT_RADIUS);

		float radius = MAX_LIGHT_RADIUS;

		m_lightSources.push_back(std::shared_ptr<LightSource>(new LightSource(position, 100, radius)));
	}
}

void World::SetupWalls()
{
	//Set positions and sizes of walls. Push the wall positions back by half of their thickness' so that they don't overlap

	float positions[8] = {0, -WORLD_SIZE / 2 - WALL_THICKNESS / 2, 0, WORLD_SIZE / 2 + WALL_THICKNESS / 2, -WORLD_SIZE / 2 - WALL_THICKNESS / 2, 0, WORLD_SIZE / 2 + WALL_THICKNESS / 2, 0};

	float halfSizes[8] = {WORLD_SIZE / 2 , WALL_THICKNESS / 2, WORLD_SIZE / 2, WALL_THICKNESS / 2, WALL_THICKNESS / 2, WORLD_SIZE / 2, WALL_THICKNESS / 2, WORLD_SIZE / 2};

	for (int i = 0; i != 4; i ++)
	{
		//Define the ground body.
		b2BodyDef wallBodyDef;
		wallBodyDef.position.Set(positions[i * 2], positions[i * 2 + 1]);

		// Call the body factory which allocates memory for the ground body
		// from a pool and creates the ground box shape (also from a pool).
		// The body is also added to the world.
		m_walls[i] = m_world->CreateBody(&wallBodyDef);

		// Define the ground box shape.
		b2PolygonShape groundBox;

		// The extents are the half-widths of the box.
		groundBox.SetAsBox(halfSizes[i * 2], halfSizes[i * 2 + 1]);

		// Add the ground fixture to the ground body.
		m_walls[i]->CreateFixture(&groundBox, 0.0f);
	}
}

void World::UpdateLighting()
{
	//if (m_mainCounter > INITIAL_STATE_LENGTH)
		//m_lightingType = SOURCES;


	if (m_lightingType == FULL)
	{
	}

	else if (m_lightingType == DAY_NIGHT)
	{
		m_lightTimer -= TIME_STEP;

		if (m_lightTimer <= 0)
		{
			m_lightTimer = LIGHT_PERIOD;

			m_lightsOn = !m_lightsOn;
		}
	}

	else if (m_lightingType == ROTATING)
	{
		m_lightRotationAngle ++;

		if (m_lightRotationAngle >= 360)
			m_lightRotationAngle = 0;
	}

	if (NUM_LIGHT_SOURCES > 0)
	{
		/*m_lightTimer -= TIME_STEP;

		if (m_lightTimer <= 0)
		{
			m_lightTimer = LIGHT_PERIOD;

			m_lightSources.clear();

			SetupLightSources();
		}*/


		//Change position of a light source

		if (RandFloat() > (1 - 0.01 * TIME_STEP))
		{
			b2Vec2 position;

			position.x = (RandFloat() * (WORLD_SIZE - 2 * MAX_LIGHT_RADIUS)) - (WORLD_SIZE / 2 - MAX_LIGHT_RADIUS);
			position.y = (RandFloat() * (WORLD_SIZE - 2 * MAX_LIGHT_RADIUS)) - (WORLD_SIZE / 2 - MAX_LIGHT_RADIUS);

			m_lightSources[int(RandFloat() * NUM_LIGHT_SOURCES)]->m_position = position;
		}


	}
}

bool World::Update()	//Main update function - everything happens here
{
	if (!m_paused)	//Only update if the simulator isn't paused
	{



		//DEBUG

		/*for (auto organism = m_organisms.begin(); organism != m_organisms.end(); organism ++)
		{
			if (std::abs((*organism)->GetBody()->GetLinearVelocity().x) > 100 || std::abs((*organism)->GetBody()->GetLinearVelocity().y) > 100)
			{
				std::cout << "TOO FAST! " << (*organism)->GetBody()->GetPosition().x << " " << (*organism)->GetBody()->GetPosition().y << " " << (*organism)->GetBody()->GetLinearVelocity().x << " " << (*organism)->GetBody()->GetLinearVelocity().y << std::endl;

				m_paused = true;
			}
		}*/





		WriteState();	//Write out data to file

		for (auto notIt = m_notifications.begin(); notIt != m_notifications.end(); )	//Update notifications (on-screen messages)
		{
			if (!(*notIt)->Update())
				notIt = m_notifications.erase(notIt);

			else
				notIt ++;
		}

		//Update internals and add forces to organisms
		for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); )
		{
			//If an organism has run out of energy, kill it
			if (!(*orgIt)->Update(m_mainCounter))
				orgIt = KillOrganism(*orgIt);

			else
				orgIt ++;
		}


		UpdateLighting();	//Update lighting

		//////////
		//
		//	CHARGING
		//
		//////////


		for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); orgIt ++)	//Charge organisms (harvest energy)							
		{
			//if ((*orgIt)->GetStatus() != DEAD)	//Don't charge DEAD organisms (ie. individual modules)
			if (true)
			{ 
				auto modules = (*orgIt)->GetModules();

				for (auto modIt = modules.begin(); modIt != modules.end(); modIt ++)	//Iterate through each module in the organism
				{
					if (modIt->second->GetType() == SOLAR)	//Only solar modules can charge
					{
						if (m_lightingType == FULL || (m_lightingType == DAY_NIGHT && m_lightsOn) || BACKGROUND_LIGHT_INTENSITY > 0)	//If light covers entire environment, modules charge in any position
						{
							(*orgIt)->Charge(CHARGING_RATE * BACKGROUND_LIGHT_INTENSITY);
						}

						else if (m_lightingType == QUADRANT) //Light covers bottom right corner
						{
							if ((*orgIt)->GetBody()->GetWorldPoint(b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE)).x > 0 && (*orgIt)->GetBody()->GetWorldPoint(b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE)).y > 0)
							{
								(*orgIt)->Charge(CHARGING_RATE * 4);
							}
						}

						if (NUM_LIGHT_SOURCES > 0)	//If not, check if modules are within a light source, and charge them if they are
						{
							for (auto lightIt = m_lightSources.begin(); lightIt != m_lightSources.end(); lightIt ++)
							{
								b2Vec2 distanceToLight = (*lightIt)->m_position - (*orgIt)->GetBody()->GetWorldPoint(b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE));

								if (distanceToLight.Length() <= (*lightIt)->m_radius)
								{
									(*orgIt)->Charge(CHARGING_RATE * SOURCE_LIGHT_INTENSITY);
									//(*orgIt)->Charge(CHARGING_RATE * 8.136398);
								}
							}
						}
					}
				}
			}
		}


		//////////
		//
		//	ANCHORS
		//
		//////////

		for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); orgIt ++)	//Activate anchors
		{
			//Only use anchors if alive 
			if ((*orgIt)->GetStatus() != DEAD)	//Don't use anchors of DEAD organisms (ie. individual modules)
			{ 

				//SET ANCHORS DUE TO LIGHT

				if ((*orgIt)->GetModuleTypeNumber(SOLAR) > 0)
				{
					auto modules = (*orgIt)->GetModules();

					for (auto modIt = modules.begin(); modIt != modules.end(); modIt ++)
					{
						if (modIt->second->GetType() == ANCHOR)
						{
							if (m_lightingType == FULL || (m_lightingType == DAY_NIGHT && m_lightsOn) || BACKGROUND_LIGHT_INTENSITY > 0)	//If light covers entire environment, use anchors in any position
							{
								(*orgIt)->AddActiveAnchor();
							}

							else if (m_lightingType == QUADRANT) //Light covers bottom right corner
							{
								if ((*orgIt)->GetBody()->GetWorldPoint(b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE)).x > 0 && (*orgIt)->GetBody()->GetWorldPoint(b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE)).y > 0)
								{
									(*orgIt)->AddActiveAnchor();
								}
							}

							else if (m_lightingType == SOURCES || NUM_LIGHT_SOURCES > 0)	//If not, check if modules are within a light source, and use anchors if they are
							{
								for (auto lightIt = m_lightSources.begin(); lightIt != m_lightSources.end(); lightIt ++)
								{
									b2Vec2 distanceToLight = (*lightIt)->m_position - (*orgIt)->GetBody()->GetWorldPoint(b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE));

									if (distanceToLight.Length() <= (*lightIt)->m_radius)
									{
										(*orgIt)->AddActiveAnchor();
									}
								}
							}
						}
					}
				}


				//SET ANCHORS DUE TO PREDATION

				if ((*orgIt)->GetModuleTypeNumber(PREDATOR) > 0)
				{
					int numAnchorsInactive = (*orgIt)->GetModuleTypeNumber(ANCHOR) - (*orgIt)->GetActiveAnchors();

					if ((*orgIt)->GetIsParasite())
					{
						for (int i = 0; i != numAnchorsInactive; i++)
							(*orgIt)->AddActiveAnchor();
					}

				}
			}
		}

		//Reproduce organisms - split organisms that have accreted all of their child modules into two
		for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); )
		{
			if ((*orgIt)->GetReproduceNow())
				orgIt = ReproduceOrganism(*orgIt);

			else
				orgIt ++;
		}

		//Check for interactions between organisms and decide whether to merge them
		InteractOrganisms();

		//Merge organisms that need merging
		for (auto mergeIt = m_merges.begin(); mergeIt != m_merges.end(); mergeIt ++)
			MergeOrganisms((*mergeIt)->m_org1, (*mergeIt)->m_org2, (*mergeIt)->m_mod1,(*mergeIt)->m_mod2, (*mergeIt)->m_side1, (*mergeIt)->m_side2);

		m_merges.clear();

		m_world->Step(TIME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

		//m_mainCounter += TIME_STEP;

		m_mainCounter ++;
	}


	//Count number of reproducing organisms

	int numCompleteOrganisms = 0;

	for (auto organism = m_organisms.begin(); organism != m_organisms.end(); organism++)
	{
		if ((*organism)->GetStatus() == REPRODUCING)
			numCompleteOrganisms ++;
	}
		


	if (int(m_mainCounter * TIME_STEP) % 100 == 0)
		std::cout << m_mainCounter * TIME_STEP << std::endl;

	if (m_mainCounter * TIME_STEP > MAX_TIME)
	{
		std::cout << "MAX TIME REACHED" << std::endl;

		getch();

		return false;
	}

	else if (m_mainCounter * TIME_STEP > 3600 && numCompleteOrganisms == 0 && !PERPETUAL_ABIOGENESIS)
	{
		std::cout << "ALL ORGANISMS DEAD" << std::endl;

		getch();
		return false;
	}

	else
		return true;
}

std::vector<std::shared_ptr<Organism>>::iterator World::ReproduceOrganism(std::weak_ptr<Organism> org)
{
	//Disconnect hosts/parasites, allow them to reconnect if they can

	if (org.lock()->GetIsHost())
	{
		std::vector<std::weak_ptr<Organism>> parasites = org.lock()->GetParasites();

		for (auto parasite = parasites.begin(); parasite != parasites.end(); parasite ++)
		{
			parasite->lock()->SetIsParasite(false);
			parasite->lock()->ResetHost();
			parasite->lock()->ResetParasiteJoint();
		}
	}

	if (org.lock()->GetIsParasite())
	{
		org.lock()->GetHost().lock()->RemoveParasite(org);

		if (org.lock()->GetHost().lock()->GetParasites().size() == 0)
			org.lock()->GetHost().lock()->SetIsHost(false);
			
	}


	/*std::cout << "REPRODUCING" << std::endl;

	getch();*/

	m_reproducedOrganisms ++;

	long long parentID = org.lock()->GetID();
	long long parentsParentID = org.lock()->GetParentID();


	//Create two new organisms, one with the child modules and one with the other modules, split energy between them

	//First child organism

	std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> childModules = org.lock()->GetChildModules();

	std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> childModuleMap;

	std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> parentModules = org.lock()->GetParentModules();

	std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> parentModuleMap;

	//Move child modules into new module map
	for (auto module = childModules.begin(); module != childModules.end(); module ++)
		childModuleMap[module->first] = module->second.lock();

	//Calculate new position of child organism
	sf::Vector2f childPosition(0, 0);

	for (auto module = childModuleMap.begin(); module != childModuleMap.end(); module ++)
	{
		b2Vec2 modPos = org.lock()->GetBody()->GetWorldPoint(b2Vec2(module->second->GetComRelPosition().x * MODULE_SIZE, module->second->GetComRelPosition().y * MODULE_SIZE));

		childPosition += sf::Vector2f(modPos.x, modPos.y);
	}

	childPosition.x /= float(childModuleMap.size());
	childPosition.y /= float(childModuleMap.size());
	
	//Empty growth plan - no longer used anyway
	std::map<sf::Vector2i, int, Vector2iCompare> emptyPlan;

	m_organisms.push_back(std::shared_ptr<Organism>(new Organism(	*m_world, 
																	m_groundBody,
																	b2Vec2(childPosition.x, childPosition.y),
																	//org.lock()->GetBody()->GetWorldPoint(b2Vec2(parentModules.begin()->first.x * MODULE_SIZE, parentModules.begin()->first.y * MODULE_SIZE)),
																	//org.lock()->GetBody()->GetWorldPoint(b2Vec2(childModules.begin()->first.x * MODULE_SIZE, childModules.begin()->first.y * MODULE_SIZE)), 
																	org.lock()->GetBody()->GetAngle(), 

																	org.lock()->GetBody()->GetLinearVelocity(),
																	//org.lock()->GetBody()->GetLinearVelocityFromLocalPoint(b2Vec2(childModules.begin()->first.x * MODULE_SIZE, childModules.begin()->first.y * MODULE_SIZE)), 
																	org.lock()->GetBody()->GetAngularVelocity(), 
																	childModuleMap, 
																	org.lock()->GetEnergy() / 2,
																	emptyPlan,
																	REPRODUCING,
																	m_startTime)));

	//Set pointers to organism on modules
	for (auto module = childModules.begin(); module != childModules.end(); module ++)
		module->second.lock()->SetOrganism(m_organisms.back());

	m_organisms.back()->SetID(NEW_ORGANISM_ID++);
	m_organisms.back()->SetParentID(parentID);

	m_organisms.back()->SetGeneration(org.lock()->GetGeneration() + 1);

	//Add new organism to write stream
	eventWriteString << BIRTH << ',' 
		<< m_organisms.back()->GetID() << ',' 
		<< m_organisms.back()->GetParentID() << ','
		<< m_mainCounter << ','
		<< m_organisms.back()->GetModuleTypeNumber(SOLAR) << ','
		<< m_organisms.back()->GetModuleTypeNumber(ANCHOR) << ','
		<< m_organisms.back()->GetModuleTypeNumber(PREDATOR) << ','
		<< m_organisms.back()->GetModuleTypeNumber(ARMOUR) << ','
		<< '\n';

	//Now parent organism


	/*std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> parentModules = org.lock()->GetParentModules();

	std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> parentModuleMap;*/

	//Move parent modules into new module map
	for (auto module = parentModules.begin(); module != parentModules.end(); module ++)
		parentModuleMap[module->first] = module->second.lock();

	//Calculate new position of child organism
	sf::Vector2f parentPosition(0, 0);

	for (auto module = parentModuleMap.begin(); module != parentModuleMap.end(); module ++)
	{
		b2Vec2 modPos = org.lock()->GetBody()->GetWorldPoint(b2Vec2(module->second->GetComRelPosition().x * MODULE_SIZE, module->second->GetComRelPosition().y * MODULE_SIZE));

		parentPosition += sf::Vector2f(modPos.x, modPos.y);
	}

	parentPosition.x /= float(parentModuleMap.size());
	parentPosition.y /= float(parentModuleMap.size());

	m_organisms.push_back(std::shared_ptr<Organism>(new Organism(	*m_world, 
																	m_groundBody,
																	//org.lock()->GetBody()->GetWorldPoint(b2Vec2(childModules.begin()->first.x * MODULE_SIZE, childModules.begin()->first.y * MODULE_SIZE)),
																	//org.lock()->GetBody()->GetWorldPoint(b2Vec2(parentModules.begin()->first.x * MODULE_SIZE, parentModules.begin()->first.y * MODULE_SIZE)),
																	b2Vec2(parentPosition.x, parentPosition.y),
																	org.lock()->GetBody()->GetAngle(), 
																	org.lock()->GetBody()->GetLinearVelocity(),
																	//org.lock()->GetBody()->GetLinearVelocityFromLocalPoint(b2Vec2(parentModules.begin()->first.x * MODULE_SIZE, parentModules.begin()->first.y * MODULE_SIZE)), 
																	org.lock()->GetBody()->GetAngularVelocity(), 
																	parentModuleMap, 
																	org.lock()->GetEnergy() / 2,
																	emptyPlan,
																	REPRODUCING,
																	m_startTime)));



	//Set pointers to organism on modules
	for (auto module = parentModules.begin(); module != parentModules.end(); module ++)
		module->second.lock()->SetOrganism(m_organisms.back());

	//Set correct birth type
	m_organisms.back()->SetBirthType(org.lock()->GetBirthType());

	m_organisms.back()->SetID(parentID);
	m_organisms.back()->SetParentID(parentsParentID);

	m_organisms.back()->SetGeneration(org.lock()->GetGeneration());






	//Then kill the original organism
	for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); orgIt ++)
	{
		if (org.lock() == *orgIt)
		{
			return m_organisms.erase(orgIt);
		}
	}
}

std::vector<std::shared_ptr<Organism>>::iterator World::KillOrganism(std::weak_ptr<Organism> org)
{
	//This returns an iterator to the nest organism in the list for the world::update function to use

	eventWriteString << DEATH << ',' 
		<< org.lock()->GetID() << ',' 
		<< m_mainCounter << ','
		<< '\n';

	//Set correct host and parasite info

	if (org.lock()->GetIsHost())
	{
		std::vector<std::weak_ptr<Organism>> parasites = org.lock()->GetParasites();

		for (auto parasite = parasites.begin(); parasite != parasites.end(); parasite ++)
		{
			parasite->lock()->SetIsParasite(false);
			parasite->lock()->ResetHost();
			parasite->lock()->ResetParasiteJoint();
		}
	}

	if (org.lock()->GetIsParasite())
	{
		org.lock()->GetHost().lock()->RemoveParasite(org);

		if (org.lock()->GetHost().lock()->GetParasites().size() == 0)
			org.lock()->GetHost().lock()->SetIsHost(false);
			
	}


	//For every module in this organism, create a new organism in the same place
	auto modules = org.lock()->GetModules();

	for (auto modIt = modules.begin(); modIt != modules.end(); modIt ++)
	{
		//The modules of the dying organism are reused for the new single module organisms. That way they keep any informatino they need to.
		//Need to reset some of their properties (position within the organism for example)

		modIt->second->SetPosition(sf::Vector2i(0, 0));

		std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> moduleMap;

		moduleMap[sf::Vector2i(0, 0)] = modIt->second;

		//TODO - the angular velocity you give the new organisms is incorrect (check how the old version did it)

		//Dead organism gets empty plan
		std::map<sf::Vector2i, int, Vector2iCompare> emptyPlan;

		emptyPlan[sf::Vector2i(0, 0)] = modIt->second->GetType();

		m_organisms.push_back(std::shared_ptr<Organism>(new Organism(	*m_world, 
																		m_groundBody,
																		org.lock()->GetBody()->GetWorldPoint(b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE)), 
																		org.lock()->GetBody()->GetAngle(), 
																		org.lock()->GetBody()->GetLinearVelocityFromLocalPoint(b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE)), 
																		0, 
																		moduleMap, 
																		0,
																		emptyPlan,
																		DEAD,
																		m_startTime)));

		modIt->second->SetOrganism(m_organisms.back());
	}

	//Then kill this organism
	for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); orgIt ++)
	{
		if (org.lock() == *orgIt)
		{
			return m_organisms.erase(orgIt);
		}
	}
}


//MERGE ORGANISMS
//
//Here, organisms that have been flagged to merge are merged. Modules are removed from one organism (secondary) and added to the other organism (primary) at the correct rotation and map position. Secondary organism is then removed
//
void World::MergeOrganisms(std::weak_ptr<Organism> org1, std::weak_ptr<Organism> org2, std::weak_ptr<Module> mod1, std::weak_ptr<Module> mod2, int side1, int side2)
{
	//std::cout << "MERGE: " << org1.lock()->GetBody()->GetPosition().x << " " << org1.lock()->GetBody()->GetPosition().y << " " << org2.lock()->GetBody()->GetPosition().x << " " << org2.lock()->GetBody()->GetPosition().y << " " << org1.lock()->GetModules().size() << " " << org2.lock()->GetModules().size() << " " << side1 << " " << side2 << std::endl;

	//{
	//	std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> mods1 = org1.lock()->GetModules();

	//	/*for (auto mod = mods1.begin(); mod != mods1.end(); mod ++)
	//	{
	//		std::cout << mod->first.x << " "  << mod->first.y << "      ";
	//	}*/

	//	std::cout << std::endl << mod1.lock()->GetPosition().x << " " << mod1.lock()->GetPosition().y << std::endl;



	//	std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> mods2 = org2.lock()->GetModules();

	//	for (auto mod = mods2.begin(); mod != mods2.end(); mod ++)
	//	{
	//		std::cout << mod->first.x << " "  << mod->first.y << "      ";
	//	}





	//	std::cout << std::endl << mod2.lock()->GetPosition().x << " " << mod2.lock()->GetPosition().y << std::endl;

	//	std::map<sf::Vector2i, int, Vector2iCompare> aPoints1 = org1.lock()->GetAttachmentPoints();

	//	for (auto aPoint = aPoints1.begin(); aPoint != aPoints1.end(); aPoint ++)
	//	{
	//		std::cout << aPoint->first.x << " " << aPoint->first.y << "       ";
	//	}

	//	std::cout << std::endl;




	//	std::map<sf::Vector2i, int, Vector2iCompare> aPoints2 = org2.lock()->GetAttachmentPoints();

	//	for (auto aPoint = aPoints2.begin(); aPoint != aPoints2.end(); aPoint ++)
	//	{
	//		std::cout << aPoint->first.x << " " << aPoint->first.y << "       ";
	//	}

	//	std::cout << std::endl;

	//}

	//Select largest organism as the 'primary' organism that accretes the smaller organism

	std::shared_ptr<Organism> priOrg;
	std::shared_ptr<Organism> secOrg;

	std::shared_ptr<Module> priMod;
	std::shared_ptr<Module> secMod;

	int priSide;
	int secSide;


	if (org1.lock()->GetStatus() == DEAD)
	{
		priOrg = org2.lock();
		secOrg = org1.lock();

		priMod = mod2.lock();
		secMod = mod1.lock();

		priSide = side2;
		secSide = side1;
	}

	else if (org2.lock()->GetStatus() == DEAD)
	{
		priOrg = org1.lock();
		secOrg = org2.lock();

		priMod = mod1.lock();
		secMod = mod2.lock();

		priSide = side1;
		secSide = side2;
	}

	else if (org1.lock()->GetModules().size() >= org2.lock()->GetModules().size())
	{
		priOrg = org1.lock();
		secOrg = org2.lock();

		priMod = mod1.lock();
		secMod = mod2.lock();

		priSide = side1;
		secSide = side2;
	}

	else
	{
		priOrg = org2.lock();
		secOrg = org1.lock();

		priMod = mod2.lock();
		secMod = mod1.lock();

		priSide = side2;
		secSide = side1;
	}


	//std::cout << "MERGE: Body1: " << priOrg->GetBody()->GetPosition().x << " " << priOrg->GetBody()->GetPosition().y << " Body2: " << secOrg->GetBody()->GetPosition().x << " " << secOrg->GetBody()->GetPosition().y << std::endl;

	//std::cout << 

	int priOrgPreviousStatus = priOrg->GetStatus();

	//Now rotate secOrg's module map coordinate system so it matches that of priOrg

	//If the connecting sides are opposite (ie. top - bottom or left - right) the coordinate systems are already aligned

	if ((priSide + 2) % 4 == secSide)
	{
	}

	//If the connecting sides are the same (eg. top - top etc.) then secOrg's coordinate system needs to be rotated 180 degrees

	else if (priSide == secSide)
	{
		auto secOrgMods = secOrg->GetModules();

		for (auto modIt = secOrgMods.begin(); modIt != secOrgMods.end(); modIt ++)
		{
			modIt->second->SetPosition(sf::Vector2i(modIt->first.x * -1, modIt->first.y * -1));
		}
	}

	//If the connecting side of secOrg is 1 greater than the connecting side of priOrg, rotate secOrg by 90

	else if ((priSide + 1) % 4 == secSide)
	{
		auto secOrgMods = secOrg->GetModules();

		for (auto modIt = secOrgMods.begin(); modIt != secOrgMods.end(); modIt ++)
		{
			modIt->second->SetPosition(sf::Vector2i(modIt->first.x * -1, modIt->first.y * 1));
		}
	}

	//If the connecting side of secOrg is 1 less than the connecting side of priOrg, rotate secOrg by 270

	else if ((secSide + 1) % 4 == priSide)
	{
		auto secOrgMods = secOrg->GetModules();

		for (auto modIt = secOrgMods.begin(); modIt != secOrgMods.end(); modIt ++)
		{
			modIt->second->SetPosition(sf::Vector2i(modIt->first.x * 1, modIt->first.y * -1));
		}
	}

	//Remember in the previous section you changed position that each module object holds, but didn't change the modules' positions in the module map

	//Now need to offset the position of each of secOrg's modules to fit it's map with that of priOrg

	sf::Vector2i secModNewPos;

	if (priSide == TOP)
		secModNewPos = priMod->GetPosition() + sf::Vector2i(0, -1);

	else if (priSide == RIGHT)
		secModNewPos = priMod->GetPosition() + sf::Vector2i(1, 0);

	else if (priSide == BOTTOM)
		secModNewPos = priMod->GetPosition() + sf::Vector2i(0, 1);

	else if (priSide == LEFT)
		secModNewPos = priMod->GetPosition() + sf::Vector2i(-1, 0);


	//Set velocity of priOrg as a combination of priOrg and secOrg velocities (conservationf of momentum)

	b2Vec2 priVel = priOrg->GetBody()->GetLinearVelocity();

	b2Vec2 secVel = secOrg->GetBody()->GetLinearVelocity();

	float priSize = priOrg->GetTotalSize();

	float secSize = secOrg->GetTotalSize();

	b2Vec2 totalMom = priSize * priVel + secSize * secVel;

	//std::cout << "MOM: " << totalMom.x << " " << totalMom.y << std::endl;

	b2Vec2 totalVel = float(1)/(priSize + secSize) * totalMom;

	//std::cout << "VEL: " << totalVel.x << " " << totalVel.y << std::endl;

	priOrg->GetBody()->SetLinearVelocity(totalVel);

	//std::cout << "MERGE: " << priOrg->GetEnergy() << " " << secOrg->GetEnergy() << std::endl;

	//Calculate the offset

	sf::Vector2i secModOffset = secModNewPos - secMod->GetPosition();

	//Now go through all the secMods and update their positions

	auto secOrgMods = secOrg->GetModules();

	for (auto modIt = secOrgMods.begin(); modIt != secOrgMods.end(); modIt ++)
	{
		modIt->second->SetPosition(modIt->second->GetPosition() + secModOffset);
	}

	//Now add all of secOrgs modules to priOrg in the correct position (and give the modules the correct new organism)

	for (auto modIt = secOrgMods.begin(); modIt != secOrgMods.end(); modIt ++)
	{
		//std::cout << "Adding module at: " << modIt->second->GetPosition().x << " " << modIt->second->GetPosition().y << std::endl;

		priOrg->AddModule(modIt->second);

		modIt->second->SetOrganism(priOrg);
	}


	//std::cout << "Resultant: " << priOrg->GetBody()->GetPosition().x << " " << priOrg->GetBody()->GetPosition().y << std::endl;


	//Add organism internals (energy etc.) to new organism

	priOrg->AddMaxEnergy(secOrg->GetMaxEnergy());
	priOrg->AddEnergy(secOrg->GetEnergy());


	//Now recenter priOrg's updated moduleMap

	//Is this neccesary or is it ok to keep the 0, 0 off center? TO CHECK

	//priOrg->CenterMap();

	priOrg->SetMerging(false);

	//Now delete secOrg
	for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); orgIt ++)
	{
		if (secOrg == *orgIt)
		{
			m_organisms.erase(orgIt);
			break;
		}
	}


	//If priOrg is now reproducing and wasn't before, a new organism has been created
	if (priOrgPreviousStatus != REPRODUCING && priOrg->GetStatus() == REPRODUCING)
	{
		m_createdOrganisms++;

		priOrg->SetBirthType(CREATED);

		priOrg->SetID(NEW_ORGANISM_ID++);

		eventWriteString << CREATION << ',' 
		<< priOrg->GetID() << ',' 
		<< priOrg->GetParentID() << ','
		<< m_mainCounter << ','
		<< priOrg->GetModuleTypeNumber(SOLAR) << ','
		<< priOrg->GetModuleTypeNumber(ANCHOR) << ','
		<< priOrg->GetModuleTypeNumber(PREDATOR) << ','
		<< priOrg->GetModuleTypeNumber(ARMOUR) << ','
		<< '\n';
	}


	//std::cout << "MERGED: " << priOrg->GetEnergy() << std::endl << std::endl;
}

void World::InteractOrganisms()
{
	//Go through each organism and check if every module side is in range of a module side of another organism

	for (auto org1It = m_organisms.begin(); org1It != m_organisms.end(); org1It ++)
	{
		for (auto org2It = m_organisms.begin(); org2It != m_organisms.end(); org2It ++)
		{
			//Don't check an organism against itself
			if (org1It != org2It)
			{
				//For each pair of organisms, check each module

				auto org1Modules = (*org1It)->GetModules();

				auto org2Modules = (*org2It)->GetModules();


				//OLD PREDATION AT RANGE FUNCTION
				//if ((*org1It)->GetTotalSize() > 1 && (*org1It)->GetModuleTypeNumber(PREDATOR) > 0 && (*org2It)->GetModuleTypeNumber(SOLAR) > 0 && (*org2It)->GetStatus() == REPRODUCING)
				//{
				//	if (((*org1It)->GetBody()->GetPosition() - (*org2It)->GetBody()->GetPosition()).Length() <= PREDATOR_RANGE)
				//	{
				//		float energy = std::max(float(0), (*org1It)->GetModuleTypeNumber(PREDATOR) * ((*org2It)->GetModuleTypeNumber(SOLAR) - (*org2It)->GetModuleTypeNumber(ARMOUR)) * PREDATOR_RATE * TIME_STEP);

				//		(*org1It)->AddEnergy(std::min(energy, (*org2It)->GetEnergy()));
				//										
				//		(*org2It)->RemoveEnergy(energy);
				//	}
				//}

				for (auto mod1It = org1Modules.begin(); mod1It != org1Modules.end(); mod1It ++)
				{
					for (auto mod2It = org2Modules.begin(); mod2It != org2Modules.end(); mod2It ++)
					{
						//Go through each pair of sides of each module and check whether they are within connection range of one another
						for (int i = 0; i != NUM_SIDES; i ++)
						{
							for (int j = 0; j != NUM_SIDES; j ++)
							{
								bool externalFace = true;

								if (i == TOP && (*org1It)->HasModuleHere(mod1It->first + sf::Vector2i(0, -1)))
									externalFace = false;

								else if (i == BOTTOM && (*org1It)->HasModuleHere(mod1It->first + sf::Vector2i(0, 1)))
									externalFace = false;

								else if (i == LEFT && (*org1It)->HasModuleHere(mod1It->first + sf::Vector2i(-1, 0)))
									externalFace = false;

								else if (i == RIGHT && (*org1It)->HasModuleHere(mod1It->first + sf::Vector2i(1, 0)))
									externalFace = false;

								if (j == TOP && (*org2It)->HasModuleHere(mod2It->first + sf::Vector2i(0, -1)))
									externalFace = false;

								else if (j == BOTTOM && (*org2It)->HasModuleHere(mod2It->first + sf::Vector2i(0, 1)))
									externalFace = false;

								else if (j == LEFT && (*org2It)->HasModuleHere(mod2It->first + sf::Vector2i(-1, 0)))
									externalFace = false;

								else if (j == RIGHT && (*org2It)->HasModuleHere(mod2It->first + sf::Vector2i(1, 0)))
									externalFace = false;

								float deltaAngle = int(std::abs((*org1It)->GetBody()->GetAngle() - (*org2It)->GetBody()->GetAngle()) * float(180) / PI) % 90;




								//Check range and orientation
								if (((*org1It)->GetSidePosition(i, mod1It->second->GetComRelPosition()) - (*org2It)->GetSidePosition(j, mod2It->second->GetComRelPosition())).Length() <= CONNECTION_RANGE && externalFace
									&& deltaAngle < 5)
								{

									//Check that organisms want to interact

									//These are the rules that determine if organisms want to connect to one another

									//Deprecated rules are commented below


									//
									//	TWO INDIVIDUAL MODULES CONNECT
									//

									//Both organisms must have a single module, at least one of them must have some minimum energy, and no more than one of them can be a boundary module

									if ((*org1It)->GetTotalSize() == 1 && (*org2It)->GetTotalSize() == 1 && ((*org1It)->GetEnergy() > (*org1It)->GetMaxEnergy() * ABIOGENESIS_ENERGY_REQUIREMENT || (*org2It)->GetEnergy() > (*org2It)->GetMaxEnergy() * ABIOGENESIS_ENERGY_REQUIREMENT) && (mod1It->second->GetType() != BOUNDARY || mod2It->second->GetType() != BOUNDARY))
									//if ((*org1It)->GetTotalSize() == 1 && (*org2It)->GetTotalSize() == 1 && ((*org1It)->GetEnergy() > 0 || (*org2It)->GetEnergy() > 0) && (mod1It->second->GetType() != BOUNDARY || mod2It->second->GetType() != BOUNDARY))
									{
										
										//Check organisms aren't already merging (to something else), and if not add them to merge list to be merged
										if (!(*org1It)->GetMerging() && !(*org2It)->GetMerging())
										{
											//Also check that modules aren't inside one another
											if (((*org1It)->GetModulePosition(mod1It->second->GetComRelPosition()) - (*org2It)->GetModulePosition(mod2It->second->GetComRelPosition())).Length() >= MODULE_SIZE)
											{
												m_merges.push_back(std::shared_ptr<Merge>(new Merge(*org1It, *org2It, mod1It->second, mod2It->second, i, j)));

												(*org1It)->SetMerging(true);
												(*org2It)->SetMerging(true);
											}
										}
									}


									//
									//	AN INDIVIDUAL MODULE AND A POLYMER (NO BOUNDARY) CONNECT
									//

									else if ((*org1It)->GetTotalSize() == 1 && (*org2It)->GetStatus() == GROWING)
									{
										//Check that the polymer has an attachment point in this position, if so merge
										bool merge = false;

										std::map<sf::Vector2i, int, Vector2iCompare> attachmentPoints = (*org2It)->GetAttachmentPoints();

										for (auto pointIt = attachmentPoints.begin(); pointIt != attachmentPoints.end(); pointIt ++)
										{
											sf::Vector2i attachmentPoint = mod2It->first;

											if (j == TOP)
												attachmentPoint += sf::Vector2i(0, -1);

											else if (j == RIGHT)
												attachmentPoint += sf::Vector2i(1, 0);

											else if (j == BOTTOM)
												attachmentPoint += sf::Vector2i(0, 1);

											else if (j == LEFT)
												attachmentPoint += sf::Vector2i(-1, 0);

											if (attachmentPoint == pointIt->first)
												merge = true;
										}

										if (merge)
										{
											//Check organisms aren't already merging (to something else), and if not add them to merge list to be merged
											if (!(*org1It)->GetMerging() && !(*org2It)->GetMerging())
											{
												m_merges.push_back(std::shared_ptr<Merge>(new Merge(*org1It, *org2It, mod1It->second, mod2It->second, i, j)));

												(*org1It)->SetMerging(true);
												(*org2It)->SetMerging(true);
											}
										}
									}

									else if ((*org2It)->GetTotalSize() == 1 && (*org1It)->GetStatus() == GROWING)
									{
										//Check that the polymer has an attachment point in this position, if so merge
										bool merge = false;

										std::map<sf::Vector2i, int, Vector2iCompare> attachmentPoints = (*org1It)->GetAttachmentPoints();

										for (auto pointIt = attachmentPoints.begin(); pointIt != attachmentPoints.end(); pointIt ++)
										{
											sf::Vector2i attachmentPoint = mod2It->first;

											if (i == TOP)
												attachmentPoint += sf::Vector2i(0, -1);

											else if (i == RIGHT)
												attachmentPoint += sf::Vector2i(1, 0);

											else if (i == BOTTOM)
												attachmentPoint += sf::Vector2i(0, 1);

											else if (i == LEFT)
												attachmentPoint += sf::Vector2i(-1, 0);

											if (attachmentPoint == pointIt->first)
												merge = true;
										}

										if (merge)
										{
											//Check organisms aren't already merging (to something else), and if not add them to merge list to be merged
											if (!(*org1It)->GetMerging() && !(*org2It)->GetMerging())
											{
												m_merges.push_back(std::shared_ptr<Merge>(new Merge(*org1It, *org2It, mod1It->second, mod2It->second, i, j)));

												(*org1It)->SetMerging(true);
												(*org2It)->SetMerging(true);
											}
										}
									}


									//
									//	AN INDIVIDUAL MODULE AND A ORGANISM (HAS BOUNDARY) CONNECT
									//

									else if ((*org1It)->GetTotalSize() == 1 && (*org2It)->GetStatus() == REPRODUCING)
									{
										//Check that the polymer has an attachment point in this position, if so merge
										bool merge = false;

										std::map<sf::Vector2i, int, Vector2iCompare> attachmentPoints = (*org2It)->GetAttachmentPoints();

										for (auto pointIt = attachmentPoints.begin(); pointIt != attachmentPoints.end(); pointIt ++)
										{
											sf::Vector2i attachmentPoint = mod2It->first;

											if (j == TOP)
												attachmentPoint += sf::Vector2i(0, -1);

											else if (j == RIGHT)
												attachmentPoint += sf::Vector2i(1, 0);

											else if (j == BOTTOM)
												attachmentPoint += sf::Vector2i(0, 1);

											else if (j == LEFT)
												attachmentPoint += sf::Vector2i(-1, 0);

											if (attachmentPoint == pointIt->first)
											{
												//Module can be accreted if it is the correct type, or an inccorect module type (not boundary, or to a boundary) can be accreted with a small mutation chance
												if ((pointIt->second == ANY && (mod1It->second->GetType() != BOUNDARY)) || (pointIt->second == mod1It->second->GetType()) || (mod1It->second->GetType() != BOUNDARY && mod2It->second->GetType() != BOUNDARY && RandFloat() < MUTATION_RATE))
												{
													merge = true;
												}
											}
										}

										if (merge)
										{
											//Check organisms aren't already merging (to something else), and if not add them to merge list to be merged
											if (!(*org1It)->GetMerging() && !(*org2It)->GetMerging())
											{
												m_merges.push_back(std::shared_ptr<Merge>(new Merge(*org1It, *org2It, mod1It->second, mod2It->second, i, j)));

												(*org1It)->SetMerging(true);
												(*org2It)->SetMerging(true);
											}
										}
									}

									else if ((*org2It)->GetTotalSize() == 1 && (*org1It)->GetStatus() == REPRODUCING)
									{
										//Check that the polymer has an attachment point in this position, if so merge
										bool merge = false;

										std::map<sf::Vector2i, int, Vector2iCompare> attachmentPoints = (*org1It)->GetAttachmentPoints();

										for (auto pointIt = attachmentPoints.begin(); pointIt != attachmentPoints.end(); pointIt ++)
										{
											sf::Vector2i attachmentPoint = mod1It->first;

											if (i == TOP)
												attachmentPoint += sf::Vector2i(0, -1);

											else if (i == RIGHT)
												attachmentPoint += sf::Vector2i(1, 0);

											else if (i == BOTTOM)
												attachmentPoint += sf::Vector2i(0, 1);

											else if (i == LEFT)
												attachmentPoint += sf::Vector2i(-1, 0);

											if (attachmentPoint == pointIt->first)
											{
												//Module can be accreted if it is the correct type, or an inccorect module type (not boundary, or to a boundary) can be accreted with a small mutation chance
												if ((pointIt->second == ANY && (mod2It->second->GetType() != BOUNDARY)) || (pointIt->second == mod2It->second->GetType()) || (mod2It->second->GetType() != BOUNDARY && mod1It->second->GetType() != BOUNDARY && RandFloat() < MUTATION_RATE))
												{
													merge = true;
												}
											}
										}

										if (merge)
										{
											//Check organisms aren't already merging (to something else), and if not add them to merge list to be merged
											if (!(*org1It)->GetMerging() && !(*org2It)->GetMerging())
											{
												m_merges.push_back(std::shared_ptr<Merge>(new Merge(*org1It, *org2It, mod1It->second, mod2It->second, i, j)));

												(*org1It)->SetMerging(true);
												(*org2It)->SetMerging(true);
											}
										}
									}

									//
									//	PARASITES
									//

									else if ((*org1It)->GetStatus() != DEAD && (*org2It)->GetStatus() == REPRODUCING  && (*org1It)->GetModuleTypeNumber(PREDATOR) > 0 && (*org2It)->GetModuleTypeNumber(SOLAR) > 0 && !(*org1It)->GetIsParasite())
									{
										//int solarArmourDelta = (*org2It)->GetModuleTypeNumber(SOLAR) - (*org2It)->GetModuleTypeNumber(SOLAR)
										
										//More solar modules than anchor modules means parasite can draw energy from the prey
										if ((*org2It)->GetModuleTypeNumber(SOLAR) > (*org2It)->GetModuleTypeNumber(ARMOUR))
										{
											sf::Vector2f point1Delta;

											if (i == TOP)
												point1Delta = sf::Vector2f(0, -0.5);

											else if (i == BOTTOM)
												point1Delta = sf::Vector2f(0, 0.5);

											else if (i == LEFT)
												point1Delta = sf::Vector2f(-0.5, 0);

											else if (i == RIGHT)
												point1Delta = sf::Vector2f(0.5, 0);

											sf::Vector2f point2Delta;

											if (j == TOP)
												point2Delta = sf::Vector2f(0, -0.5);

											else if (j == BOTTOM)
												point2Delta = sf::Vector2f(0, 0.5);

											else if (j == LEFT)
												point2Delta = sf::Vector2f(-0.5, 0);

											else if (j == RIGHT)
												point2Delta = sf::Vector2f(0.5, 0);





											(*org2It)->SetIsHost(true);
											(*org1It)->SetIsParasite(true);

											(*org1It)->SetupParasiteJoint((*org2It)->GetBody(), mod1It->second->GetComRelPosition() + point1Delta, mod2It->second->GetComRelPosition() + point2Delta);
											//(*org1It)->SetupParasiteJoint((*org2It)->GetBody(), mod1It->second->GetComRelPosition(), mod2It->second->GetComRelPosition());

											(*org1It)->SetHost(*org2It);

											(*org2It)->AddParasite(*org1It);

											std::cout << "PARASITE" << std::endl;

											std::cout << "Body1: " << (*org1It)->GetBody()->GetPosition().x << " " << (*org1It)->GetBody()->GetPosition().y
												<< " conn point: " << mod1It->second->GetComRelPosition().x + point1Delta.x << " "  << mod1It->second->GetComRelPosition().y + point1Delta.y


												<< std::endl;


											std::cout << "Body2: " << (*org2It)->GetBody()->GetPosition().x << " " << (*org2It)->GetBody()->GetPosition().y
												<< " conn point: " << mod2It->second->GetComRelPosition().x + point2Delta.x << " "  << mod2It->second->GetComRelPosition().y + point2Delta.y
												<< std::endl;
										}
									}

									else if ((*org2It)->GetStatus() != DEAD && (*org1It)->GetStatus() == REPRODUCING  && (*org2It)->GetModuleTypeNumber(PREDATOR) > 0 && (*org1It)->GetModuleTypeNumber(SOLAR) > 0 && !(*org2It)->GetIsParasite())
									{
										//int solarArmourDelta = (*org2It)->GetModuleTypeNumber(SOLAR) - (*org2It)->GetModuleTypeNumber(SOLAR)
										
										//More solar modules than anchor modules means parasite can draw energy from the prey
										if ((*org1It)->GetModuleTypeNumber(SOLAR) > (*org1It)->GetModuleTypeNumber(ARMOUR))
										{

											sf::Vector2f point1Delta;

											if (i == TOP)
												point1Delta = sf::Vector2f(0, -0.5);

											else if (i == BOTTOM)
												point1Delta = sf::Vector2f(0, 0.5);

											else if (i == LEFT)
												point1Delta = sf::Vector2f(-0.5, 0);

											else if (i == RIGHT)
												point1Delta = sf::Vector2f(0.5, 0);

											sf::Vector2f point2Delta;

											if (j == TOP)
												point2Delta = sf::Vector2f(0, -0.5);

											else if (j == BOTTOM)
												point2Delta = sf::Vector2f(0, 0.5);

											else if (j == LEFT)
												point2Delta = sf::Vector2f(-0.5, 0);

											else if (j == RIGHT)
												point2Delta = sf::Vector2f(0.5, 0);

											(*org1It)->SetIsHost(true);
											(*org2It)->SetIsParasite(true);

											(*org2It)->SetupParasiteJoint((*org1It)->GetBody(), mod2It->second->GetComRelPosition() + point2Delta, mod1It->second->GetComRelPosition() + point1Delta);
											//(*org2It)->SetupParasiteJoint((*org1It)->GetBody(), mod2It->second->GetComRelPosition(), mod1It->second->GetComRelPosition());

											(*org2It)->SetHost(*org1It);

											(*org1It)->AddParasite(*org2It);

											std::cout << "PARASITE" << std::endl;

											std::cout << "Body1: " << (*org1It)->GetBody()->GetPosition().x << " " << (*org1It)->GetBody()->GetPosition().y
												<< " conn point: " << mod1It->second->GetComRelPosition().x + point1Delta.x << " "  << mod1It->second->GetComRelPosition().y + point1Delta.y
												<< std::endl;


											std::cout << "Body2: " << (*org2It)->GetBody()->GetPosition().x << " " << (*org2It)->GetBody()->GetPosition().y
												<< " conn point: " << mod2It->second->GetComRelPosition().x + point2Delta.x << " "  << mod2It->second->GetComRelPosition().y + point2Delta.y
												<< std::endl;
										}
									}








									//
									//	REPRODUCING ORGANISM SEEDS INDIVIDUAL MODULE WITH GROWTH PLAN
									//

									//If one the organisms is reproducing and the other is dead, try to download growth plan
									//To reproduce, the seed module needs to be of the same type as that at (0, 0) of the growth plan

//									if ((*org1It)->GetStatus() == REPRODUCING && (*org2It)->GetStatus() == DEAD && !(*org2It)->GetDeathCooldown())
//									{
//										//Check that the organism plan contains at least one module of the the same type as the seed module
//
//										std::map<sf::Vector2i, int, Vector2iCompare> plan = (*org1It)->GetPlan();
//
//										bool canSeed = false;
//
//										for (auto planMod = plan.begin(); planMod != plan.end(); planMod ++)
//										{
//											if (planMod->second == (*org2It)->GetModules()[std::make_pair(0, 0)]->GetType())
//											{
//												canSeed = true;
//												break;
//											}
//										}
//
//										if ((*org1It)->GetEnergy() >= 0.8 * MODULE_MAX_ENERGY && canSeed)
//										{
//										//	std::cout << "Seeding" << std::endl;
//
//											//Transfer energy and plan between organisms
//											(*org2It)->AddEnergy(0.8 * MODULE_MAX_ENERGY);
//
//											(*org1It)->RemoveEnergy(0.8 * MODULE_MAX_ENERGY);	//Energy transfer could have some efficiency factor
//
//											//Set organism 2 state as growing (unless it's a single module plan in which case the new organism is already complete and can start reproducing)
//											if ((*org1It)->GetPlan().size() > 1)
//												(*org2It)->SetStatus(GROWING);
//
//											else
//												(*org2It)->SetStatus(REPRODUCING);
//
//											//Transfer plan (might need to modify this in some way) (or change the module position in the new growing organism)
//											(*org2It)->SetPlan((*org1It)->GetPlan());		//Could also mutate plan at this point
////
//											//Create notification
//											m_notifications.push_back(std::shared_ptr<Notification>(new Notification(sf::Vector2f((*org2It)->GetBody()->GetPosition().x * SCALING_FACTOR, (*org2It)->GetBody()->GetPosition().y * SCALING_FACTOR), "Seeded", m_font)));
//										
//											std::cout << "Finished seeding" << std::endl;
//										}
//									}

									//else if ((*org2It)->GetStatus() == REPRODUCING && (*org1It)->GetStatus() == DEAD && !(*org1It)->GetDeathCooldown())
									//{
									//	std::map<sf::Vector2i, int, Vector2iCompare> plan = (*org2It)->GetPlan();

									//	bool canSeed = false;

									//	for (auto planMod = plan.begin(); planMod != plan.end(); planMod ++)
									//	{
									//		if (planMod->second == (*org1It)->GetModules()[std::make_pair(0, 0)]->GetType())
									//		{
									//			canSeed = true;
									//			break;
									//		}
									//	}

									//	if ((*org2It)->GetEnergy() >= 0.8 * MODULE_MAX_ENERGY && canSeed)
									//	{
									//		//std::cout << "Seeding" << std::endl;

									//		//Transfer energy and plan between organisms
									//		(*org1It)->AddEnergy(0.8 * MODULE_MAX_ENERGY);

									//		(*org2It)->RemoveEnergy(0.8 * MODULE_MAX_ENERGY);	//Energy transfer could have some efficiency factor

									//		//Set organism 1 state as growing (unless it's a single module plan in which case the new organism is already complete and can start reproducing)
									//		if ((*org2It)->GetPlan().size() > 1)
									//			(*org1It)->SetStatus(GROWING);

									//		else
									//			(*org1It)->SetStatus(REPRODUCING);

									//		//Transfer plan (might need to modify this in some way) (or change the module position in the new growing organism)
									//		(*org1It)->SetPlan((*org2It)->GetPlan());		//Could also mutate plan at this point

									//		//Create notification
									//		m_notifications.push_back(std::shared_ptr<Notification>(new Notification(sf::Vector2f((*org1It)->GetBody()->GetPosition().x * SCALING_FACTOR, (*org1It)->GetBody()->GetPosition().y * SCALING_FACTOR), "Seeded", m_font)));
									//	
									//		std::cout << "Finished seeding" << std::endl;
									//	}
									//}

									//
									//	GROWING ORGANISM ACCRETES INDIVIDUAL MODULE
									//

									//If one organism is growing and one is dead, merge. 
									
									//if ((*org1It)->GetStatus() == DEAD && (*org2It)->GetStatus() == GROWING)
									//{
									//	//Check that the module to be accreted is of the proper type to fit in the plan at that position

									//	/*if ((*org2It)->GetPlanNeighbouringModuleType(mod2It->first, j) == mod1It->second->GetType())*/
									//	if ((*org2It)->GetModuleInPlan(sf::Vector2i(mod2It->first.first, mod2It->first.second), j, mod1It->second->GetType()))
									//	{
									//		//Add organisms to merge list and set as merging (if not already merging)

									//		if (!(*org1It)->GetMerging() && !(*org2It)->GetMerging())
									//		{
									//			m_merges.push_back(std::shared_ptr<Merge>(new Merge(*org1It, *org2It, mod1It->second, mod2It->second, i, j)));

									//			//std::cout << std::endl << std::endl << "MERGE" << std::endl;

									//			//std::cout << "Mod1: " << mod1It->second->GetPosition().x << " " << mod1It->second->GetPosition().y << std::endl;
									//			//std::cout << "Mod2: " << mod2It->second->GetPosition().x << " " << mod2It->second->GetPosition().y << std::endl;

									//			(*org1It)->SetMerging(true);
									//			(*org2It)->SetMerging(true);
									//		}
									//	}
									//}

									//else if ((*org2It)->GetStatus() == DEAD && (*org1It)->GetStatus() == GROWING)
									//{
									//	//Check that the module to be accreted is of the proper type to fit in the plan at that position

									//	//if ((*org1It)->GetPlanNeighbouringModuleType(mod1It->first, i) == mod2It->second->GetType())
									//	if ((*org1It)->GetModuleInPlan(sf::Vector2i(mod1It->first.first, mod1It->first.second), i, mod2It->second->GetType()))
									//	{
									//		//Add organisms to merge list and set as merging (if not already merging)

									//		if (!(*org1It)->GetMerging() && !(*org2It)->GetMerging())
									//		{
									//			m_merges.push_back(std::shared_ptr<Merge>(new Merge(*org1It, *org2It, mod1It->second, mod2It->second, i, j)));

									//			//std::cout << std::endl << std::endl << "MERGE" << std::endl;

									//			//std::cout << "Mod1: " << mod1It->second->GetPosition().x << " " << mod1It->second->GetPosition().y << std::endl;
									//			//std::cout << "Mod2: " << mod2It->second->GetPosition().x << " " << mod2It->second->GetPosition().y << std::endl;

									//			(*org1It)->SetMerging(true);
									//			(*org2It)->SetMerging(true);
									//		}
									//	}
									//}


									//
									//	PREDATAION
									//


									////If organism have predator modules allow them to prey upon one another
									//else if (((*org1It)->GetStatus() == GROWING || (*org1It)->GetStatus() == REPRODUCING) && (*org2It)->GetStatus() == REPRODUCING && mod1It->second->GetType() == PREDATOR && mod2It->second->GetType() != ARMOUR)
									//{
									//	float preyEnergy = (*org2It)->GetEnergy();
									//	float predEnergyNeed = (*org1It)->GetMaxEnergy() - (*org1It)->GetEnergy();

									//	(*org1It)->AddEnergy(std::max(preyEnergy, predEnergyNeed));
									//	(*org2It)->RemoveEnergy((*org2It)->GetEnergy());

									//	(*org2It)->SetStatus(DEAD);
									//	(*org2It)->SetToKill();

									//	std::cout << "Finished eating" << std::endl;

									//	m_notifications.push_back(std::shared_ptr<Notification>(new Notification(sf::Vector2f((*org1It)->GetBody()->GetPosition().x * SCALING_FACTOR, (*org1It)->GetBody()->GetPosition().y * SCALING_FACTOR), "Eating", m_font)));
									//}

									//else if (((*org2It)->GetStatus() == GROWING || (*org2It)->GetStatus() == REPRODUCING) && (*org1It)->GetStatus() == REPRODUCING && mod2It->second->GetType() == PREDATOR && mod1It->second->GetType() != ARMOUR)
									//{
									//	float preyEnergy = (*org1It)->GetEnergy();
									//	float predEnergyNeed = (*org2It)->GetMaxEnergy() - (*org2It)->GetEnergy();

									//	(*org2It)->AddEnergy(std::max(preyEnergy, predEnergyNeed));
									//	(*org1It)->RemoveEnergy((*org1It)->GetEnergy());

									//	(*org1It)->SetStatus(DEAD);
									//	(*org1It)->SetToKill();

									//	std::cout << "Finished eating" << std::endl;

									//	m_notifications.push_back(std::shared_ptr<Notification>(new Notification(sf::Vector2f((*org2It)->GetBody()->GetPosition().x * SCALING_FACTOR, (*org2It)->GetBody()->GetPosition().y * SCALING_FACTOR), "Eating", m_font)));
									//}
								}
							}
						}
					}
				}
			}
		}
	}
}


void World::Draw()
{
	m_window.clear(sf::Color::White);

	//Draw world
	DrawWalls(m_window);

	//Draw modules
	//for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
		//(*modIt)->Draw(window);

	for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); orgIt++ )
		(*orgIt)->Draw(m_window);


	DrawLightSources(m_window);


	//Draw notifications
	for (auto notIt = m_notifications.begin(); notIt != m_notifications.end(); notIt ++)
	{
		(*notIt)->Draw(m_window);
	}

	sf::View view = m_window.getView();

	m_window.setView(m_window.getDefaultView());

	//Draw selected organism info
	if (!m_selectedOrganism.expired())
	{
		std::stringstream ss;

		if (m_selectedOrganism.lock()->GetStatus() == DEAD)
			ss << "State: DEAD"; 

		else if (m_selectedOrganism.lock()->GetStatus() == GROWING)
			ss << "State: GROWING"; 

		else if (m_selectedOrganism.lock()->GetStatus() == REPRODUCING)
			ss << "State: REPRODUCING"; 


		ss << "\nEnergy: " << int(m_selectedOrganism.lock()->GetEnergy()) << "/" << int(m_selectedOrganism.lock()->GetMaxEnergy());

		ss << "\nPos: " << m_selectedOrganism.lock()->GetBody()->GetPosition().x << " " << m_selectedOrganism.lock()->GetBody()->GetPosition().y;

		ss << "\nAngle: " << m_selectedOrganism.lock()->GetBody()->GetAngle();

		m_selectedOrgInfo.setString(ss.str());

		if (m_selectedOrganism.lock()->GetIsParasite())
			m_selectedOrgInfo.setColor(sf::Color::Red);

		else if (m_selectedOrganism.lock()->GetIsHost())
			m_selectedOrgInfo.setColor(sf::Color::Green);

		else
			m_selectedOrgInfo.setColor(sf::Color::Black);

		m_window.draw(m_selectedOrgInfo);





		std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> modules = m_selectedOrganism.lock()->GetModules();

		for (auto module = modules.begin(); module != modules.end(); module ++)
		{
			sf::RectangleShape box(sf::Vector2f(MODULE_SIZE * SCALING_FACTOR, MODULE_SIZE * SCALING_FACTOR));

			box.setOrigin(MODULE_SIZE * SCALING_FACTOR / 2, MODULE_SIZE * SCALING_FACTOR / 2);

			box.setOutlineThickness(1);

			box.setOutlineColor(sf::Color::Black);

			box.setPosition(module->first.x * MODULE_SIZE * SCALING_FACTOR + 100, module->first.y * MODULE_SIZE * SCALING_FACTOR + 200);

			if (module->second->GetType() == SOLAR)
			box.setFillColor(sf::Color(0, 0, 100, 150));

			else if (module->second->GetType() == ANCHOR)
				box.setFillColor(sf::Color(0, 100, 0, 150));

			else if (module->second->GetType() == BOUNDARY)
				box.setFillColor(sf::Color(255, 165, 0, 150));

			else if ((module)->second->GetType() == PREDATOR)
			box.setFillColor(sf::Color(255, 0, 255, 150));

			else if ((module)->second->GetType() == ARMOUR)
				box.setFillColor(sf::Color(255, 255, 0, 150));

			m_window.draw(box);
		}

		std::map<sf::Vector2i, int, Vector2iCompare> attachmentPoints = m_selectedOrganism.lock()->GetAttachmentPoints();

		for (auto attachmentPoint = attachmentPoints.begin(); attachmentPoint != attachmentPoints.end(); attachmentPoint ++)
		{
			sf::RectangleShape box(sf::Vector2f(MODULE_SIZE * SCALING_FACTOR, MODULE_SIZE * SCALING_FACTOR));

			box.setOrigin(MODULE_SIZE * SCALING_FACTOR / 2, MODULE_SIZE * SCALING_FACTOR / 2);

			box.setOutlineThickness(1);

			box.setOutlineColor(sf::Color::Black);

			box.setPosition(attachmentPoint->first.x * MODULE_SIZE * SCALING_FACTOR + 100, attachmentPoint->first.y * MODULE_SIZE * SCALING_FACTOR + 200);

			box.setFillColor(sf::Color::White);

			m_window.draw(box);
		}


		//Draw misc
		{
			std::stringstream ss;

			ss << "Current: " << m_selectedOrganism.lock()->GetActiveAnchors() << " Previous: " << m_selectedOrganism.lock()->GetPreviousAnchors();

			m_miscText.setString(ss.str());

			m_window.draw(m_miscText);
		}
	}


	//Draw FPS text
	{
		std::stringstream ss;

		ss << "FPS: " << m_FPS;

		m_FPSText.setString(ss.str());

		m_window.draw(m_FPSText);
	}

	//Draw simulation time
	{
		std::stringstream ss;

		ss << "Simulation time: " << int(m_mainCounter);

		m_simTimeText.setString(ss.str());

		m_window.draw(m_simTimeText);
	}

	m_window.setView(view);

	m_window.display();

	//Lock to FPS
	while (m_mainClock.getElapsedTime().asMilliseconds() < 1 / (float(m_FPS) / 1000))
	{
	}

	m_mainClock.restart();
}

void World::DrawLightSources(sf::RenderWindow& window)
{
	if (BACKGROUND_LIGHT_INTENSITY > 0)
	{
		/*sf::RectangleShape rect(sf::Vector2f(WORLD_SIZE * SCALING_FACTOR, WORLD_SIZE * SCALING_FACTOR));

		rect.setOrigin(WORLD_SIZE * SCALING_FACTOR / 2, WORLD_SIZE * SCALING_FACTOR / 2);

		rect.setPosition(0, 0);

		rect.setFillColor(sf::Color(255, 250, 205, 100 * BACKGROUND_LIGHT_INTENSITY));

		window.draw(rect);*/
	}

	else if (m_lightingType == QUADRANT)
	{
		sf::RectangleShape rect(sf::Vector2f(WORLD_SIZE * SCALING_FACTOR * 0.5, WORLD_SIZE * SCALING_FACTOR * 0.5));

		rect.setPosition(0, 0);

		rect.setFillColor(sf::Color(255, 250, 205, 100));

		window.draw(rect);
	}

	else if (m_lightingType == ROTATING)
	{
		//sf::ConvexShape shape;

		//shape.setPointCount(5);

		//sf::Vector2f trianglePoints(WORLD_SIZE * SCALING_FACTOR / 2, -50);
		//sf::Vector2f trianglePoints(WORLD_SIZE * SCALING_FACTOR / 2,  50);

		//// define the points
		//shape.setPoint(0, sf::Vector2f(0, 0));
		//shape.setPoint(1, sf::Vector2f(150, 10));
		//shape.setPoint(2, sf::Vector2f(120, 90));
	}

	else if (NUM_LIGHT_SOURCES > 0)
	{
		for (auto lightIt = m_lightSources.begin(); lightIt != m_lightSources.end(); lightIt ++)
		{
			sf::CircleShape circle((*lightIt)->m_radius * SCALING_FACTOR);

			circle.setPosition((*lightIt)->m_position.x * SCALING_FACTOR, (*lightIt)->m_position.y * SCALING_FACTOR);
			circle.setFillColor(sf::Color(255, 250, 205, 200));
			circle.setOrigin((*lightIt)->m_radius * SCALING_FACTOR, (*lightIt)->m_radius * SCALING_FACTOR);

			window.draw(circle);
		}
	}
}

void World::HandleEvents()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
	{
		sf::View view = m_window.getView();

		view.move(sf::Vector2f(-10, 0));

		m_window.setView(view);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
	{
		sf::View view = m_window.getView();

		view.move(sf::Vector2f(10, 0));

		m_window.setView(view);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
	{
		sf::View view = m_window.getView();

		view.move(sf::Vector2f(0, -10));

		m_window.setView(view);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
	{
		sf::View view = m_window.getView();

		view.move(sf::Vector2f(0, 10));

		m_window.setView(view);
	}

	sf::Event event;
	while(m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_window.close();
		}

		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::F)
			{
				if (m_FPS < 1000)
					m_FPS += 5;
			}

			else if (event.key.code == sf::Keyboard::S)
			{
				if (m_FPS > 10)
					m_FPS -= 5;
			}

			else if (event.key.code == sf::Keyboard::P)
			{
				m_paused = !m_paused;
			}

			else if (event.key.code == sf::Keyboard::Subtract)
			{
				sf::View view = m_window.getView();

				view.zoom(0.5);

				m_window.setView(view);
			}

			else if (event.key.code == sf::Keyboard::Add)
			{
				sf::View view = m_window.getView();

				view.zoom(2);

				m_window.setView(view);
			}
		}

		else if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				if (!m_selectedOrganism.expired())
					m_selectedOrganism.lock()->SetSelected(false);

				std::weak_ptr<Organism> org;

				m_selectedOrganism = org;

				//Check if there's an organism at that position
				for (auto orgIt = m_organisms.begin(); orgIt != m_organisms.end(); orgIt ++)
				{
					sf::Vector2i scaledPosition = sf::Vector2i((*orgIt)->GetBody()->GetPosition().x * SCALING_FACTOR, (*orgIt)->GetBody()->GetPosition().y * SCALING_FACTOR);

					sf::Vector2f mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));



					//std::cout << "MOUSE: " << mousePos.x << " " << mousePos.y << std::endl;

					if (pow(float(mousePos.x - scaledPosition.x), 2) + pow(float(mousePos.y - scaledPosition.y), 2) <= pow(float(MODULE_SIZE / 2 * SCALING_FACTOR * (*orgIt)->GetModules().size()), 2))
					{
						m_selectedOrganism = *orgIt;

						(*orgIt)->SetSelected(true);

						break;
					}
				}
			}
		}
	}
}

void World::DrawWalls(sf::RenderWindow& window)
{
	sf::RectangleShape rect(sf::Vector2f(WORLD_SIZE * SCALING_FACTOR, WORLD_SIZE * SCALING_FACTOR));

	rect.setOrigin(WORLD_SIZE * SCALING_FACTOR / 2, WORLD_SIZE * SCALING_FACTOR / 2);

	rect.setPosition(0, 0);

	rect.setFillColor(sf::Color(130, 130, 130));

	window.draw(rect);
}

void World::AddModule(sf::Vector2f pos)
{
	//m_modules.push_back(std::shared_ptr<Module>(new Module(*m_world, pos)));
}

void World::AddSingleModuleOrganism(b2Vec2 pos, float rot, b2Vec2 vel, float angVel, int type)
{
	//Just add an organism with a single module (to be used when setting up initial state) (it's a dead organism)
	std::shared_ptr<Module> module = std::shared_ptr<Module>(new Module(*m_world, sf::Vector2i(0, 0), type));

	m_modules.push_back(module);

	std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> moduleMap;

	moduleMap[sf::Vector2i(0, 0)] = module;

	//Empty plan for dead organism
	std::map<sf::Vector2i, int, Vector2iCompare> emptyPlan;

	if (type == SOLAR || type == PREDATOR)
		m_organisms.push_back(std::shared_ptr<Organism>(new Organism(*m_world, m_groundBody, pos, rot, vel, angVel, moduleMap, 150, emptyPlan, GROWING, m_startTime)));

	else
		m_organisms.push_back(std::shared_ptr<Organism>(new Organism(*m_world, m_groundBody, pos, rot, vel, angVel, moduleMap, 0, emptyPlan, GROWING, m_startTime)));

	module->SetOrganism(m_organisms.back());
}

void World::AddCustomOrganism(std::map<std::pair<int, int>, std::shared_ptr<Module>> moduleMap, b2Vec2 position, float angle = 0)
{
	////Build plan is just the current topology
	//std::map<sf::Vector2i, int, Vector2iCompare> plan;

	//for (auto modIt = moduleMap.begin(); modIt != moduleMap.end(); modIt ++)
	//	plan[sf::Vector2i(modIt->first.first, modIt->first.second)] = modIt->second->GetType();

	//int status;

	//if (plan.size() == 1)
	//	status = DEAD;

	//else
	//	status = REPRODUCING;

	//m_organisms.push_back(std::shared_ptr<Organism>(new Organism(*m_world, m_groundBody, position, angle, b2Vec2(0, 0), 0, moduleMap, MODULE_MAX_ENERGY * moduleMap.size(), plan, status, m_startTime)));

	//for (auto modIt = moduleMap.begin(); modIt != moduleMap.end(); modIt ++)
	//{
	//	m_modules.push_back(modIt->second);
	//	modIt->second->SetOrganism(m_organisms.back());
	//}
}

void World::WriteState()
{
	//Read out various stats to text file

	m_writeCounter ++;

	if (m_writeCounter >= WRITE_PERIOD)
	{
		//
		//	EVENT
		//

		m_eventOutputFile.open(m_filename2.str(), std::ios_base::app);

		m_eventOutputFile << eventWriteString.str();

		m_eventOutputFile.close();

		eventWriteString.clear();

		eventWriteString.str("");		//Clear string


		//
		//	STATE
		//

		m_stateOutputFile.open(m_filename.str(), std::ios_base::app);

		m_writeCounter = 0;

		int numCompleteOrganisms = 0;
		int numGrowingOrganisms = 0;
		float averageParentSize = 0;
		float averageTotalSize = 0;

		float averageTotalSolarModules = 0;
		float averageTotalAnchorModules = 0;
		float averageTotalArmourModules = 0;
		float averageTotalPredatorModules = 0;
		float averageParentSolarModules = 0;
		float averageParentAnchorModules = 0;
		float averageParentArmourModules = 0;
		float averageParentPredatorModules = 0;

		long long minGeneration = m_organisms.back()->GetGeneration();
		long long maxGeneration = 0;
		float averageGeneration = 0;

		int currentNumBornOrganisms = 0;
		int currentNumCreatedOrganisms = 0;

		int numFreeBoundaryModules = 0;
		int numFreeSolarModules = 0;
		int numFreeAnchorModules = 0;
		int numFreeArmourModules = 0;
		int numFreePredatorModules = 0;


		float averageOrganismLifetime = 0;

		for (auto organism = m_organisms.begin(); organism != m_organisms.end(); organism++)
		{
			if ((*organism)->GetStatus() == DEAD || (*organism)->GetModules().size() == 1)
			{
				if ((*organism)->GetModuleTypeNumber(BOUNDARY) == 1)
					numFreeBoundaryModules ++;

				else if ((*organism)->GetModuleTypeNumber(SOLAR) == 1)
					numFreeSolarModules ++;

				else if ((*organism)->GetModuleTypeNumber(ANCHOR) == 1)
					numFreeAnchorModules ++;

				else if ((*organism)->GetModuleTypeNumber(ARMOUR) == 1)
					numFreeArmourModules ++;

				else if ((*organism)->GetModuleTypeNumber(PREDATOR) == 1)
					numFreePredatorModules ++;
			}


			else if ((*organism)->GetStatus() == GROWING)
				numGrowingOrganisms ++;

			else if ((*organism)->GetStatus() == REPRODUCING)
			{
				maxGeneration = std::max(maxGeneration, (*organism)->GetGeneration());

				minGeneration = std::min(minGeneration, (*organism)->GetGeneration());

				averageGeneration += (*organism)->GetGeneration();

				numCompleteOrganisms ++;
				averageTotalSize += (*organism)->GetModules().size();
				averageParentSize += (*organism)->GetParentModules().size();

				averageOrganismLifetime += (*organism)->GetLifetime();

				std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> modules = (*organism)->GetModules();

				for (auto module = modules.begin(); module != modules.end(); module ++)
				{
					if (module->second->GetType() == SOLAR)
						averageTotalSolarModules ++;

					else if (module->second->GetType() == ANCHOR)
						averageTotalAnchorModules ++;

					else if (module->second->GetType() == ARMOUR)
						averageTotalArmourModules ++;

					else if (module->second->GetType() == PREDATOR)
						averageTotalPredatorModules ++;
				}

				std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> parentModules = (*organism)->GetParentModules();

				for (auto module = parentModules.begin(); module != parentModules.end(); module ++)
				{
					if (module->second.lock()->GetType() == SOLAR)
						averageParentSolarModules ++;

					else if (module->second.lock()->GetType() == ANCHOR)
						averageParentAnchorModules ++;

					else if (module->second.lock()->GetType() == ARMOUR)
						averageParentArmourModules ++;

					else if (module->second.lock()->GetType() == PREDATOR)
						averageParentPredatorModules ++;
				}

				if ((*organism)->GetBirthType() == BORN)
					currentNumBornOrganisms ++;

				else if ((*organism)->GetBirthType() == CREATED)
					currentNumCreatedOrganisms ++;
			}
		}

		if (numCompleteOrganisms)
		{
			averageTotalSize /= float(numCompleteOrganisms);
			averageParentSize /= float(numCompleteOrganisms);
			averageTotalSolarModules /= float(numCompleteOrganisms);
			averageTotalAnchorModules /= float(numCompleteOrganisms);
			averageTotalArmourModules /= float(numCompleteOrganisms);
			averageTotalPredatorModules /= float(numCompleteOrganisms);
			averageParentSolarModules /= float(numCompleteOrganisms);
			averageParentAnchorModules /= float(numCompleteOrganisms);
			averageParentArmourModules /= float(numCompleteOrganisms);
			averageParentPredatorModules /= float(numCompleteOrganisms);

			averageOrganismLifetime /= float(numCompleteOrganisms);
			averageOrganismLifetime *= TIME_STEP;

			averageGeneration /= float(numCompleteOrganisms);
		}

		else
		{
			averageTotalSize = 0;
			averageTotalSolarModules = 0;
			averageTotalAnchorModules = 0;
			averageParentSize = 0;
			averageParentSolarModules = 0;
			averageParentAnchorModules = 0;
		}
		
		std::stringstream ss;

		ss << averageOrganismLifetime << ','
		<< m_createdOrganisms << ','
		<< m_reproducedOrganisms << ','
		<< currentNumCreatedOrganisms << ','
		<< currentNumBornOrganisms << ','
		<< numCompleteOrganisms << ',' 
		<< numGrowingOrganisms << ','
		<< averageParentSize << ','
		<< averageParentSolarModules << ','
		<< averageParentAnchorModules << ','
		<< averageParentArmourModules << ','
		<< averageParentPredatorModules << ','
		<< averageTotalSize << ','
		<< averageTotalSolarModules << ','
		<< averageTotalAnchorModules << ','
		<< averageTotalArmourModules << ','
		<< averageTotalPredatorModules << ','
		<< maxGeneration << ','
		<< minGeneration << ','
		<< averageGeneration << ','
		<< numFreeBoundaryModules << ','
		<< numFreeSolarModules << ','
		<< numFreeAnchorModules << ','
		<< numFreeArmourModules << ','
		<< numFreePredatorModules << ',';

		ss << '\n';

		m_stateOutputFile << ss.str();

		m_stateOutputFile.close();


		//
		//	SPECIES
		//

		//Now sort organisms into species. Based only on number of each module (not position of moudle within organism)
		//Then read out # of E/I/B modules and # of organisms of that species, for each species

		std::map<std::tuple<int, int, int, int, int>, int> tempSpecies;

		for (auto org = m_organisms.begin(); org != m_organisms.end(); org ++)
		{
			if ((*org)->GetStatus() == REPRODUCING)
			{
				std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> modules = (*org)->GetParentModules();

				int boundary = 0;
				int solar = 0;
				int anchor = 0;
				int armour = 0;
				int predator = 0;

				for (auto mod = modules.begin(); mod != modules.end(); mod++)
				{

					if (mod->second.lock()->GetType() == BOUNDARY)
						boundary ++;

					else if (mod->second.lock()->GetType() == SOLAR)
						solar ++;

					else if (mod->second.lock()->GetType() == ANCHOR)
						anchor ++;

					else if (mod->second.lock()->GetType() == ARMOUR)
						armour ++;

					else if (mod->second.lock()->GetType() == PREDATOR)
						predator ++;
				}

				//If this species is already in the map

				if (tempSpecies.count(std::make_tuple(boundary, solar, anchor, armour, predator)))
					tempSpecies[std::make_tuple(boundary, solar, anchor, armour, predator)] ++;

				else
					tempSpecies[std::make_tuple(boundary, solar, anchor, armour, predator)] = 1;
			}
		}

		//Reset all saved numbers to 0

		for (auto species = m_species.begin(); species != m_species.end(); species ++)
			species->second = 0;

		
		//Now save these species and write out, checking if they already exist or not (fill with 0s if they didn't)

		//Write out current species here
		for (auto species = tempSpecies.begin(); species != tempSpecies.end(); species ++)
		{
			//If species is already registered, append to file
			if (m_species.count(species->first))
			{

				m_species[std::make_tuple(std::get<0>(species->first), std::get<1>(species->first), std::get<2>(species->first), std::get<3>(species->first), std::get<4>(species->first))] = species->second;

				std::stringstream ss;

				ss << m_outputName << "/SPECIES_" << std::get<0>(species->first) << "_" << std::get<1>(species->first) << "_" << std::get<2>(species->first) << "_" << std::get<3>(species->first) << "_" << std::get<4>(species->first) << ".csv";

				std::ofstream speciesOutput(ss.str(), std::ofstream::app);

				speciesOutput << species->second << ',' << m_mainCounter << '\n';

				speciesOutput.close();
			}

			//If not, create a new file and set all previous time entries to 0

			else
			{
				m_species[std::make_tuple(std::get<0>(species->first), std::get<1>(species->first), std::get<2>(species->first), std::get<3>(species->first), std::get<4>(species->first))] = species->second;

				std::stringstream ss;

				ss << m_outputName << "/SPECIES_" << std::get<0>(species->first) << "_" << std::get<1>(species->first) << "_" << std::get<2>(species->first) << "_" << std::get<3>(species->first) << "_" << std::get<4>(species->first) <<".csv";

				std::ofstream speciesOutput(ss.str());

				for (int i = 0; i < m_mainCounter; i+= WRITE_PERIOD)
				{
					speciesOutput << 0 << ',' << i << '\n';
				}

				speciesOutput << species->second << ',' << m_mainCounter << '\n';

				speciesOutput.close();
			}

		}

		//Write out remaining (extinct) species here
		for (auto species = m_species.begin(); species != m_species.end(); species ++)
		{
			if (!tempSpecies.count(species->first))
			{
				std::stringstream ss;

				ss << m_outputName << "/SPECIES_" << std::get<0>(species->first) << "_" << std::get<1>(species->first) << "_" << std::get<2>(species->first) << "_" << std::get<3>(species->first) << "_" << std::get<4>(species->first) << ".csv";

				std::ofstream speciesOutput(ss.str(), std::ofstream::app);

				speciesOutput << species->second << ',' << m_mainCounter << '\n';

				speciesOutput.close();
			}
		}






		
		//Now go through every complete organism, add number of modules, type and position of each module

		/*for (auto organism = m_organisms.begin(); organism != m_organisms.end(); organism++)
		{
			if ((*organism)->GetStatus() == REPRODUCING)
			{
				std::map<std::pair<int, int>, std::shared_ptr<Module>> modules = (*organism)->GetModules();

				ss << modules.size() << ',';

				for (auto module = modules.begin(); module != modules.end(); module ++)
				{
					ss << module->second->GetType() << ',' << module->second->GetPosition().x << ',' << module->second->GetPosition().y << ',';
				}
			}
		}*/
	}
}