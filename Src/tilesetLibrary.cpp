#include <cassert>
#include <exception>
#include <fstream>
#include "SDL3/SDL_log.h"
#include "Parsers/json.hpp"
#include "tilesetLibrary.h"
#include "arena.h"
#include "fstream"

using namespace nlohmann;
using namespace std;

// Sätter upp static array för TilesetDataEntry 
static const TilesetDataEntry all_tilesets_data[]
{
	{TILESETS::Main, "assets/tilesets/main_tileset.tsj"},

	{TILESETS::Entities, "assets/tilesets/entities_tileset.tsj"} 
};

// tar ett cell id som parameter och sedan hittar högsta firstgid som är lägre än specificerat ID. 
uint16_t Get_Tileset_ID_Offset_From_Tilemap(int id_limit, const json& tmj_result)
{
	int highest_tilemap_start_id = 0;
	for (const json& tileset : tmj_result["tilesets"])
	{
		int first_id = tileset["firstgid"].get<int>(); // "firstgid" = IDt på första tilen i specifikt tileset, men baserat på ev. tilesets som kommit före

		if (first_id <= id_limit && first_id > highest_tilemap_start_id)
		{
			highest_tilemap_start_id = first_id;
		}
	}

	return highest_tilemap_start_id;

}

uint16_t GetLocalTileID(uint16_t id_global, const json& tmj_result)
{
	return id_global - Get_Tileset_ID_Offset_From_Tilemap(id_global, tmj_result);
}

namespace AssetManagement
{
	// Loopar över TileDataEntry arrayen och kallar LoadTileset för varje punkt i arrayen
	void LoadAllTilesets(Tileset* tilesetBuffer, Memory::Arena* arena_images)
	{
		for (TilesetDataEntry entry : all_tilesets_data)
		{
			LoadTileset(&entry, tilesetBuffer, arena_images);
		}
	}

	// Försöker hitta filen på entry->path och använder json parser för att byta från text till något kodbart 
	void LoadTileset(TilesetDataEntry* entry, Tileset* tilesetBuffer, Memory::Arena* arena_images)
	{
		assert(entry->type != TILESETS::COUNT);
		assert(entry->type != TILESETS::NONE);

		Tileset* tileset = &tilesetBuffer[(int)entry->type];
		tileset->type = entry->type;
		fstream stream(entry->path);

		// SÄKERHETSSPÄRR: Förhindrar att en tom ström skickas till parsern
		if (!stream.is_open())
		{
			SDL_Log("CRITICAL ERROR: Kunde inte oppna tileset-filen: %s", entry->path);
			return;
		}

		// FELHANTERING: Skyddar mot korrupt eller felformaterad JSON-struktur
		try
		{
			auto jsonResult = json::parse(stream);
			int tile_count = jsonResult["tilecount"].get<int>(); 
			tileset->walkableBuffer = ALLOC_ARRAY(arena_images, bool, tile_count); 

			auto& tiles = jsonResult["tiles"];

			for(const auto& tile : tiles)
			{
				int tile_id = tile ["id"].get<int>(); 

				for(const auto& tile_property : tile["properties"]) 
				{
					if(tile_property["name"] == "walkable")
					{
						tileset->walkableBuffer[tile_id] = tile_property["value"].get<bool>();
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			SDL_Log("JSON ERROR i LoadTileset (%s): %s", entry->path, e.what());
		}
	}
}