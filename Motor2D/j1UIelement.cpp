#include "j1UIelement.h"
#include "j1App.h"
#include "j1Window.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Input.h"
#include "j1Map.h"
#include "j1Scene.h"
#include "j1UIManager.h"
#include "j1Collision.h"
#include "j1EntityManager.h"

j1UIelement::j1UIelement()
{
}

j1UIelement::~j1UIelement()
{
}

bool j1UIelement::Start()
{
	text = App->ui->GetAtlas();
	return true;
}

void j1UIelement::Draw(bool debug)
{
	if (hovering)
	{
		SDL_SetTextureColorMod(text, 200, 200, 200);
		SDL_SetTextureAlphaMod(text, 255);
	}
	else
	{
		SDL_SetTextureColorMod(text, 255, 255, 255);
		SDL_SetTextureAlphaMod(text, 255);
	}

	App->render->Blit(text, Position.x, Position.y, &rect, 0.0f);

	if (debug)
	{
		App->render->DrawQuad({ Position.x, Position.y, rect.w, rect.h }, 0, 255, 255, 255, false, false);
	}


}

bool j1UIelement::OnHover()
{	
	
	bool ret = false;
	SDL_Point mouse;
	App->input->GetMousePosition(mouse.x, mouse.y);
	SDL_Rect intersect = { Position.x, Position.y, rect.w, rect.h};

	if (SDL_PointInRect(&mouse, &intersect) && this->enabled && this->interact)
		ret = true;
	
	return ret;	
}

void j1UIelement::OnClick()
{

	
}

void j1UIelement::OnRelease()
{

}

void j1UIelement::Move(float dt, iPoint MousePos)
{
	if (parent != nullptr)
	{
		Position.x = parent->Position.x - PostoParent.x;
		Position.y = parent->Position.y - PostoParent.x;
	}
	else
	{
		Position.x += ((MousePos.x - Position.x) - MovePoint.x);
		Position.y += ((MousePos.y - Position.y) - MovePoint.y);


	}

}




