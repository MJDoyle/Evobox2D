#ifndef ORGANISM_HPP
#define ORGANISM_HPP

#include "Module.hpp"

class Organism : public std::enable_shared_from_this<Organism>
{
	public:

		Organism(b2World& world, b2Body* groundBody, b2Vec2 position, float rot, b2Vec2 linearVelocity, float angularVelocity, std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> modules, float energy, std::map<sf::Vector2i, int, Vector2iCompare> plan, int organismStatus, time_t programStartTime);

		~Organism();

		void Draw(sf::RenderWindow& window);

		bool Update(long double timestep);

		void AddModule(std::shared_ptr<Module> module);

		void AddMaxEnergy(float maxEnergy) {m_maxEnergy += maxEnergy;}

		void SetMerging(bool merging) {m_merging = merging;}

		void SetStatus(int status) {m_organismStatus = status;}

		bool GetMerging() {return m_merging;}
		int GetStatus() {return m_organismStatus;}

		int GetPlanNeighbouringModuleType(std::pair<int, int> modPos, int side);

		bool GetModuleInPlan(sf::Vector2i modPos, int side, int type);

		std::map<sf::Vector2i, int, Vector2iCompare> GetPlan() {return m_plans.back();}

		void SetPlan(std::map<sf::Vector2i, int, Vector2iCompare> plan);

		//Add a chunk of energy
		void AddEnergy(float energy) {m_energy = std::min(m_energy + energy, m_maxEnergy);}

		//Remove a chunk of energy
		void RemoveEnergy(float energy) {m_energy = std::max(m_energy - energy, float(0));}

		void Charge(float rate);

		std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare>& GetModules() {return m_modules;}

		b2Body* GetBody() {return m_body;}

		b2Vec2 GetSidePosition(int side, sf::Vector2f position);

		b2Vec2 GetModulePosition(sf::Vector2f position);

		float GetEnergy() {return m_energy;}
		float GetMaxEnergy() {return m_maxEnergy;}

		//This is used after merging to set the correct position of each module
		void CenterMap();

		void SetSelected(bool selected) {m_selected = selected;}

		void AddActiveAnchor() {m_numActiveAnchors++;}

		int GetActiveAnchors() {return m_numActiveAnchors;}
		int GetPreviousAnchors() {return m_previouslyActiveAnchors;}

		void SetToKill() {m_toKill = true;}

		bool GetDeathCooldown();

		int GetTotalSize() {return m_modules.size();}

		int GetOrganismSize() {return m_organismSize;}

		std::map<sf::Vector2i, int, Vector2iCompare> GetAttachmentPoints() {return m_attachmentPoints;}

		void ClearChildModules();

		std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> GetChildModules() {return m_childModules;}

		std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> GetParentModules();

		bool HasModuleHere(sf::Vector2i pos);

		long long GetLifetime() {return m_lifetime;}

		int GetBirthType() {return m_birthType;}

		void SetBirthType(int birthType) {m_birthType = birthType;}

		void CountModuleNumbers();

		int GetModuleTypeNumber(int type);

		long long GetID() {return m_ID;}
		long long GetParentID() {return m_parentID;}

		void SetID(long long ID) {m_ID = ID;}
		void SetParentID(long long parentID) {m_parentID = parentID;}

		void SetGeneration(long long generation) {m_generation = generation;}

		long long GetGeneration() {return m_generation;}

		bool GetReproduceNow() {return m_reproduceNow;}

		void SetupParasiteJoint(b2Body* otherBody, sf::Vector2f thisPoint, sf::Vector2f otherPoint);

		void SetupParasiteJoint(b2Body* otherBody, b2Vec2 thisPoint, b2Vec2 otherPoint);

		void ResetParasiteJoint();

		bool m_isParasite;
		bool m_isHost;

		bool GetIsParasite() {return m_isParasite;}
		bool GetIsHost() {return m_isHost;}

		std::weak_ptr<Organism> GetHost() {return m_host;}

		std::vector<std::weak_ptr<Organism>> GetParasites() {return m_parasites;}

		void SetIsParasite(bool isParasite) {m_isParasite = isParasite;}
		void SetIsHost(bool isHost) {m_isHost = isHost;}

		void SetHost(std::weak_ptr<Organism> host) {m_host = host;}

		void ResetHost() {m_host.reset();}

		void AddParasite(std::weak_ptr<Organism> parasite) {m_parasites.push_back(parasite);}

		void RemoveParasite(std::weak_ptr<Organism> parasite);

		b2Vec2 GetParasiteJointLocalPointA() {return m_parasiteJoint->GetAnchorA();}


	private:

		bool m_reproduceNow;

		long long m_generation;

		//Box2D body
		b2Body* m_body;

		//Ground body
		b2Body* m_groundBody;

		//World
		b2World* m_world;

		//Number of active anchords (used for calculating friction)
		int m_numActiveAnchors;

		//Cooldown period after dying before it can be used again
		float m_deathCooldown;

		//If number has changed, recalculat friction
		int m_previouslyActiveAnchors;

		//Joint for friction
		//b2FrictionJoint* m_frictionJoint;

		//Joint for parasite connection
		b2WeldJoint* m_parasiteJoint;

		std::weak_ptr<Organism> m_host;

		std::vector<std::weak_ptr<Organism>> m_parasites;

		bool m_toKill;

		void SetupPhysics(b2World& world, b2Vec2 position, float rot, b2Vec2 linearVelocity, float angularVelocity);

		void ResetPhysics(sf::Vector2f posOffset);

		std::map<sf::Vector2i, std::shared_ptr<Module>, Vector2iCompare> m_modules;

		//Weak pointers to the child modules
		std::map<sf::Vector2i, std::weak_ptr<Module>, Vector2iCompare> m_childModules;

		//List of module types
		std::map<int, int> m_moduleNumbers;

		int m_organismSize;

		std::vector<std::map<sf::Vector2i, int, Vector2iCompare>> m_plans; 

		bool m_merging;

		bool m_completeOrganism;

		bool m_selected;

		int m_organismStatus;

		int m_birthType;

		std::vector<sf::Vector2i> GetModuleNeighbours(sf::Vector2i position);

		std::map<sf::Vector2i, int, Vector2iCompare> m_attachmentPoints;

		sf::Vector2i m_replicationDirection;

		//sf::Vector2i m_replicationSide;

		sf::Vector2i m_boundaryPosition;

		bool m_boundarySet;

		//Internals

		void SetupInternals(float energy);

		bool UpdateInternals(long double timestep);

		//Check if the organism is complete (does the module map match the plan)
		bool CheckComplete();

		float m_energy;

		float m_maxEnergy;

		//Output file for this organism
		std::ofstream m_outputFile;

		long long int m_ID;

		long long m_parentID;

		void OpenOutputFile();

		time_t m_startTime;


		long long m_lifetime;


		//For recrods

		short m_startingStatus;

		std::vector<short> m_moduleAddList;

		std::vector<sf::Vector2i> m_modulePosList;

		sf::Vector2f m_CoM;

		sf::Vector2f CalculateCoM();
};

#endif