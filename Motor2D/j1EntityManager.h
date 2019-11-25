#ifndef __j1ENTITYMANAGER_H__
#define __j1ENTITYMANAGER_H__

#include "j1Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "j1Player.h"
#include "j1LandEnemy.h"
#include "p2Defs.h"
#include "j1Entity.h"

class j1Entity;


class j1EntityManager : public j1Module
{
	//------------Methods-----------//
public:
	//--------INTERNAL CONTROL---------//
	//Constructor
	j1EntityManager();

	//Destructor
	~j1EntityManager();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called each loop iteration
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();


	//--------SAVE & LOAD---------//
	//Called when loading a save
	bool Load(pugi::xml_node&);

	//Called to save the game
	bool Save(pugi::xml_node&) const;

	//--------ENTITY MANAGEMENT---------//
	//Function that adds entities
	j1Entity* AddEntity(entityType type, iPoint position);

	//Function that initializes entities
	bool InitEntity(j1Entity* tmp, pugi::xml_node& config);

	//Function that initializes entities
	bool CleanMapEnt();

	//Since player is used in other modules, we are going to have a pointer only to him
	j1Player* player = nullptr;

	//Config awake nodes; in order to awake entities in-game
	pugi::xml_node landEnemyNode;
	pugi::xml_node flyEnemyNode;
	pugi::xml_node playerNode;

private:
	p2List<j1Entity*> EntityList;
};
#endif