#include <cstdint> // uint8_t används för cell- och entitetsdata från JSON.
#include <cstdio>
#include <fstream> // fstream läser nivåfilerna från disk.
#include <vector> // JSON-lagren läses in som vector<uint8_t>.
#include <exception> // För att kunna try/catch om levels ej laddar korrekt 
#include "SDL3/SDL_log.h"
#include "levels.h" // Nivåstrukturerna och funktionerna som implementeras här.
#include "arena.h" // Minnesarenan används för nivå- och entitetsbuffertar.
#include "Parsers/json.hpp" // nlohmann::json används för att tolka TMJ-filer.
#include "common.h"
#include "entity.h"                    
#include "tilesetLibrary.h" // Entiteter skapas från nivåns entitetslager.

using namespace std;
using namespace nlohmann;

const int LEVEL_INDEX = 0; // Index för TMJ-filens mark-/cellager.
const int ENTITIES_INDEX = 1; // Index för TMJ-filens entitetslager.

namespace AssetManagement
{
	std::vector<uint16_t> GetCellDataFromJsonLayer(nlohmann::json &parsedJson, const char *layerName, bool *wasFound)
	{
		std:vector<uint16_t> result;
		*wasFound = false;
		for (const auto& layer : parsedJson["layers"])
		{
			if (layer["name"] == layerName)
			{
				result = layer["data"].get<vector<uint16_t>>();
				*wasFound = true;
				break;
			}
		}

		return result;

	}

	int GetFirstNonZeroCell(std::vector<uint16_t> *list)
	{
		for (int id : *list)
		{
			if(id != 0)
			{
				return id;
			}
		}

		assert(false);
		return -1;
	}
}

// Hjälpfunktion från levels.h för att kolla om tile är walkable
bool IsWalkable(int x, int y, LevelData* level)
{
	uint16_t id = GetCellID(level, x, y);
	return level->tileset->walkableBuffer[id];
}

// Läser nivåns dimensioner och cellager från en TMJ-fil till den angivna arenan.
void CreateLevel(Arena* arena, LevelData* level, Tileset* tileset, const char* level_name)
{
	fstream stream(level_name); // Filströmmen som läser den angivna nivåfilen.

	if (!stream.is_open())
	{
		SDL_Log("Couldn't open file %s", level_name);
		return;
	}

	
	auto result = json::parse(stream);

	bool found = false;
	vector<uint16_t> levelData = AssetManagement::GetCellDataFromJsonLayer(result, "level", &found);

	assert(found);

	int first_non_zero_id = AssetManagement::GetFirstNonZeroCell(&levelData);
	int id_offset = Get_Tileset_ID_Offset_From_Tilemap(first_non_zero_id, result);

	level->w = result["width"].get<int>();
	level->h = result["height"].get<int>();
	level->level_path = level_name;
	level->tileset = tileset;
	level->cells = ALLOC_ARRAY(arena, uint16_t, level->w * level->h);

	for (int i = 0; i < level->w * level->h; i++) 
	{
	int local_id = levelData[i] - id_offset;

		if(local_id < 0)
		{
			local_id = 0;
		}

	level->cells[i] = local_id;

	}

	vector<uint16_t> onLevel = AssetManagement::GetCellDataFromJsonLayer(result, "on_level", &found);

	if(found)
	{
		int firstNonZero = AssetManagement::GetFirstNonZeroCell(&onLevel);
		id_offset = Get_Tileset_ID_Offset_From_Tilemap(firstNonZero, result);
		level->goalCount = 0;

		for (int i = 0; i < level->w * level->h; i++)
		{
			int local_id = onLevel[i] - id_offset + 1;
			if(local_id > 0)
			{
				level->goalCount++;
			}
		}

		level->goals = ALLOC_ARRAY(arena, Goal, level->goalCount);
		int index = 0;
		for (int i = 0; i < level->w * level->h; i++)
		{
			int local_id = onLevel[i] - id_offset + 1; 
			if(local_id < 0)
			{
				local_id = 0;
			}

			if(local_id != 0)
			{
				int x;
				int y;
				Expand1DTo2D(i, level->w, &x, &y);
				level->goals[index].x = x;
				level->goals[index].y = y;
				index++;
			}
		}


	}
}


// Returnerar cell-ID:t med koordinaterna omräknade till buffertens radindex.
uint16_t GetCellID(LevelData* level, int x, int y)
{
	return level->cells [y * level->w + x];
}

// Söker linjärt efter en entitet som står på den angivna cellen.
Entity* GetEntity(LevelData* level, int x, int y)
{
	for (int i = 0; i < level->entityCount; i++)
	{
		if (level->entityBuffer[i].active && level->entityBuffer[i].x == x && level->entityBuffer[i].y == y)
		{
			return &level->entityBuffer[i];
		}
	}

return nullptr;

}

// Återanvänder en tom entitetsplats eller utökar den använda delen av bufferten.
Entity* GetNextAvailableEntity(LevelData* level)
{
	for (int i = 0; i < level->entityCount; i++)
	{
		if(!level->entityBuffer[i].active)
		{
			return &level->entityBuffer[i];
		}
	}
	return &level->entityBuffer[level->entityCount++];

}

// Skapar eller ersätter en entitet på koordinaten och initierar dess starttillstånd.
void AddEntity(ENTITY_ID entity_id, int x, int y, LevelData* level)
{
	Entity* entity = GetEntity(level, x, y); // Befintlig entitet som kan ersättas på cellen.

	if (entity == nullptr)
	{
		entity = GetNextAvailableEntity(level);
	}

	entity->active = true;
	entity->x = x;
	entity->y = y;
	entity->x_prev = x;
	entity->y_prev = y;
	entity->id = entity_id;
	entity->action = Actions::NONE;
	InitializeBaseBehaviour(entity);
}

// Läser entitetslagret från TMJ-filen och bygger om den aktiva entitetsbufferten.
void CreateEntities(LevelData* lvl_data, Arena* arena)
{
	Reset(arena);
	lvl_data->entityCount = 0;
	lvl_data->entityBuffer = (Entity*)Memory::Allocate(arena, sizeof(Entity) * 256);

	fstream stream(lvl_data->level_path); // Hämtar JSON filen som lagrades under CreateLevel()

	auto result = json::parse(stream);

	bool found = false;
	vector<uint16_t> entities = AssetManagement::GetCellDataFromJsonLayer(result, "entities", &found);


	if (!found)
	{
		return;
	}

	for (int i = 0; i < lvl_data->w * lvl_data->h; i++)
	{
		if (entities[i] == 0 )
		{
			continue;
		}

		uint16_t entity_id = GetLocalTileID(entities[i], result);
		int x;
		int y;
		Expand1DTo2D(i, lvl_data->w, &x, &y);
		AddEntity((ENTITY_ID)entity_id, x, y, lvl_data);
	}

}
		


// Följer celler i en riktning tills en vägg eller första entitet hittas.
Entity* RaycastFirstEntity(int x_origin, int y_origin, Direction direction, LevelData* level, bool ignore_walls)
{
	Position facingVector; // Stegvektor som motsvarar den efterfrågade riktningen.

	switch (direction)
	{
	case Direction::RIGHT:
		facingVector = {1, 0};
		break;

	case Direction::LEFT:
		facingVector = {-1, 0};
		break;

	case Direction::UP:
		facingVector = {0, 1};
		break;
		
	case Direction::DOWN:
		facingVector = {0, -1};
		break;	
	}

	int x_search = x_origin + facingVector.x; // Första cellen som checkas horisontellt.
	int y_search = y_origin + facingVector.y; // Första cellen som checkas vertikalt.

	while(x_search > 0 && x_search < level->w && y_search > 0 && y_search < level->h)
	{
		if(!IsWalkable(x_search, y_search, level) && !ignore_walls)
		{
			break;
		}

		Entity* entity_search = GetEntity(level, x_search, y_search);
		if(entity_search != nullptr)
		{
			return entity_search;
		}

		x_search += facingVector.x;
		y_search += facingVector.y;
	}

	return nullptr;

}

// Nollställer entiteten på cellen så att platsen kan återanvändas.
void RemoveEntity(int x, int y, LevelData* level)
{
	Entity* entity = GetEntity(level, x, y);
	if (entity == nullptr)
	{
		return;
	}

	*entity = {};

}
