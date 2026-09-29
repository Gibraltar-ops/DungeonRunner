#pragma once

#include <cstdint>
#include "arena.h" // Arena används för nivåns och entiteternas minnesallokering.
#include "entity.h" // Entity, ID och Direction används i nivådata och nivåfunktioner.
#include "tilesetLibrary.h" // uint8_t används för kompakta cell-ID:n. **EDIT: ÄNDRAT TILL UINT16_T
#include "Parsers/json.hpp"

using namespace Memory; // Gör Arena direkt tillgänglig i denna header.

struct Goal
{
	int x;
	int y;
	float blink_timer;
};

struct LevelData
{
	int w;                 // Nivåns bredd i celler.
	int h;                 // Nivåns höjd i celler.
	uint16_t* cells;        // En celltyp per ruta i nivåns bakgrundslager.
	Goal* goals;
	int goalCount;
	const char* level_path;// Sökväg till TMJ-filen, används när entiteter läses in.
	Entity* entityBuffer;  // Dynamisk lista med nivåns entiteter.
	int entityCount;       // Antal använda entitetsplatser i bufferten.
	const Tileset* tileset; // Const pga. Vi vill inte justera det när vi skickar med till LevelData

};


namespace AssetManagement
{
	std::vector<uint16_t> GetCellDataFromJsonLayer(nlohmann::json& parsedJson, const char* layerName, bool* wasFound);

	int GetFirstNonZeroCell(std::vector<uint16_t>* list);
}


void CreateLevel(Arena* arena, LevelData* level, Tileset* tileset, const char* level_name); // Läser nivåns cellager från en TMJ-fil.
void CreateEntities(LevelData* lvl_data, Arena* arena); // Läser entitetslagret och bygger dess entitetsbuffert.
void AddEntity(ENTITY_ID entity_id, int x, int y, LevelData* level); // Skapar eller ersätter en entitet på en cell.
void RemoveEntity(int x, int y, LevelData* level); // Tar bort entiteten på en cell om en finns där.

Entity* GetNextAvailableEntitySlot(Entity* entityBuffer); // Avsedd hjälpfunktion för att hitta en ledig plats i en entitetsbuffert.
Entity* GetEntity(LevelData* level, int x, int y); // Hittar entiteten på en given cell.
Entity* RaycastFirstEntity(int x_origin, int y_origin, Direction direction, LevelData* level, bool ignore_walls = false); // Hittar första entiteten i en riktning.
uint16_t GetCellID(LevelData* level, int x, int y); // Returnerar celltypen på koordinaten.

bool IsWalkable(int x, int y, LevelData* level); // Enkel boolcheck för att se om en tile går att gå på
