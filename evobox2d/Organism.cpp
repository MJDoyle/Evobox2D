#include "Organism.hpp"

Organism::Organism(b2World& world, b2Body* groundBody, b2Vec2 position, float rot, b2Vec2 linearVelocity, float angularVelocity, std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> modules, float energy, std::map<sf::Vector2i, int, Vector2iCompare> plan, int organismStatus, time_t programStartTime)
{
	//Set up output file for this organism

	m_parasiteJoint = NULL;

	m_isParasite = false;
	m_isHost = false;

	m_birthType = BORN;

	m_startingStatus = organismStatus;

	m_organismSize = 0;

	m_boundarySet = false;

	m_ID = -1;
	m_parentID = -1;

	m_startTime = programStartTime;

	m_plans.push_back(plan);

	m_modules = modules;

	m_groundBody = groundBody;

	m_world = &world;

	m_numActiveAnchors = 0;
	m_previouslyActiveAnchors = 0;

	m_lifetime = 0;

	m_reproduceNow = false;

	//for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
	//{
	//	std::cout << "NewModPos: " << modIt->second->GetPosition().x << " " << modIt->second->GetPosition().y << std::endl;
	//}

	SetPlan(plan);

	m_merging = false;

	m_completeOrganism = CheckComplete();

	m_organismStatus = organismStatus;

	//if (m_organismStatus != DEAD)
	//	OpenOutputFile();

	/*if(m_modules.size() <= 1)
		m_organismStatus = DEAD;

	else if (m_completeOrganism)
		m_organismStatus = REPRODUCING;

	else 
		m_organismStatus = GROWING;*/

	m_CoM = CalculateCoM();

	CenterMap();

	SetupInternals(energy);

	SetupPhysics(world, position, rot, linearVelocity, angularVelocity);

	//std::cout << "Modules: " << std::endl;

	//for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
	//{
	//	std::cout << modIt->first.first << " " << modIt->first.second << std::endl;
	//}

	//std::cout << std::endl;

	m_selected = false;


	if (m_organismStatus == DEAD)
		m_deathCooldown = 5;

	m_toKill = false;

	if (organismStatus == REPRODUCING)
	{
		//Find boundary module
		for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
		{
			if (modIt->second->GetType() == BOUNDARY)
			{
				m_boundaryPosition = modIt->first;

				m_boundarySet = true;
			}
		}

		m_organismSize = m_modules.size();

		//Get the neighbour of the boundary module
		m_replicationDirection = GetModuleNeighbours(m_boundaryPosition).back() - m_boundaryPosition;

		//Set the possible attachment sites for new modules
		m_attachmentPoints.clear();

		if (m_replicationDirection.x != 0)
		{
			m_attachmentPoints[m_boundaryPosition + sf::Vector2i(0, -1)] = BOUNDARY;
			m_attachmentPoints[m_boundaryPosition + sf::Vector2i(0, 1)] = BOUNDARY;
		}

		else
		{
			m_attachmentPoints[m_boundaryPosition + sf::Vector2i(-1, 0)] = BOUNDARY;
			m_attachmentPoints[m_boundaryPosition + sf::Vector2i(1, 0)] = BOUNDARY;
		}
	}

	for (auto mod = m_modules.begin(); mod != m_modules.end(); mod ++)
	{
		m_moduleAddList.push_back(mod->second->GetType());

		m_modulePosList.push_back(mod->first);
	}

	CountModuleNumbers();
}

Organism::~Organism()
{
	if (m_outputFile.is_open())
		m_outputFile.close();

	//m_world->DestroyJoint(m_frictionJoint);

	if (m_parasiteJoint != NULL)
		m_world->DestroyJoint(m_parasiteJoint);

	//Delete body and fixture pointers
	m_body->GetWorld()->DestroyBody(m_body);
}

sf::Vector2f Organism::CalculateCoM()
{
	sf::Vector2f centerOfMass = sf::Vector2f(0, 0);

	for (auto module = m_modules.begin(); module != m_modules.end(); module ++)
		centerOfMass += sf::Vector2f(module->first);

	centerOfMass.x /= float(m_modules.size());
	centerOfMass.y /= float(m_modules.size());

	return centerOfMass;
}

void Organism::Charge(float rate)
{
	//if (m_organismStatus != DEAD || m_modules.size() == 1)
	//{

	bool canCharge = false;

	if (m_organismStatus == REPRODUCING || m_organismStatus == GROWING)
		canCharge = true;

	if(PERPETUAL_ABIOGENESIS)
	{

		if (m_organismStatus != DEAD || m_modules.size() == 1)
			canCharge = true;
	}

	if (canCharge)
	{
		m_energy += rate * TIME_STEP;

		if (m_energy > m_maxEnergy)
			m_energy = m_maxEnergy;
	}


}

void Organism::SetupInternals(float energy)
{
	int numEnergy = 0;

	for (auto module = m_modules.begin(); module != m_modules.end(); module ++)
	{
		if (module->second->GetType() == SOLAR || module->second->GetType() == PREDATOR)
			numEnergy ++;
	}

	m_maxEnergy = numEnergy * MODULE_MAX_ENERGY;

	m_energy = energy;
}

bool Organism::GetDeathCooldown()
{
	if (m_deathCooldown >  0)
		return true;

	else 
		return false;
}

bool Organism::UpdateInternals(long double timestep)
{
	//Update lifetime
	if (m_organismStatus == REPRODUCING)
		m_lifetime ++;

	//Read out current timestep, status and energy to output file
	if (m_outputFile.is_open())
		m_outputFile << timestep << ',' << m_organismStatus << ',' << m_energy << '\n';

	if (m_deathCooldown > 0)
		m_deathCooldown -= TIME_STEP;


	//If parasite, check if host still exists
	if (m_isParasite)
	{
		if (m_host.expired())
			m_isParasite = false;
	}

	//If parasite, harvest energy from host
	if (m_isParasite)
	{
		float hostEnergy = m_host.lock()->GetEnergy();

		m_energy += std::min(hostEnergy, float(GetModuleTypeNumber(PREDATOR) * (m_host.lock()->GetModuleTypeNumber(SOLAR) - m_host.lock()->GetModuleTypeNumber(ARMOUR)) * TIME_STEP * PREDATOR_RATE));

	}


	//Host loses corresponding amount of energy
	if (m_isHost)
	{
		for (auto parasite = m_parasites.begin(); parasite != m_parasites.end(); parasite ++)
		{
			m_energy -= parasite->lock()->GetModuleTypeNumber(PREDATOR) * (GetModuleTypeNumber(SOLAR) - GetModuleTypeNumber(ARMOUR)) * TIME_STEP * PREDATOR_RATE;
		}
	}


	m_energy -= m_modules.size() * MODULE_POWER_DRAW * TIME_STEP;

	if (m_energy > m_maxEnergy)
		m_energy = m_maxEnergy;

	if (m_energy <= 0)
	{
		m_energy = 0;

		m_organismStatus = DEAD;

		if (m_modules.size() > 1)
			return false;
	}

	if (m_toKill)
		return false;

	return true;
}

void Organism::ResetPhysics(sf::Vector2f posOffset)
{
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = m_body->GetPosition() + b2Vec2(posOffset.x, posOffset.y);
	bodyDef.linearVelocity = m_body->GetLinearVelocity();

	bodyDef.allowSleep = false;
	bodyDef.angle = m_body->GetAngle();;
	bodyDef.angularVelocity = m_body->GetAngularVelocity();
	bodyDef.angularDamping = 0.4;
	bodyDef.linearDamping = 0.4;

	//Destroy old body and fixtures
	m_world->DestroyBody(m_body);

	//Create new body
	m_body = m_world->CreateBody(&bodyDef);


	//Add new fixtures

	for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
	{
		//std::cout << "Adding fixture" << std::endl;

		//Define a box shape for our dynamic body.
		b2PolygonShape dynamicBox;

		dynamicBox.SetAsBox(MODULE_SIZE / 2, MODULE_SIZE / 2);


		dynamicBox.m_vertices[0] = b2Vec2((modIt->second->GetComRelPosition().x - 0.5) * MODULE_SIZE, (modIt->second->GetComRelPosition().y - 0.5) * MODULE_SIZE);
		dynamicBox.m_vertices[1] = b2Vec2((modIt->second->GetComRelPosition().x + 0.5) * MODULE_SIZE, (modIt->second->GetComRelPosition().y - 0.5) * MODULE_SIZE);
		dynamicBox.m_vertices[2] = b2Vec2((modIt->second->GetComRelPosition().x + 0.5) * MODULE_SIZE, (modIt->second->GetComRelPosition().y + 0.5) * MODULE_SIZE);
		dynamicBox.m_vertices[3] = b2Vec2((modIt->second->GetComRelPosition().x - 0.5) * MODULE_SIZE, (modIt->second->GetComRelPosition().y + 0.5) * MODULE_SIZE);
		dynamicBox.m_centroid = b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE);


		/*dynamicBox.m_vertices[0] += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);
		dynamicBox.m_vertices[1] += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);
		dynamicBox.m_vertices[2] += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);
		dynamicBox.m_vertices[3] += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);
		dynamicBox.m_centroid += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);*/

		//Define the dynamic body fixture.
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;

		fixtureDef.restitution = 0.8;

		//Set the box density to be non-zero, so it will be dynamic.
		fixtureDef.density = 1.0f;

		//Override the default friction.
		fixtureDef.friction = 0.6f;

		//Add the shape to the body.
		m_body->CreateFixture(&fixtureDef);
	}


	/*m_world->DestroyJoint(m_frictionJoint);

	b2FrictionJointDef jointDef;
	jointDef.bodyA = m_groundBody;
	jointDef.bodyB = m_body;
	jointDef.maxForce = 0;
	jointDef.maxTorque = 0;

	m_frictionJoint = (b2FrictionJoint*)m_world->CreateJoint(&jointDef);*/
}

void Organism::SetupParasiteJoint(b2Body* otherBody, b2Vec2 thisPoint, b2Vec2 otherPoint)
{
	b2WeldJointDef jointDef;

	jointDef.bodyA = m_body;
	jointDef.bodyB = otherBody;
	jointDef.collideConnected = false;
	jointDef.localAnchorA = b2Vec2(thisPoint.x, thisPoint.y);
	jointDef.localAnchorB = b2Vec2(otherPoint.x, otherPoint.y);
	//jointDef.referenceAngle = otherBody->GetAngle() - m_body->GetAngle();

	float deltaAngle = otherBody->GetAngle() - m_body->GetAngle();

	float angle;

	if (deltaAngle > 0)
		angle = int(deltaAngle / (PI / float(2)) + float(0.5)) * PI / float(2);

	else
		angle = int(deltaAngle / (PI / float(2)) - float(0.5)) * PI / float(2);

	jointDef.referenceAngle = angle;

	m_parasiteJoint = (b2WeldJoint*)m_world->CreateJoint(&jointDef);
}

void Organism::SetupParasiteJoint(b2Body* otherBody, sf::Vector2f thisPoint, sf::Vector2f otherPoint)
{

	b2WeldJointDef jointDef;

	jointDef.bodyA = m_body;
	jointDef.bodyB = otherBody;
	jointDef.collideConnected = false;
	jointDef.localAnchorA = b2Vec2(thisPoint.x * MODULE_SIZE, thisPoint.y * MODULE_SIZE);
	jointDef.localAnchorB = b2Vec2(otherPoint.x * MODULE_SIZE, otherPoint.y * MODULE_SIZE);
	//jointDef.referenceAngle = otherBody->GetAngle() - m_body->GetAngle();

	float deltaAngle = otherBody->GetAngle() - m_body->GetAngle();

	float angle;

	if (deltaAngle > 0)
		angle = int(deltaAngle / (PI / float(2)) + float(0.5)) * PI / float(2);

	else
		angle = int(deltaAngle / (PI / float(2)) - float(0.5)) * PI / float(2);

	jointDef.referenceAngle = angle;

	m_parasiteJoint = (b2WeldJoint*)m_world->CreateJoint(&jointDef);
}

void Organism::ResetParasiteJoint()
{
	std::cout << "Reset joint1" << std::endl;

	if (m_parasiteJoint != NULL)
	{
		std::cout << "test: " << m_parasiteJoint->GetAnchorA().x << std::endl;

		m_world->DestroyJoint(m_parasiteJoint);
	}

	std::cout << "Reset joint2" << std::endl;

	m_parasiteJoint = NULL;
}

void Organism::SetupPhysics(b2World& world, b2Vec2 position, float rot, b2Vec2 linearVelocity, float angularVelocity)
{
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = position;
	bodyDef.linearVelocity = linearVelocity;

	bodyDef.allowSleep = false;
	bodyDef.angle = rot;
	bodyDef.angularVelocity = angularVelocity;
	bodyDef.angularDamping = 0.4;
	bodyDef.linearDamping = 0.4;

	m_body = world.CreateBody(&bodyDef);

	//Now add a fixture to the body for each module in the organism

	for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
	{
		//std::cout << "Adding fixture" << std::endl;

		//Define a box shape for our dynamic body.
		b2PolygonShape dynamicBox;

		dynamicBox.SetAsBox(MODULE_SIZE / 2, MODULE_SIZE / 2);


		dynamicBox.m_vertices[0] = b2Vec2((modIt->second->GetComRelPosition().x - 0.5) * MODULE_SIZE, (modIt->second->GetComRelPosition().y - 0.5) * MODULE_SIZE);
		dynamicBox.m_vertices[1] = b2Vec2((modIt->second->GetComRelPosition().x + 0.5) * MODULE_SIZE, (modIt->second->GetComRelPosition().y - 0.5) * MODULE_SIZE);
		dynamicBox.m_vertices[2] = b2Vec2((modIt->second->GetComRelPosition().x + 0.5) * MODULE_SIZE, (modIt->second->GetComRelPosition().y + 0.5) * MODULE_SIZE);
		dynamicBox.m_vertices[3] = b2Vec2((modIt->second->GetComRelPosition().x - 0.5) * MODULE_SIZE, (modIt->second->GetComRelPosition().y + 0.5) * MODULE_SIZE);
		dynamicBox.m_centroid = b2Vec2(modIt->second->GetComRelPosition().x * MODULE_SIZE, modIt->second->GetComRelPosition().y * MODULE_SIZE);


		/*dynamicBox.m_vertices[0] += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);
		dynamicBox.m_vertices[1] += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);
		dynamicBox.m_vertices[2] += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);
		dynamicBox.m_vertices[3] += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);
		dynamicBox.m_centroid += b2Vec2(modIt->first.x * MODULE_SIZE, modIt->first.y * MODULE_SIZE);*/

		//Define the dynamic body fixture.
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;

		fixtureDef.restitution = 0.8;

		//Set the box density to be non-zero, so it will be dynamic.
		fixtureDef.density = 1.0f;

		//Override the default friction.
		fixtureDef.friction = 0.6f;

		//Add the shape to the body.
		m_body->CreateFixture(&fixtureDef);
	}

	/*b2FrictionJointDef jointDef;
	jointDef.bodyA = m_groundBody;
	jointDef.bodyB = m_body;
	jointDef.maxForce = 0;
	jointDef.maxTorque = 0;

	m_frictionJoint = (b2FrictionJoint*)m_world->CreateJoint(&jointDef);*/
}

void Organism::AddModule(std::shared_ptr<Module> module)
{
	//Get world point of anchor position for parasite and host if needed 9and reset joints if needed)

	b2Vec2 hostAnchor(0, 0);

	std::vector<b2Vec2> parasiteAnchors;

	if (m_isParasite)
		b2Vec2 hostAnchor = m_parasiteJoint->GetAnchorB();

	if (m_isHost)
	{
		for (auto parasite = m_parasites.begin(); parasite != m_parasites.end(); parasite ++)
		{
			parasiteAnchors.push_back(parasite->lock()->GetParasiteJointLocalPointA());

			//Don't need to reset parasite joint as this is reset when host is destroyed
			parasite->lock()->ResetParasiteJoint();
		}
	}


	m_moduleAddList.push_back(module->GetType());

	m_modulePosList.push_back(module->GetPosition());

	//Add module to map
	if (m_modules.count(sf::Vector2i(module->GetPosition().x, module->GetPosition().y)))
	{
		std::cout << "ALERT!" << std::endl;

		while (true);
	}

	m_modules[sf::Vector2i(module->GetPosition().x, module->GetPosition().y)] = module;

	//Need to adjust robot position as new module is added

	sf::Vector2f CoMdelta = CalculateCoM() - m_CoM;

	CoMdelta.x *= MODULE_SIZE;
	CoMdelta.y *= MODULE_SIZE;

	sf::Vector2f positionDelta = sf::Vector2f(CoMdelta.x * cos(m_body->GetAngle()) - CoMdelta.y * sin(m_body->GetAngle()), CoMdelta.x * sin(m_body->GetAngle()) + CoMdelta.y * cos(m_body->GetAngle()));

	

	m_CoM = CalculateCoM();

	CenterMap();

	//Setup physics (add fixture)

	//std::cout << "Adding fixture" << std::endl;

	//Check and update the current state of the organism
	
	//Check for boundary

	int numBoundaries = 0;


	for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
	{
		//std::cout << "MODS: " << modIt->first.x << " " << modIt->first.y << std::endl;

		if (modIt->second->GetType() == BOUNDARY)
		{
			numBoundaries ++;

			if (!m_boundarySet)
			{
				m_boundaryPosition = modIt->first;

				m_boundarySet = true;
			}
		}
	}

	//If the previous state was GROWING (or DEAD) and now the organism has a boundary, set the growth direction and the organism size for the organism to be replicated
	if (numBoundaries && m_modules.size() > 1 && (m_organismStatus == GROWING || m_organismStatus == DEAD))
	{
		//m_ID = NEW_ORGANISM_ID++;

		m_generation = 1;

		m_organismSize = m_modules.size();

		//Get the neighbour of the boundary module
		m_replicationDirection = GetModuleNeighbours(m_boundaryPosition).back() - m_boundaryPosition;
	}

	//If the previous state was REPRODUCING, the connected module must be a child, so add it to child module map
	if (m_organismStatus == REPRODUCING)
		m_childModules[module->GetPosition()] = module;	

	//Set new state
	if (numBoundaries)
		m_organismStatus = REPRODUCING;

	else
		m_organismStatus = GROWING;


	//Set the possible attachment sites for new modules
	
	m_attachmentPoints.clear();

	if (m_organismStatus == GROWING)
	{
		for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
		{
			//Check module neighbours. If it has two neighbours, it won't accept any new modules. If it has one connection it will accept a new module on the opposite face.
			std::vector<sf::Vector2i> neighbours = GetModuleNeighbours(modIt->first);

			if (neighbours.size() == 1)
			{
				m_attachmentPoints[modIt->first - (neighbours.back() - modIt->first)] = ANY;
			}
		}
	}

	else if (m_organismStatus == REPRODUCING)
	{
		//If the organism only has one boundary, there will be two attachment points, one on either side of the boundary (growth direction rotated 90 and -90)
		if (numBoundaries == 1)
		{
			//std::cout << "ONE BOUND: " << m_boundaryPosition.x << " " << m_boundaryPosition.y << " DIR: " << m_replicationDirection.x << " " << m_replicationDirection.y << std::endl;


			if (m_replicationDirection.x != 0)
			{
				m_attachmentPoints[m_boundaryPosition + sf::Vector2i(0, -1)] = BOUNDARY;
				m_attachmentPoints[m_boundaryPosition + sf::Vector2i(0, 1)] = BOUNDARY;
			}

			else
			{
				m_attachmentPoints[m_boundaryPosition + sf::Vector2i(-1, 0)] = BOUNDARY;
				m_attachmentPoints[m_boundaryPosition + sf::Vector2i(1, 0)] = BOUNDARY;
			}
		}

		//If the organism

		else
		{
			if (m_modules.size() < m_organismSize * 2)
			{
				/*std::cout << "CHECK: " << m_organismStatus << " " << numBoundaries << " " << module->GetType() << " " << m_body->GetPosition().x << " " << m_body->GetPosition().y << " " << m_modules.size() << " " << m_startingStatus << std::endl;


				for (int i = 0; i != m_moduleAddList.size(); i ++)
				{
					std::cout << m_moduleAddList[i] << " " << m_modulePosList[0].x << " " << m_modulePosList[0].y << std::endl;
				}

				std::cout << std::endl;

				for (auto mod = m_modules.begin(); mod != m_modules.end(); mod++)
				{
					std::cout << mod->second->GetType() << " " << mod->first.x << " " << mod->first.y << std::endl;
				}

				std::cout << std::endl << std::endl;*/

				m_attachmentPoints[module->GetPosition() + m_replicationDirection] = m_modules[m_boundaryPosition + sf::Vector2i(m_replicationDirection.x * (m_modules.size() - m_organismSize), m_replicationDirection.y * (m_modules.size() - m_organismSize))]->GetType();
			}

			else
			{
				m_attachmentPoints[module->GetPosition() + m_replicationDirection] = ANY;
			}
		}
	}




	//Check no two module occupy same positions

	/*for (auto mod = m_modules.begin(); mod != m_modules.end(); mod++)
	{
		for (auto mod2 = m_modules.begin(); mod2 != m_modules.end(); mod2++)
		{
			if (mod != mod2)
			{
				if (mod->first 
			}
		}
	}*/



	/*m_completeOrganism = CheckComplete();

	if (m_organismStatus == GROWING && m_completeOrganism)
		m_organismStatus = REPRODUCING;

	if (m_organismStatus != DEAD)
		OpenOutputFile();*/

	CountModuleNumbers();

	ResetPhysics(positionDelta);

	//Decide whether to reproduce or not

	//Small mutation chance for shortened organism to form

	if (m_childModules.size() > 1)
	{
		int numParentModules = m_modules.size() - m_childModules.size();


		//Small chance to reproduce early with not enough modules
		if (m_childModules.size() < numParentModules)
		{
			if (RandFloat() * (numParentModules - m_childModules.size()) < MUTATION_RATE * 10)
				m_reproduceNow = true;
		}

		//Large chance to reproduce with enough modules
		else if (m_childModules.size() >= numParentModules)
		{
			if (RandFloat() + 0.1 * (m_childModules.size() - numParentModules) >= MUTATION_RATE * 10)
				m_reproduceNow = true;


		}

	}

	//Reset host/parasite state (and the parasite/host state of the corresponding parasite/host)

	if (m_isHost)
	{
		//If now armour >= solar then no longer host
		if (GetModuleTypeNumber(ARMOUR) >= GetModuleTypeNumber(SOLAR))
		{
			m_isHost = false;

			for (auto parasite = m_parasites.begin(); parasite != m_parasites.end(); parasite ++)
			{
				std::cout << "Parasite1: " << parasite->lock()->GetBody()->GetPosition().x << " " << parasite->lock()->GetBody()->GetPosition().y << parasite->lock()->GetIsParasite();

				if (parasite->lock()->GetIsParasite())
					std::cout << " is parasite" << std::endl;

				else
					std::cout << " is not parasite" << std::endl;

				parasite->lock()->SetIsParasite(false);
				parasite->lock()->ResetHost();

				//Don't need to reset parasite joint as this is reset when host is destroyed
				//parasite->lock()->ResetParasiteJoint(); 

				std::cout << "End" << std::endl;

			}

			m_parasites.clear();
		}

		//If not, parasites stay as they are, remake parasite joint(s)
		else
		{
			int i = 0;

			for (auto parasite = m_parasites.begin(); parasite != m_parasites.end(); parasite ++)
			{
				std::cout << "Parasite2: " << parasite->lock()->GetBody()->GetPosition().x << " " << parasite->lock()->GetBody()->GetPosition().y << parasite->lock()->GetIsParasite() << std::endl;

				//Don't need to reset parasite joint as this is reset when host is destroyed
				//parasite->lock()->ResetParasiteJoint();

				parasite->lock()->SetupParasiteJoint(m_body, parasiteAnchors[i], m_body->GetLocalPoint(parasite->lock()->GetBody()->GetWorldPoint(parasiteAnchors[i])));

				std::cout << "End" << std::endl;

				i++;
			}
		}
	}

	if (m_isParasite)
	{
		//Adding module still acts as parasite. Just reset joint
		//localA is the parasite connection
		//localB is the host connection

		SetupParasiteJoint(m_host.lock()->GetBody(), m_body->GetLocalPoint(m_host.lock()->GetBody()->GetWorldPoint(hostAnchor)), hostAnchor);

		//SetupParasiteJoint()
	}

	/*if (m_isHost)
	{
		m_isHost = false;

		for (auto parasite = m_parasites.begin(); parasite != m_parasites.end(); parasite ++)
		{

			parasite->lock()->SetIsParasite(false);
			parasite->lock()->ResetHost();
		}

		m_parasites.clear();
	}

	if (m_isParasite)
	{
		m_isParasite = false;

		m_host.lock()->RemoveParasite(shared_from_this());

		if (m_host.lock()->GetParasites().size() == 1)
			m_host.lock()->SetIsHost(false);

		m_host.reset();
	}*/
}

void Organism::CountModuleNumbers()
{
	m_moduleNumbers.clear();

	for (auto module = m_modules.begin(); module != m_modules.end(); module++)
	{
		if (m_moduleNumbers.count(module->second->GetType()))
			m_moduleNumbers[module->second->GetType()] ++;

		else
			m_moduleNumbers[module->second->GetType()] = 1;

	}
}

int Organism::GetModuleTypeNumber(int type)
{
	if (m_moduleNumbers.count(type))
		return m_moduleNumbers[type];

	else
		return 0;
}

bool Organism::Update(long double timestep)
{

	//Deal with anchors and friction

	//Check if anchor status has changed

	if (m_numActiveAnchors)
	{
		m_body->SetLinearDamping(5 * m_numActiveAnchors);
	}

	else
	{		
	m_body->SetLinearDamping(0);
	}


	//Reset anchor numbers
	m_previouslyActiveAnchors = m_numActiveAnchors;
	m_numActiveAnchors = 0;





	//TODO - THIS NEEDS REWORKING
	

	//Create force to apply to organism

	//Normalize position vector
	b2Vec2 position = m_body->GetPosition();

	position.Normalize();


	//Now rotate it
	b2Vec2 force;

	force.x = position.x * cos(PI / 2) - position.y * sin(PI / 2);
	force.y = position.x * sin(PI / 2) + position.y * cos(PI / 2);


	//Now multiply by force magnitude
	force = std::sqrt(float(m_modules.size())) * force;

	m_body->ApplyForceToCenter(force, false);

	//Check that each module is within the walls, else move it out


	return UpdateInternals(timestep);
}

void Organism::Draw(sf::RenderWindow& window)
{
	for (auto modIt = m_modules.begin(); modIt != m_modules.end(); modIt ++)
	{

		sf::RectangleShape box(sf::Vector2f(MODULE_SIZE * SCALING_FACTOR, MODULE_SIZE * SCALING_FACTOR));

		box.setOrigin(MODULE_SIZE * SCALING_FACTOR / 2, MODULE_SIZE * SCALING_FACTOR / 2);


		//box.setOutlineThickness(2);
		//box.setOutlineColor(sf::Color::Black);

		//modIt->first.first



		sf::Vector2f modPosition;

		modPosition.x = modIt->second->GetComRelPosition().x * cos(m_body->GetAngle()) - modIt->second->GetComRelPosition().y * sin(m_body->GetAngle());
		modPosition.y = modIt->second->GetComRelPosition().x * sin(m_body->GetAngle()) + modIt->second->GetComRelPosition().y * cos(m_body->GetAngle());


		box.setPosition(m_body->GetPosition().x * SCALING_FACTOR + modPosition.x * MODULE_SIZE * SCALING_FACTOR, m_body->GetPosition().y * SCALING_FACTOR + modPosition.y * MODULE_SIZE * SCALING_FACTOR);

		box.setRotation(m_body->GetAngle() * float(180) / PI);

		if ((modIt)->second->GetType() == SOLAR)
			box.setFillColor(sf::Color(0, 0, 100, 55 + (m_energy / (m_maxEnergy + 0.01)) * 200));

		else if ((modIt)->second->GetType() == ANCHOR)
			box.setFillColor(sf::Color(0, 100, 0, 55 + (m_energy / (m_maxEnergy + 0.01)) * 200));

		else if ((modIt)->second->GetType() == BOUNDARY)
			box.setFillColor(sf::Color(255, 165, 0, 55 + (m_energy / (m_maxEnergy + 0.01)) * 200));

		else if ((modIt)->second->GetType() == PREDATOR)
			box.setFillColor(sf::Color(255, 0, 255, 55 + (m_energy / (m_maxEnergy + 0.01)) * 200));

		else if ((modIt)->second->GetType() == ARMOUR)
			box.setFillColor(sf::Color(255, 255, 0, 55 + (m_energy / (m_maxEnergy + 0.01)) * 200));

		window.draw(box);
	}

	//for (auto mod = m_modules.begin(); mod != m_modules.end(); mod ++)
	//{
	//	for (int i = 0; i != 4; i++)
	//	{
	//		sf::CircleShape circle(4);

	//		circle.setFillColor(sf::Color::Red);
	//		circle.setOrigin(4, 4);
	//		circle.setPosition(sf::Vector2f(GetSidePosition(i, mod->first).x * SCALING_FACTOR, GetSidePosition(i, mod->first).y * SCALING_FACTOR));

	//		window.draw(circle);
	//	}
	//}




	if (m_selected)
	{
		sf::CircleShape blob(MODULE_SIZE * SCALING_FACTOR * 0.5);

		blob.setOrigin(MODULE_SIZE * SCALING_FACTOR * 0.5, MODULE_SIZE * SCALING_FACTOR * 0.5);

		blob.setFillColor(sf::Color(0, 200, 0, 150));

		blob.setPosition(m_body->GetPosition().x * SCALING_FACTOR, m_body->GetPosition().y * SCALING_FACTOR);

		window.draw(blob);
	}
}

void Organism::CenterMap()
{
	for (auto module = m_modules.begin(); module != m_modules.end(); module ++)
		module->second->SetComRelPosition(sf::Vector2f(float(module->first.x) - m_CoM.x, float(module->first.y) - m_CoM.y));
}

int Organism::GetPlanNeighbouringModuleType(std::pair<int, int> modPos, int side)
{
	//int type = -1;

	//if (side == TOP && m_plan.count(std::make_pair(modPos.first, modPos.second - 1)))
	//	type = m_plan[std::make_pair(modPos.first, modPos.second - 1)];

	//else if (side == RIGHT && m_plan.count(std::make_pair(modPos.first + 1, modPos.second)))
	//	type = m_plan[std::make_pair(modPos.first + 1, modPos.second)];

	//else if (side == BOTTOM && m_plan.count(std::make_pair(modPos.first, modPos.second + 1)))
	//	type = m_plan[std::make_pair(modPos.first, modPos.second + 1)];

	//else if (side == LEFT && m_plan.count(std::make_pair(modPos.first - 1, modPos.second)))
	//	type = m_plan[std::make_pair(modPos.first - 1, modPos.second)];

	//return type;

	return -1;
}

bool Organism::GetModuleInPlan(sf::Vector2i modPos, int side, int type)
{
	//std::cout << "Pos: " << modPos.x << " " << modPos.y << " Side: " << side << " Type: " << type << std::endl;

	for (auto plan = m_plans.begin(); plan != m_plans.end(); plan ++)
	{
		if (side == TOP && plan->count(modPos + sf::Vector2i(0, -1)) && (*plan)[modPos + sf::Vector2i(0, -1)] == type)
			return true; 

		else if (side == BOTTOM && plan->count(modPos + sf::Vector2i(0, 1)) && (*plan)[modPos + sf::Vector2i(0, 1)] == type)
			return true; 

		else if (side == RIGHT && plan->count(modPos + sf::Vector2i(1, 0)) && (*plan)[modPos + sf::Vector2i(1, 0)] == type)
			return true; 

		else if (side == LEFT && plan->count(modPos + sf::Vector2i(-1, 0)) && (*plan)[modPos + sf::Vector2i(-1, 0)] == type)
			return true; 
	}

	return false;
}

b2Vec2 Organism::GetSidePosition(int side, sf::Vector2f position)
{
	b2Vec2 localPoint;

	if (side == TOP)
	{
		localPoint.x = position.x * MODULE_SIZE;
		localPoint.y = (position.y - 0.5) * MODULE_SIZE;
	}

	else if (side == RIGHT)
	{
		localPoint.x = (position.x + 0.5) * MODULE_SIZE;
		localPoint.y = position.y* MODULE_SIZE;
	}

	else if (side == BOTTOM)
	{
		localPoint.x = position.x * MODULE_SIZE;
		localPoint.y = (position.y + 0.5) * MODULE_SIZE;
	}

	else if (side == LEFT)
	{
		localPoint.x = (position.x - 0.5) * MODULE_SIZE;
		localPoint.y = position.y* MODULE_SIZE;
	}

	return m_body->GetWorldPoint(localPoint);
}

b2Vec2 Organism::GetModulePosition(sf::Vector2f position)
{
	return m_body->GetWorldPoint(b2Vec2(position.x * MODULE_SIZE, position.y * MODULE_SIZE));
}


bool Organism::CheckComplete()
{
	//Run through all variants of the plan and cross check them with the current module map. Any that are no longer 
	
	for (auto plan = m_plans.begin(); plan != m_plans.end(); )
	{
		bool valid = true;

		//Check if the plan is valid
		for (auto module = m_modules.begin(); module != m_modules.end(); module ++)
		{
			if (!plan->count(sf::Vector2i(module->first.x, module->first.y)) || (*plan)[sf::Vector2i(module->first.x, module->first.y)] != module->second->GetType())
				valid = false;
		}

		//if not valid, remove it from the list

		if (!valid)
			plan = m_plans.erase(plan);

		else
			plan ++;
	}


	//std::cout << "PLANS: " << m_plans.size() << std::endl;


	//Now check if any of the plans are complete (possible to have multiple completed ones)

	bool anyComplete = false;

	for (auto plan = m_plans.begin(); plan != m_plans.end(); plan ++) 
	{
		bool complete = true;

		for (auto planModule = plan->begin(); planModule != plan->end(); planModule ++)
		{
			if (!m_modules.count(planModule->first) || m_modules[planModule->first]->GetType() != planModule->second)
				complete = false;
		}

		if (complete)
		{
			anyComplete = true;
			break;
		}
	}


	if (!anyComplete)
		return false;

	else
	{
		std::map<sf::Vector2i, int, Vector2iCompare> plan = m_plans.back();

		m_plans.clear();

		m_plans.push_back(plan);

		return true;
	}




	/*if (m_modules.size() != m_plan.size())
		return false;

	for (auto modIt = m_plan.begin(); modIt != m_plan.end(); modIt ++)
	{
		if (m_modules.count(modIt->first))
		{
			if (m_modules[modIt->first]->GetType() != modIt->second)
				return false;
		}

		else
			return false;
	}*/
}

//This is used to set the plan only when a dead organism (single module at (0, 0)) is being seeded
void Organism::SetPlan(std::map<sf::Vector2i, int, Vector2iCompare> plan)
{
	m_plans.clear();

	/*std::cout << "New plan: " << std::endl;

	for (auto mod = plan.begin(); mod != plan.end(); mod ++)
	{
		std::cout << mod->first.x << " " << mod->first.y << std::endl;
	}*/

	//Run through the plan, find all locations that have the same type of module as the one at (0, 0)
	for (auto planMod = plan.begin(); planMod != plan.end(); planMod ++)
	{
		//For each such location there will be 4 variants of the plan (4-way rotation)
		if (planMod->second == m_modules[sf::Vector2i(0, 0)]->GetType())
		{
			for (int i = 0; i != 4; i ++)
			{
				std::map<sf::Vector2i, int, Vector2iCompare> shiftedPlan;

				//std::cout << "Shift: " << std::endl;

				//First shift all the module positions in the plan so that the original module will still be at (0, 0)
				for (auto planMod2 = plan.begin(); planMod2 != plan.end(); planMod2 ++)
				{
					shiftedPlan[planMod2->first - planMod->first] = planMod2->second;

					//std::cout << "Mod: " << (planMod2->first - planMod->first).x << " " << (planMod2->first - planMod->first).y << std::endl;
				}

				std::map<sf::Vector2i, int, Vector2iCompare> rotatedPlan;

				//std::cout << "Rotate: " << std::endl;

				//Then rotate all positions by 0/90/180/270 about (0, 0)
				for (auto planMod2 = shiftedPlan.begin(); planMod2 != shiftedPlan.end(); planMod2 ++)
				{
					sf::Vector2i rotatedCoords;

					//Avoid rounding errors
					float cosAngle = cos(i * PI / float(2));

					float sinAngle = sin(i * PI / float(2));

					if (cosAngle > 0)
					{
						if (cosAngle < 0.5)
							cosAngle = 0;

						else
							cosAngle = 1;
					}

					else
					{
						if (cosAngle < -0.5)
							cosAngle = -1;

						else
							cosAngle = 0;
					}

					rotatedCoords.x = planMod2->first.x * cosAngle - planMod2->first.y * sinAngle;
					rotatedCoords.y = planMod2->first.x * sinAngle + planMod2->first.y * cosAngle;

					rotatedPlan[rotatedCoords] = planMod2->second;

					//std::cout << "Mod: " << rotatedCoords.x << " " << rotatedCoords.y << std::endl;
				}

				//Now add plan to list of plans
				m_plans.push_back(rotatedPlan);
			}
		}
	}

	/*std::cout << "PLANS: " << std::endl;

	for (auto plan = m_plans.begin(); plan != m_plans.end(); plan ++)
	{
		std::cout << "plan: " << std::endl;

		for (auto planModule = plan->begin(); planModule != plan->end(); planModule ++)
		{
			std::cout << planModule->first.x << " " << planModule->first.y << " " << planModule->second << std::endl;
		}
	}*/

}

void Organism::OpenOutputFile()
{
	std::stringstream ss;

	ss << "Output/Org";

	struct tm * now = localtime( & m_startTime );

	ss << (now->tm_year + 1900) << '-' 
		<< (now->tm_mon + 1) << '-'
		<<  now->tm_mday << '-'
		<< now->tm_hour << '-'
		<< now->tm_min << '-'
		<< now->tm_sec << '-'
		<< m_ID
		<< ".csv";

	m_outputFile.open(ss.str());
}

std::vector<sf::Vector2i> Organism::GetModuleNeighbours(sf::Vector2i position)
{
	std::vector<sf::Vector2i> neighbours;

	if (m_modules.count(position + sf::Vector2i(0, -1)))
		neighbours.push_back(position + sf::Vector2i(0, -1));

	if (m_modules.count(position + sf::Vector2i(0, 1)))
		neighbours.push_back(position + sf::Vector2i(0, 1));

	if (m_modules.count(position + sf::Vector2i(1, 0)))
		neighbours.push_back(position + sf::Vector2i(1, 0));

	if (m_modules.count(position + sf::Vector2i(-1, 0)))
		neighbours.push_back(position + sf::Vector2i(-1, 0));

	return neighbours;
}

void Organism::ClearChildModules()
{
	m_childModules.clear();
}

std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> Organism::GetParentModules()
{
	std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> parentModules;

	for (auto module = m_modules.begin(); module != m_modules.end(); module ++)
	{
		if (!m_childModules.count(module->first))
			parentModules[module->first] = module->second;
	}

	return parentModules;
}

bool Organism::HasModuleHere(sf::Vector2i pos)
{
	if (m_modules.count(pos))
		return true;

	else
		return false;
}

void Organism::RemoveParasite(std::weak_ptr<Organism> parasite)
{
	for (auto parasite2 = m_parasites.begin(); parasite2 != m_parasites.end(); )
	{
		if (parasite2->lock() == parasite.lock())
			parasite2 = m_parasites.erase(parasite2);

		else
			parasite2++;

	}
}