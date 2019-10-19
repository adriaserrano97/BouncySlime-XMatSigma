#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Map.h"
#include "j1Collision.h"
#include "j1Player.h"
#include "j1Window.h"
#include <math.h>

j1Map::j1Map() : j1Module(), map_loaded(false)
{
	name.create("map");
}

// Destructor
j1Map::~j1Map()
{}

// Called before render is available
bool j1Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder.create(config.child("folder").child_value());

	return ret;
}

void j1Map::Draw()
{
	if(map_loaded == false)
		return;
	p2List_item<LayerInfo*>* layer = data.layerList.start;

	for (layer; layer != NULL; layer = layer->next)
	for (int y = 0; y < data.height; ++y)
	{
		for (int x = 0; x < data.width; ++x)
		{
			int tile_id = layer->data->Get(x, y);
			if (tile_id > 0)
			{
				TileSet* tileset = GetTilesetFromTileId(tile_id);
				if (tileset != nullptr)
				{
					SDL_Rect r = tileset->GetTileRect(tile_id);
					iPoint pos = MapToWorld(x, y);
					float parallaxSpeed = layer->data->parallaxSpeed;
					if(parallaxSpeed <= 1.0f)
						App->render->Blit(tileset->texture, pos.x, pos.y, &r,parallaxSpeed);
				}
			}
		}
	}

}

iPoint j1Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	ret.x = x * data.tile_width;
	ret.y = y * data.tile_width;

	return ret;
}

TileSet* j1Map::GetTilesetFromTileId(int id) const
{
	p2List_item<TileSet*>* item = data.tilesets.start;
	TileSet* set = item->data;

	while (item)
	{
		if (id < item->data->firstgid)
		{
			set = item->prev->data;
			break;
		}
		set = item->data;
		item = item->next;
	}
	return set;
}


// Called before quitting
bool j1Map::CleanUp()
{
	LOG("Unloading map");

	// Remove all tilesets
	p2List_item<TileSet*>* item;
	item = data.tilesets.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	data.tilesets.clear();

	// Remove all layers
	p2List_item<LayerInfo*>* item2;
	item2 = data.layerList.start;

	while (item2 != NULL)
	{
		RELEASE(item2->data);
		item2 = item2->next;
	}
	data.layerList.clear();

	// Clean up the pugui tree
	map_file.reset();

	return true;
}

// Load new map
bool j1Map::Load(const char* file_name)
{
	bool ret = true;
	p2SString tmp("%s%s", folder.GetString(), file_name);

	pugi::xml_parse_result result = map_file.load_file(tmp.GetString());

	if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
		ret = false;
	}

	// Load general info ----------------------------------------------
	if(ret == true)
	{
		ret = LoadMap();
	}

	// Load all tilesets info ----------------------------------------------
	pugi::xml_node tileset;
	for (tileset = map_file.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if (ret == true)
		{
			ret = LoadTilesetDetails(tileset, set);
		}

		if (ret == true)
		{
			ret = LoadTilesetImage(tileset, set);
		}

		data.tilesets.add(set);
	}

	// Load layer info ----------------------------------------------
	for (pugi::xml_node nodeLayer = map_file.child("map").child("layer"); nodeLayer; nodeLayer = nodeLayer.next_sibling("layer")) 
	{
		LayerInfo* layerInfo = new LayerInfo();

		load_Layer(nodeLayer,layerInfo);

	}

	pugi::xml_node layer;
	for (layer = map_file.child("map").child("layer"); layer && ret; layer = layer.next_sibling("layer"))
	{
		LayerInfo* layerInfo = new LayerInfo();

		ret = load_Layer(layer, layerInfo);

		if (ret == true)
			data.layerList.add(layerInfo);
	}
	// Load colliders info ----------------------------------------------
	for (pugi::xml_node nodeColliderGroup = map_file.child("map").child("objectgroup"); nodeColliderGroup; nodeColliderGroup = nodeColliderGroup.next_sibling("objectgroup"))
	{
		for (pugi::xml_node nodeCollider = nodeColliderGroup.child("object"); nodeCollider; nodeCollider = nodeCollider.next_sibling("object"))
		{
			load_collider(nodeCollider);
		}

	}

	if(ret == true)
	{
		LOG("Successfully parsed map XML file: %s", file_name);
		LOG("width: %d height: %d", data.width, data.height);
		LOG("tile_width: %d tile_height: %d", data.tile_width, data.tile_height);

		p2List_item<TileSet*>* item = data.tilesets.start;
		while(item != NULL)
		{
			TileSet* s = item->data;
			LOG("Tileset ----");
			LOG("name: %s firstgid: %d", s->name.GetString(), s->firstgid);
			LOG("tile width: %d tile height: %d", s->tile_width, s->tile_height);
			LOG("spacing: %d margin: %d", s->spacing, s->margin);
			item = item->next;
		}
		
		p2List_item<LayerInfo*>* item_layer = data.layerList.start;
		while(item_layer != NULL)
		{
			LayerInfo* l = item_layer->data;
			LOG("Layer ----");
			LOG("name: %s", l->name.GetString());
			LOG("tile width: %d tile height: %d", l->width, l->height);
			item_layer = item_layer->next;
		}
	}

	map_loaded = ret;

	return ret;
}

// Load map general properties
bool j1Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = map_file.child("map");

	if(map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		data.width = map.attribute("width").as_int();
		data.height = map.attribute("height").as_int();
		data.tile_width = map.attribute("tilewidth").as_int();
		data.tile_height = map.attribute("tileheight").as_int();
		p2SString bg_color(map.attribute("backgroundcolor").as_string());

		data.background_color.r = 0;
		data.background_color.g = 0;
		data.background_color.b = 0;
		data.background_color.a = 0;

		if(bg_color.Length() > 0)
		{
			p2SString red, green, blue;
			bg_color.SubString(1, 2, red);
			bg_color.SubString(3, 4, green);
			bg_color.SubString(5, 6, blue);

			int v = 0;

			sscanf_s(red.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.r = v;

			sscanf_s(green.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.g = v;

			sscanf_s(blue.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.b = v;
		}

		p2SString orientation(map.attribute("orientation").as_string());

		if(orientation == "orthogonal")
		{
			data.type = MAPTYPE_ORTHOGONAL;
		}
		else if(orientation == "isometric")
		{
			data.type = MAPTYPE_ISOMETRIC;
		}
		else if(orientation == "staggered")
		{
			data.type = MAPTYPE_STAGGERED;
		}
		else
		{
			data.type = MAPTYPE_UNKNOWN;
		}
	}

	return ret;
}

bool j1Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	set->name.create(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if(offset != NULL)
	{
		set->offset_x = offset.attribute("x").as_int();
		set->offset_y = offset.attribute("y").as_int();
	}
	else
	{
		set->offset_x = 0;
		set->offset_y = 0;
	}

	return ret;
}

bool j1Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if(image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		set->texture = App->tex->Load(PATH(folder.GetString(), image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);
		set->tex_width = image.attribute("width").as_int();

		if(set->tex_width <= 0)
		{
			set->tex_width = w;
		}

		set->tex_height = image.attribute("height").as_int();

		if(set->tex_height <= 0)
		{
			set->tex_height = h;
		}

		set->num_tiles_width = set->tex_width / set->tile_width;
		set->num_tiles_height = set->tex_height / set->tile_height;
	}

	return ret;
}


bool j1Map::load_Layer(pugi::xml_node& node, LayerInfo* layerInfo)
{
	bool ret = true;

	layerInfo->name = node.attribute("name").as_string();
	layerInfo->width = node.attribute("width").as_uint();
	layerInfo->height = node.attribute("height").as_uint();
	layerInfo->parallaxSpeed = node.child("properties").child("property").attribute("value").as_float(0.0f);

	//LoadProperties(node, layerInfo->properties);
	pugi::xml_node layer_data = node.child("data");


	if (layer_data == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'layer/data' tag.");
		ret = false;
		RELEASE(layerInfo);
	}
	else
	{
		layerInfo->tileArray = new uint[layerInfo->width * layerInfo->height];

		memset(layerInfo->tileArray, 0, layerInfo->width * layerInfo->height);

		uint i = 0;
		for (pugi::xml_node tile = node.child("data").child("tile"); tile; tile = tile.next_sibling("tile")) {
			layerInfo->tileArray[i] = tile.attribute("gid").as_uint(0);
			++i;
		}

	}

	return ret;
}

bool j1Map::load_collider(pugi::xml_node& node)
{
	bool ret = true;

	SDL_Rect colliderRect;

	colliderRect.x = node.attribute("x").as_int();
	colliderRect.y = node.attribute("y").as_int();
	colliderRect.w = node.attribute("width").as_int();
	colliderRect.h = node.attribute("height").as_int();

	COLLIDER_TYPE type;
	p2SString name = node.attribute("name").as_string();

	if (name == "Floor")
		type = COLLIDER_WALL;
	else if (name == "Start") 
	{
		type = COLLIDER_START;
		App->player->SetPos(colliderRect.x, colliderRect.y);
	}
	else
		type = COLLIDER_NONE;
		
	if (type == COLLIDER_NONE)
	{
		LOG("Error loading collider. Type not especified");
		ret = false;
	}
	else if (type != COLLIDER_START)
	{
		Collider* collision  = new Collider(colliderRect, type, this);
		App->collision->AddControlCollider(collision);
		data.colliderList.add(collision);
	}

	return ret;
}

SDL_Rect TileSet::GetTileRect(uint tileId) {

	SDL_Rect toReturn;
	
	int relative_id = tileId - firstgid;

	toReturn.w = tile_width;
	toReturn.h = tile_height;

	toReturn.x = margin + ((toReturn.w + spacing) * (relative_id % num_tiles_width));
	toReturn.y = margin + ((toReturn.h + spacing) * (relative_id / num_tiles_width));



	return(toReturn);
};