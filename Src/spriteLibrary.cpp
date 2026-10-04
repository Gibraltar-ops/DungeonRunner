#include <cassert> // assert stoppar körningen om en bildresurs inte kan laddas.
#include <cmath> // För att känna av lerp 
#include <cstdint>
#include "spriteLibrary.h" // Sprite-typer och laddningsfunktioner implementeras här.
#include "SDL3_Image/SDL_image.h" // IMG_Load läser bildfiler till SDL_Surface.
#include "entity.h" // Entitetens ID, riktning och beteenden avgör vald sprite.
#include "common.h"

using namespace std;

const char* FALLBACK_PATH = "assets/Sprites/fallback.png"; // Reservbild när en spritefil saknas.

static const SpriteDataEntry all_sprite_data[] = // Tabell som kopplar sprite-ID:n till deras bildfiler och pivoter.
{
	{SPRITE_ID::Fallback, FALLBACK_PATH, 8, 8},
	{.id = SPRITE_ID::Demon, 
	.path = "assets/Sprites/demon_idle.png",
	.pivot_x = 12,
	.pivot_y = 24,
	.tileset_cell_count_x = 4,
	.tileset_cell_count_y = 1,
	.framerate = 8 },
	{.id = SPRITE_ID::Demon_Run, 
	.path = "assets/Sprites/demon_run.png",
	.pivot_x = 12,
	.pivot_y = 24,
	.tileset_cell_count_x = 4,
	.tileset_cell_count_y = 1,
	.framerate = 8 },
	{SPRITE_ID::Rock, "assets/Sprites/crate.png", 10, 18},
	{SPRITE_ID::Medusa_Rotate, "assets/Sprites/medusa_rotate.png", 12, 24, 8, 1}, // Spritesheet, där av 8, 1 som sista parametrar
	{.id = SPRITE_ID::Medusa_Idle_Left,
	.path = "assets/Sprites/medusa_idle_left.png",
	.pivot_x = 12,
	.pivot_y = 24,
	.tileset_cell_count_x = 4,
	.tileset_cell_count_y = 1,
	.framerate = 8	
	},
	{SPRITE_ID::Medusa_Idle_Front, "assets/Sprites/medusa_idle_front.png", 12, 24, 4, 1, 8},
	{SPRITE_ID::Medusa_Idle_Back, "assets/Sprites/medusa_idle_back.png", 12, 24, 4, 1, 8},
	{SPRITE_ID::Dropshadow, "assets/Sprites/dropshadow.png", 8, 8},
	{SPRITE_ID::black_1x1, "assets/Sprites/black_1x1.png", 0, 0},
	{SPRITE_ID::titlescreen_background, "assets/Sprites/titlescreen.png"},
	{SPRITE_ID::dungeon_tileset, "assets/Sprites/hell_of_a_time_dungeon_tileset.png", 0, 0, 9, 9}, // Skall tas bort när jag löst main_tileset
	{SPRITE_ID::Goal, "assets/Sprites/goal.png", 8, 8, 8, 1},

	{SPRITE_ID::main_tileset, "assets/Sprites/main_tileset.png", 0, 0, 32, 32},
	{SPRITE_ID::entities_tileset, "assets/Sprites/entities_tileset.png", NOT_SET, NOT_SET, 6, 1}, 


	{SPRITE_ID::Menu_Horizon, "assets/Sprites/mainmenu_background.png"},
	{SPRITE_ID::Menu_Cloud_Back, "assets/Sprites/mainmenu_cloud_back.png"},
	{SPRITE_ID::Menu_Cloud_Front, "assets/Sprites/mainmenu_cloud_front.png"},
	{SPRITE_ID::Menu_Middle, "assets/Sprites/mainmenu_middle.png"},
	{SPRITE_ID::Menu_Front, "assets/Sprites/mainmenu_front.png"},

	{SPRITE_ID::Button_Basic, "assets/Sprites/basic_button.png", 0, 0, 3, 3},

	{.id = SPRITE_ID::Golem, 
	 .path = "assets/Sprites/walker_idle.png",  
	 .pivot_x = 12,                           
	 .pivot_y = 24,
	 .tileset_cell_count_x = 4,                
	 .tileset_cell_count_y = 1,
	 .framerate = 8 },

	{.id = SPRITE_ID::Golem_Run, 
	 .path = "assets/Sprites/walker_run.png",   
	 .pivot_x = 12,
	 .pivot_y = 24,
	 .tileset_cell_count_x = 4,
	 .tileset_cell_count_y = 1,
	 .framerate = 8 },
    {SPRITE_ID::Siren, "assets/Sprites/siren.png"},
	
	{.id = SPRITE_ID::Enemy, 
	.path = "assets/Sprites/enemy_idle.png",
	.pivot_x = 12,
	.pivot_y = 24,
	.tileset_cell_count_x = 4,
	.tileset_cell_count_y = 1,
	.framerate = 8 },
	{.id = SPRITE_ID::Enemy_Run, 
	.path = "assets/Sprites/enemy_run.png",
	.pivot_x = 12,
	.pivot_y = 24,
	.tileset_cell_count_x = 4,
	.tileset_cell_count_y = 1,
	.framerate = 8 },

};

// Väljer en sprite som motsvarar entitetens aktuella beteende och riktning.
SpriteRenderInfo GetSprite_FromEntityState(Entity* entity, Sprite* spritebuffer, const uint64_t* ticks_total)
{
	if(HasBehaviour(entity, Behaviour::IS_PETRIFIED))
	{
		return GetSprite(SPRITE_ID::Rock, spritebuffer);
	}

	if(entity->id == ENTITY_ID::MEDUSA && entity->action == Actions::ROTATING)
	{
		Sprite* spritesheet = GetSprite(SPRITE_ID::Medusa_Rotate, spritebuffer);

		int start = 0;
		int end = 0;

		switch(entity->facing_previous)
		{
		case Direction::RIGHT:
			start = 6;
			break;

		case Direction::LEFT:
			start = 2;
			break;

		case Direction::UP:
			start = 4;
			break;

		case Direction::DOWN:
			start = 0;
			break;
		}

		switch(entity->facing_current)
		{
		case Direction::RIGHT:
			end = 6;
			break;

		case Direction::LEFT:
			end = 2; 
			break;

		case Direction::UP:
			end = 4;
			break;

		case Direction::DOWN:
			end = 0;
			break;
		}

	int sprite_count = GetSpriteCount(spritesheet);
	int forward = ((end - start) % sprite_count + sprite_count) % sprite_count;
	int backward = sprite_count - forward;
	end = (forward <= backward) ? (start + forward) : (start - backward);

	int current_frame = ((int)std::lerp(start, end, entity->progress_01) % sprite_count);
	return {current_frame, spritesheet};

	}


	switch (entity->id)
	{
	case ENTITY_ID::MEDUSA:
		{
			Sprite* sprite = nullptr;
			int frame = 0;

			switch (entity->facing_current)
			{
			case Direction::RIGHT:
				sprite = GetSprite(SPRITE_ID::Medusa_Idle_Left, spritebuffer);
				frame = (int)((*ticks_total * 8) / FPS % GetSpriteCount(sprite));
				return {frame, sprite, true};
			

			case Direction::LEFT:
				sprite = GetSprite(SPRITE_ID::Medusa_Idle_Left, spritebuffer);
				frame = (int)((*ticks_total * 8) / FPS % GetSpriteCount(sprite));
				return {frame, sprite};

			case Direction::DOWN:
				sprite = GetSprite(SPRITE_ID::Medusa_Idle_Back, spritebuffer);
				frame = (int)((*ticks_total * 8) / FPS % GetSpriteCount(sprite));
				return {frame, sprite};
				

			case Direction::UP:
				sprite = GetSprite(SPRITE_ID::Medusa_Idle_Front, spritebuffer);
				frame = (int)((*ticks_total * 8) / FPS % GetSpriteCount(sprite));
				return {frame, sprite};
				
			}
		}

	case ENTITY_ID::DEMON:
		{
			Sprite* sprite = nullptr;
			int frame = 0;
			bool flipped = (entity->facing_current == Direction::LEFT);

			if (entity->action == Actions::MOVING)
			{
				sprite = GetSprite(SPRITE_ID::Demon_Run, spritebuffer);
				frame = (int)(entity->progress_01 * GetSpriteCount(sprite)) % GetSpriteCount(sprite);
			}
			else
			{
				sprite = GetSprite(SPRITE_ID::Demon, spritebuffer);
				frame = (int)((*ticks_total * sprite->framerate) / FPS % GetSpriteCount(sprite));
			}

			return SpriteRenderInfo(frame, sprite, flipped);
		}

	case ENTITY_ID::ROCK:
		return GetSprite(SPRITE_ID::Rock, spritebuffer);

	case ENTITY_ID::ENEMY:
		{
			Sprite* sprite = nullptr;
			int frame = 0;
			bool flipped = (entity->facing_current == Direction::LEFT);

			if (entity->action == Actions::MOVING)
			{
				sprite = GetSprite(SPRITE_ID::Enemy_Run, spritebuffer);
				frame = (int)(entity->progress_01 * GetSpriteCount(sprite)) % GetSpriteCount(sprite);
			}
			else
			{
				sprite = GetSprite(SPRITE_ID::Enemy, spritebuffer);
				frame = (int)((*ticks_total * sprite->framerate) / FPS % GetSpriteCount(sprite));
			}

			return SpriteRenderInfo(frame, sprite, flipped);
		}

	case ENTITY_ID::GOLEM:
    {
        Sprite* sprite = nullptr;
        int frame = 0;
        bool flipped = (entity->facing_current == Direction::LEFT);

        if (entity->action == Actions::MOVING)
        {
            sprite = GetSprite(SPRITE_ID::Golem_Run, spritebuffer);
            frame = (int)(entity->progress_01 * GetSpriteCount(sprite)) % GetSpriteCount(sprite);
        }
        else
        {
            sprite = GetSprite(SPRITE_ID::Golem, spritebuffer);
            frame = (int)((*ticks_total * sprite->framerate) / FPS % GetSpriteCount(sprite));
        }

        return SpriteRenderInfo(frame, sprite, flipped);
    }

	case ENTITY_ID::SIREN:
		return GetSprite(SPRITE_ID::Siren, spritebuffer);

	default:
		return GetSprite(SPRITE_ID::Fallback, spritebuffer);

	}

}

Sprite* GetSprite(SPRITE_ID sprite_id, Sprite* spriteBuffer)
{
	return &spriteBuffer[(int)sprite_id];
}

// Hämtar den vanliga spriten för ett objekt-ID och använder fallback vid behov.
Sprite* GetSpriteFromID(ENTITY_ID id, Sprite* spriteBuffer)
{

	Sprite* sprite_to_return = nullptr; // Vald sprite; nullptr innebär att fallback ska användas.

	switch (id)
	{
	
	case ENTITY_ID::DEMON:
		return &spriteBuffer[(int)SPRITE_ID::Demon];

	case ENTITY_ID::ROCK:
		return &spriteBuffer[(int)SPRITE_ID::Rock];

	case ENTITY_ID::MEDUSA:
		sprite_to_return = nullptr;
		break;

	case ENTITY_ID::SIREN:
		return &spriteBuffer[(int)SPRITE_ID::Siren];

	case ENTITY_ID::GOLEM:
		return &spriteBuffer[(int)SPRITE_ID::Golem];
		break;

	case ENTITY_ID::ENEMY:
		return &spriteBuffer[(int)SPRITE_ID::Enemy];

	}

	if (sprite_to_return == nullptr || sprite_to_return->texture == nullptr)
	{
		sprite_to_return = &spriteBuffer[(int)SPRITE_ID::Fallback];
	}

	return sprite_to_return;

}

namespace AssetManagement
{
	// Läser in varje post i spritetabellen till den gemensamma spritebufferten.
	void LoadAllSprites(Sprite* spriteBuffer, SDL_Renderer* renderer)
	{
		for (SpriteDataEntry entry : all_sprite_data)
		{
			LoadSprite (spriteBuffer, entry, renderer);
		}

	}

	// Läser en bildfil, skapar en SDL-textur och sparar dess metadata på rätt buffertindex.
	void LoadSprite(Sprite* spriteBuffer, SpriteDataEntry entry, SDL_Renderer* renderer)
	{
		SDL_Surface* surface = IMG_Load(entry.path); // Tillfällig avkodad bild från resursfilen.


		if (surface == nullptr)
		{
			surface = IMG_Load(FALLBACK_PATH);
		}

		assert(surface != nullptr);

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // GPU-textur skapad av bilden.
		Sprite* sprite = &spriteBuffer[(int)entry.id]; // Målposten i den gemensamma spritebufferten.
		sprite->texture = texture;
		sprite->height = texture->h;
		sprite->width = texture->w;

		sprite->framerate = entry.framerate;

		if(entry.pivot_x == NOT_SET || entry.pivot_y == NOT_SET)
		{
			sprite->pivot_x = sprite->width / 2;
			sprite->pivot_y = sprite->height / 2;
		}
		else
		{
			sprite->pivot_x = entry.pivot_x;
			sprite->pivot_y = entry.pivot_y;
		}

		sprite->sprite_count_x = entry.tileset_cell_count_x;
		sprite->sprite_count_y = entry.tileset_cell_count_y;

		SDL_DestroySurface(surface);
	}
}



