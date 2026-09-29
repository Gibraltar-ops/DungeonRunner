#pragma once

#include "SDL3/SDL_render.h" // SDL_Texture används av Sprite.
#include "entity.h" // ID, Entity och Direction avgör vilken sprite som ska hämtas.
#include <cstdint>

struct Sprite
{
	SDL_Texture* texture; // SDL-texturen som ska ritas.
	int width; // Texturens bredd i originalpixlar.
	int height; // Texturens höjd i originalpixlar.
	int pivot_x; // Horisontell ritningspunkt relativt spriten.
	int pivot_y; // Vertikal ritningspunkt relativt spriten.
	int sprite_count_x; 
	int sprite_count_y;
	int framerate;
};

// Constructor samt refererar en sprite med pointer samt frame för 1D till 2D algoritm
struct SpriteRenderInfo
{
	Sprite* sprite;
	int frame;
	bool flipped_x;

	// default constructor, inga parametrar accepterade
	SpriteRenderInfo()
	{
		this->sprite = nullptr;
		this->frame = 0;
		this->flipped_x = false;
	}

	// Kör igenom båda parametrar och assignar dom
	SpriteRenderInfo(int frame, Sprite* sprite)
	{
		this->frame = frame;
		this->sprite = sprite;
		this->flipped_x = false;
	}

	SpriteRenderInfo(int frame, Sprite* sprite, bool flipped_x)
	{
		this->frame = frame;
		this->sprite = sprite;
		this->flipped_x = flipped_x;
	}

	// Kör igenom en sprite så vi kan sätta en sprite perimeter som substitut för en hel SpriteRenderInfo
	SpriteRenderInfo(Sprite* sprite)
	{
		this->sprite = sprite;
		this->frame = 0;
		this->flipped_x = false;
	}
};

enum class SPRITE_ID
{
	Fallback,
	Rock,
	Demon,
	Demon_Run,
	Medusa_Rotate,
	Medusa_Idle_Left,
	Medusa_Idle_Front,
	Medusa_Idle_Back,
	Golem,
	Siren,
	Dropshadow,
	titlescreen_background,
	black_1x1,
	dungeon_tileset, // skall tas bort när jag löst main_tileset
	main_tileset,
	entities_tileset,
	selection_marker,
	Goal,


	Menu_Horizon,
	Menu_Cloud_Back,
	Menu_Cloud_Front,
	Menu_Middle,
	Menu_Front, 

	Button_Basic,

	Enemy,
	Enemy_Run,
};

Sprite* GetSprite(SPRITE_ID sprite_id, Sprite* spriteBuffer);
SpriteRenderInfo GetSprite_FromEntityState(Entity* entity, Sprite* spriteBuffer, const uint64_t* ticks_total); // Hämtar sprite utifrån tillstånd och riktning.


const int NOT_SET= -1; // Markör för automatiskt beräknad pivotpunkt.

struct SpriteDataEntry
{
	SPRITE_ID id; // Positionen i spritebufferten.
	const char* path; // Relativ filsökväg till bildresursen.
	int pivot_x = NOT_SET; // Valfri horisontell pivotpunkt.
	int pivot_y = NOT_SET; // Valfri vertikal pivotpunkt.
	int tileset_cell_count_x = NOT_SET;
	int tileset_cell_count_y = NOT_SET;
	int framerate = NOT_SET;
};


Sprite* GetSpriteFromID(ENTITY_ID id, Sprite* spriteBuffer); // Hämtar den vanliga spriten för ett objekt-ID.


namespace AssetManagement
{
	void LoadSprite(Sprite* spriteBuffer, SpriteDataEntry entry, SDL_Renderer* renderer); // Läser en bild till spritebufferten.
	void LoadAllSprites(Sprite* spriteBuffer, SDL_Renderer* renderer); // Läser in alla sprite-resurser.
}

inline int GetSpriteCount(Sprite* sprite)
{
	if(sprite->sprite_count_x == NOT_SET) return 1;
	if(sprite->sprite_count_y == NOT_SET) return 1;
	return sprite->sprite_count_x * sprite->sprite_count_y;
}
