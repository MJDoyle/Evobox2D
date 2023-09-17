#include "Config.hpp"

int PERPETUAL_ABIOGENESIS = 0;

int PRED_PREY = 1;

//BOX2D

//const float TIME_STEP = float(1) / float(60);

const float TIME_STEP = 0.05;

const int VELOCITY_ITERATIONS = 6;

const int POSITION_ITERATIONS = 2;




//WORLD

int WORLD_SIZE = 143;

const int WALL_THICKNESS = 50;

int NUM_STARTING_ORGANISMS = 150;

int NUM_LIGHT_SOURCES = 3;

float BACKGROUND_LIGHT_INTENSITY = 1;

float SOURCE_LIGHT_INTENSITY = 0;

const int LIGHT_PERIOD = 50;

const int INITIAL_STATE_LENGTH = 2;

float MAX_LIGHT_RADIUS = 20;


int LIGHTING_TYPE = FULL;


float MUTATION_RATE = 0.001;

float MAX_TIME = 21600; //6 hours

int GRAPHICS_ON = 1;

//MODULES

const int MODULE_SIZE = 2;

const int NUM_SIDES = 4;

const float CONNECTION_RANGE = 0.5;		//Was 0.5

float CHARGING_RATE = 0.3;
//const float CHARGING_RATE = 0;

const float PREDATOR_RANGE = 10;

float PREDATOR_RATE = 0.5;

const float ABIOGENESIS_ENERGY_REQUIREMENT = 0.2;

//INTERNALS

const float MODULE_MAX_ENERGY = 300;

float MODULE_POWER_DRAW = 0.1;
//const float MODULE_POWER_DRAW = 0;

const int NUM_MODULE_TYPES = 3;


//DRAWING

const int SCALING_FACTOR = 10;

const sf::Vector2f SCREEN_SIZE = sf::Vector2f(1000, 1000);

const int TEXT_DRAW_TIMER = 100;

//WRITING OUT

const int WRITE_PERIOD = 100;

long long int NEW_ORGANISM_ID = 0;



//MISC

std::default_random_engine & RandEngine()
{
	static std::default_random_engine e;

	return e;
}

float RandFloat()
{
	static std::uniform_real_distribution<float> d(0, 1);

	return d(RandEngine());
}

void SeedRand(unsigned long seed) {RandEngine().seed(seed);}