#pragma once

#include "Parsers/json.hpp"
#include <cstdint>

using namespace nlohmann; // Specificerat så vi slipper skriva nlohmann:: för att få tillgång till jsonstrukten

namespace Memory
{
	struct Arena;
}

// Här lagrar vi olika tilesets 
enum class TILESETS
{
	NONE = 0, 
	Main = 1,
	Entities = 2,
	COUNT = 3
};

// Enkel bool check för att kolla att IDt för tilen är walkable eller ej 
struct Tileset
{
	TILESETS type;
	bool* walkableBuffer;
};

struct TilesetDataEntry
{
	TILESETS type;
	const char* path;
};

// Används för att få tiles ID/Position inuti sitt eget tileset
// Gör att vi inte behöver oroa oss för att vår .tmj kan ha flera tilesets som används
uint16_t GetLocalTileID(uint16_t id_global, const json& tmj_result); 
uint16_t Get_Tileset_ID_Offset_From_Tilemap(int id_limit, const json& tmj_result);

namespace AssetManagement
{
	void LoadAllTilesets(Tileset* tilesetBuffer, Memory::Arena* arena_images); // Forward deklarerar arena struct så att denna fil kan använda den som parameter till funktionerna
	void LoadTileset(TilesetDataEntry* entry, Tileset* tilesetBuffer, Memory::Arena* arena_images);
}