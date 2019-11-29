#ifndef __j1FLYINGENEMY_H__
#define __j1FLYINGENEMY_H__

#include "j1Entity.h"
#include "p2Defs.h"
#include "p2List.h"
#include "p2Point.h"
#include "p2DynArray.h"

#define CHASING_DISTANCE 200

enum class flying_state : int
{
	ST_UNKNOWN = -1,

	ST_IDLE,
	ST_CHASING,

	ST_ALL,
};


class j1FlyingEnemy : public j1Entity
{
public:

	//--------INTERNAL CONTROL---------//
	//Constructor
	j1FlyingEnemy();

	//Destructor
	~j1FlyingEnemy();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called each loop iteration
	bool PostUpdate(bool debug);

	// Called before quitting
	bool CleanUp();


	//--------SAVE & LOAD---------//
	//Called when loading a save
	bool Load(pugi::xml_node&);

	//Called to save the game
	bool Save(pugi::xml_node&) const;

	//--------POSITION ---------//
	void UpdatePos(float dt);

	//When the enemy is idle, it has a defined movement
	virtual void TraceFollower(float dt);

	//--------COLLISION ---------//

	//If a collision is detected by the j1Collision, distributes collisions according to it's type
	void OnCollision(Collider* playerCol, Collider* coll);

	//Calculate the collisions with the enviroment
	void RecalculatePos(SDL_Rect, SDL_Rect);

	//--------DRAW---------//
	//Blits the entity into the world
	virtual void Draw();


private:
	//--------MOVEMENT ---------//
	bool GetPathfinding();
	bool landcheck = false;
	bool flycheck = false;
	void ReturnToStart(float dt);

	//--------INTERNAL CONTRO� ---------//
	float pathTimer;
	float timer = 0.f;

	//--------STATE ---------//
	//determines what the enemy is doing
	void UpdateState();
	flying_state enemy_state;
	p2DynArray<iPoint> path;

	//--------ANIMATIONS---------//
	Animation animIdle;
	SDL_Rect enemyRect = { 0,0,0,0 };

	//------COLLIDER------//
	//the entity's sprite is too little compared to the player so we will double it's size.
	int scalesize = 2;

};


#endif // !