#include <Box2D/Box2D.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <conio.h>
#include <tuple>
#include <direct.h>
#include <algorithm>

#define PI 3.1415927

extern int PERPETUAL_ABIOGENESIS;

extern int PRED_PREY;

//BOX2D

extern const float TIME_STEP;

extern const int VELOCITY_ITERATIONS;

extern const int POSITION_ITERATIONS;





//WORLD

extern int WORLD_SIZE;

extern const int WALL_THICKNESS;

extern int NUM_STARTING_ORGANISMS;

extern int NUM_LIGHT_SOURCES;

extern float BACKGROUND_LIGHT_INTENSITY;

extern float SOURCE_LIGHT_INTENSITY;

extern const int LIGHT_PERIOD;

extern const int INITIAL_STATE_LENGTH;

extern float MAX_LIGHT_RADIUS;

const enum LIGHTING {FULL, SOURCES, DAY_NIGHT, ROTATING, QUADRANT, BOTH};


extern int LIGHTING_TYPE;

extern float MUTATION_RATE;

extern float MAX_TIME;

extern int GRAPHICS_ON;


//MODULES

extern const int MODULE_SIZE;

const enum SIDE {TOP, RIGHT, BOTTOM, LEFT};

extern const int NUM_SIDES;

extern const float CONNECTION_RANGE;

extern float CHARGING_RATE;

extern const float ABIOGENESIS_ENERGY_REQUIREMENT;


//INTERNALS

extern const float MODULE_MAX_ENERGY;

extern float MODULE_POWER_DRAW;

extern const int NUM_MODULE_TYPES;

const enum TYPE {SOLAR, PREDATOR, ARMOUR, ANCHOR, BOUNDARY, ANY};


extern const float PREDATOR_RANGE;

extern float PREDATOR_RATE;

const enum STATE {GROWING, REPRODUCING, DEAD};

const enum BIRTH_TYPE {BORN, CREATED};



//EVENT TYPES

const enum EVENT_TYPES {CREATION, BIRTH, DEATH};

//DRAWING

extern const int SCALING_FACTOR;

extern const sf::Vector2f SCREEN_SIZE;

extern const int TEXT_DRAW_TIMER;


//WRITING OUT

extern const int WRITE_PERIOD;


extern long long int NEW_ORGANISM_ID;


//MISC

std::default_random_engine& RandEngine();

float RandFloat();

void SeedRand(unsigned long seed);


struct Vector2iCompare
{
	bool operator() (const sf::Vector2i& lhs, const sf::Vector2i& rhs) const
	{
		if (lhs.x < rhs.x)
			return true;

		if (lhs.x > rhs.x)
			return false;

		if (lhs.y < rhs.y)
			return true;

		if (lhs.y > rhs.y)
			return false;

		return false;
	}
};