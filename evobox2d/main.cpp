/*
* Copyright (c) 2006-2007 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/




//NOTE: This code refers to Organism and Module classes. A Module is a single indivisible unit. An Organism is a structure composed of a non-zero number
//		of Modules (i.e. this is not completely congruent with the evo-bot concept. In this code even an individual Module is contained within an
//		instance of the Organism class - one with n = 1 modules).






#include "World.hpp"

//The main world - this holds everything for a given simulation run
std::shared_ptr<World> world;

int main(int argc, char *argv[])
{
	//Input to program:

	// Charging rate

	// Power draw

	// Predator rate

	// Mutation rate

	// Seed

	// Output name

	unsigned long seed = 14;

	char * outputName = "OutputDefault";


	std::cout << "Args: " << argc << std::endl;

	if (argc == 17)
	{
		CHARGING_RATE = std::atof(argv[1]);

		MODULE_POWER_DRAW = std::atof(argv[2]);

		PREDATOR_RATE = std::atof(argv[3]);

		MUTATION_RATE = std::atof(argv[4]);

		seed = std::atoi(argv[5]);

		outputName = argv[6];

		GRAPHICS_ON = std::atoi(argv[7]);

		PERPETUAL_ABIOGENESIS = std::atoi(argv[8]);

		MAX_TIME = std::atof(argv[9]);

		NUM_STARTING_ORGANISMS = std::atoi(argv[10]);

		PRED_PREY = std::atoi(argv[11]);

		BACKGROUND_LIGHT_INTENSITY = std::atof(argv[12]);

		SOURCE_LIGHT_INTENSITY = std::atof(argv[13]);

		NUM_LIGHT_SOURCES = std::atoi(argv[14]);

		MAX_LIGHT_RADIUS = std::atof(argv[15]);

		WORLD_SIZE = std::atoi(argv[16]);
	}

	world = std::shared_ptr<World>(new World(seed, outputName));			//Set up the new world

	if (GRAPHICS_ON)
		world->SetupWindow();

	while (true)											//Main loop, everything happens here. Handle use input, update simulation, draw output
	{
		if (GRAPHICS_ON)
			world->HandleEvents();

		if (!world->Update())
			break;

		if (GRAPHICS_ON)
			world->Draw();
	}
}



































//class Module2
//{
//	public:
//
//		Module2() {}
//
//		void SetUp(b2World& theWorld)
//		{
//
//			std::cout << "Setting up body" << std::endl;
//
//			// Define the dynamic body. We set its position and call the body factory.
//			b2BodyDef bodyDef;
//			bodyDef.type = b2_dynamicBody;
//			bodyDef.position.Set(1.50f, 4.0f);
//			bodyDef.allowSleep = false;
//			bodyDef.angle = PI / 5;
//			m_body = theWorld.CreateBody(&bodyDef);
//
//			// Define another box shape for our dynamic body.
//			b2PolygonShape dynamicBox;
//			dynamicBox.SetAsBox(0.1f, 0.1f);
//
//			// Define the dynamic body fixture.
//			b2FixtureDef fixtureDef;
//			fixtureDef.shape = &dynamicBox;
//
//			fixtureDef.restitution = 1;
//
//			// Set the box density to be non-zero, so it will be dynamic.
//			fixtureDef.density = 10.0f;
//
//			// Override the default friction.
//			fixtureDef.friction = 0.6f;
//
//			// Add the shape to the body.
//			m_body->CreateFixture(&fixtureDef);
//
//			std::cout << "Position: " << m_body->GetPosition().x << " " << m_body->GetPosition().y << std::endl;
//		}
//
//		b2Body* m_body;
//
//};


//std::vector<std::shared_ptr<Module2>> modules;
//
//int main(int argc, char** argv)
//{
//	B2_NOT_USED(argc);
//	B2_NOT_USED(argv);
//
//	// Define the gravity vector.
//	b2Vec2 gravity(0.0f, -10.0f);
//
//	// Construct a world object, which will hold and simulate the rigid bodies.
//	b2World world(gravity);
//
//	modules.push_back(std::shared_ptr<Module2>(new Module2()));
//
//	modules[0]->SetUp(world);
//
//	std::cout << modules[0]->m_body->GetPosition().x << " " << modules[0]->m_body->GetPosition().y << std::endl;
//
//	std::cout << "TEST" << std::endl;
//
//	// Define the ground body.
//	b2BodyDef groundBodyDef;
//	groundBodyDef.position.Set(0.0f, -1.0f);
//
//	// Call the body factory which allocates memory for the ground body
//	// from a pool and creates the ground box shape (also from a pool).
//	// The body is also added to the world.
//	b2Body* groundBody = world.CreateBody(&groundBodyDef);
//
//	// Define the ground box shape.
//	b2PolygonShape groundBox;
//
//	// The extents are the half-widths of the box.
//	groundBox.SetAsBox(5.0f, 1.0f);
//
//	// Add the ground fixture to the ground body.
//	groundBody->CreateFixture(&groundBox, 0.0f);
//
//
//	//Wall boxes 
//	b2BodyDef rightWallBodyDef;
//	rightWallBodyDef.position.Set(4.0f, 0.0f);
//
//	b2Body* rightWallBody = world.CreateBody(&rightWallBodyDef);
//
//	b2PolygonShape rightWallBox;
//
//	rightWallBox.SetAsBox(1, 5);
//
//	rightWallBody->CreateFixture(&rightWallBox, 0);
//
//	b2BodyDef leftWallBodyDef;
//	leftWallBodyDef.position.Set(-1.0f, 0.0f);
//
//	b2Body* leftWallBody = world.CreateBody(&leftWallBodyDef);
//
//	b2PolygonShape leftWallBox;
//
//	leftWallBox.SetAsBox(1, 5);
//
//	leftWallBody->CreateFixture(&leftWallBox, 0);
//
//
//
//	//// Define the dynamic body. We set its position and call the body factory.
//	//b2BodyDef bodyDef;
//	//bodyDef.type = b2_dynamicBody;
//	//bodyDef.position.Set(15.0f, 40.0f);
//	//bodyDef.allowSleep = false;
//	//bodyDef.angle = PI / 5;
//	//b2Body* body = world.CreateBody(&bodyDef);
//
//	//// Define another box shape for our dynamic body.
//	//b2PolygonShape dynamicBox;
//	//dynamicBox.SetAsBox(1.0f, 1.0f);
//
//	//// Define the dynamic body fixture.
//	//b2FixtureDef fixtureDef;
//	//fixtureDef.shape = &dynamicBox;
//
//	//fixtureDef.restitution = 1;
//
//	//// Set the box density to be non-zero, so it will be dynamic.
//	//fixtureDef.density = 10.0f;
//
//	//// Override the default friction.
//	//fixtureDef.friction = 0.6f;
//
//	//// Add the shape to the body.
//	//body->CreateFixture(&fixtureDef);
//
//	// Prepare for simulation. Typically we use a time step of 1/60 of a
//	// second (60Hz) and 10 iterations. This provides a high quality simulation
//	// in most game scenarios.
//	float32 timeStep = 1.0f / 60.0f;
//	int32 velocityIterations = 6;
//	int32 positionIterations = 2;
//
//	mainClock.restart();
//
//	while (true)
//	{
//		//Get input
//
//		sf::Event event;
//		while(window.pollEvent(event))
//		{
//			if (event.type == sf::Event::Closed)
//			{
//				window.close();
//			}
//		}
//
//		//Update
//
//		// Instruct the world to perform a single step of simulation.
//		// It is generally best to keep the time step and iterations fixed.
//		world.Step(timeStep, velocityIterations, positionIterations);
//
//		//printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
//
//		//Draw
//
//
//		window.clear(sf::Color::White);
//
//
//
//		sf::RectangleShape ground(sf::Vector2f(1000, 200));
//
//		ground.setOrigin(500, 100);
//
//		ground.setPosition(0, -100);
//
//		ground.setFillColor(sf::Color::Yellow);
//
//		window.draw(ground);
//
//
//
//		sf::RectangleShape rightWallBox(sf::Vector2f(200, 1000));
//
//		rightWallBox.setOrigin(100, 500);
//
//		rightWallBox.setPosition(-100, 0);
//
//		rightWallBox.setFillColor(sf::Color::Yellow);
//
//		window.draw(rightWallBox);
//
//
//	/*b2BodyDef leftWallBodyDef;
//	leftWallBodyDef.position.Set(10.0f, 0.0f);
//
//	b2Body* leftWallBody = world.CreateBody(&leftWallBodyDef);
//
//	b2PolygonShape leftWallBox;
//
//	leftWallBox.SetAsBox(10, 50);
//
//	leftWallBody->CreateFixture(&leftWallBox, 0);*/
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//		sf::RectangleShape box(sf::Vector2f(20, 20));
//
//		box.setOrigin(10, 10);
//
//		std::cout << modules[0]->m_body->GetPosition().x << " " << modules[0]->m_body->GetPosition().y << std::endl;
//
//		std::cout << "TEST" << std::endl;
//
//		box.setPosition(modules[0]->m_body->GetPosition().x * 100, modules[0]->m_body->GetPosition().y * 100);
//
//		box.setRotation(modules[0]->m_body->GetAngle() * float(180) / PI);
//
//		//std::cout << "Position: " << body->GetPosition().x * 10 + 100 << " " << body->GetPosition().y * 10 + 100 << std::endl;
//
//		box.setFillColor(sf::Color::Black);
//
//		window.draw(box);
//
//
//		window.display();
//
//
//		//Set FPS
//
//		while (mainClock.getElapsedTime().asMilliseconds() < 1 / 0.06)
//		{
//		}
//
//		mainClock.restart();
//	}
//
//	return 0;
//}
