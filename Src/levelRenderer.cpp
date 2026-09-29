#include <cstdint> // uint8_t används för cell-ID:n.
#include <cmath> // sinf används för lerp/hoppanimationen.
#include <algorithm> // För att få med STD::sort
#include "levelRenderer.h" // Deklarationerna för nivå- och entitetsritning.
#include "common.h" // CELL_SIZE_PX används för nivåns pixelmått.
#include "rendering.h" // Hjälpfunktioner som faktiskt ritar sprites.
#include "entity.h" // Entitetstillstånd avgör animation och sprite.
#include "camera.h" // Kameran skickas vidare till ritningsfunktionerna.
#include "spriteLibrary.h" // Sprite-ID:n och spriteuppslag används här.
#include "levels.h" // LevelData innehåller celler och entiteter som ritas.
#include "tilesetLibrary.h"

// Ritar alla bakgrundsceller för den aktuella nivån.
void RenderLevel(GameData* gameData, SDL_Renderer* renderer)
{
	Gameplay* gameplay = &gameData->scenes.gameplay;
	LevelData* level = &gameplay->levels[gameplay->currentLevelIndex]; 

	Sprite* sprite;

	// Hämtar rätt tileset-sprite baserat på nivåns tileset-typ
	switch(level->tileset->type)
	{
	case TILESETS::Main:
		sprite = GetSprite(SPRITE_ID::main_tileset, gameData->spriteBuffer);
		break;

		case TILESETS::Entities:      
        sprite = GetSprite(SPRITE_ID::main_tileset, gameData->spriteBuffer); 
        break;

	case TILESETS::NONE:
	case TILESETS::COUNT:
		assert(false);
		break;
	}

	for(int x = 0; x < level->w; x++)
	{
		for (int y = 0 ; y < level->h; y++) 
		{
			// uint16_t istället för uint8_t, och vi skickar med level-pekaren
			uint16_t id = GetCellID(level, x, y); 
			
			//Renderar från rendersprite istället för enskilda sprites
			RenderTile(sprite, id, level, renderer, &gameData->camera, x, y, 1, 1);
		}
	}

	for(int i = 0; i < level->goalCount; i++)
	{
		Goal goal = level->goals[i];
		Sprite* sprite = GetSprite(SPRITE_ID::Goal, gameData->spriteBuffer);
		int frame = (int)(goal.blink_timer / 0.2) % (sprite->sprite_count_x * sprite->sprite_count_y);
		RenderSprite_OnTile({frame,sprite}, level, renderer, &gameData->camera, goal.x, goal.y);
	}
}

bool IsEntityBelowOtherEntity(Entity* a, Entity* b) // Kollar om entities y-värde efter sortering är mindre än ett annat y-värde för att måla upp entitien med lägst värde främst i rendereren
{
	return a->y < b->y;
}

// Ritar samtliga aktiva entiteter med interpolerad rörelse och eventuellt hopp.
void RenderEntities(GameData* data, SDL_Renderer* renderer)
{
	LevelData* lvl = &data->scenes.gameplay.levels[data->scenes.gameplay.currentLevelIndex];

	Entity** SortedEntities = ALLOC_ARRAY(data->arena_scratch, Entity*, lvl->entityCount);

	for (int i = 0; i < lvl->entityCount; i++)
	{
		SortedEntities[i] = &lvl->entityBuffer[i];
	}

	std::sort(SortedEntities, SortedEntities + lvl->entityCount, IsEntityBelowOtherEntity);

	Gameplay* gameplay = &data->scenes.gameplay;

	Entity* activeEntity = gameplay->activePlayerBuffer[gameplay->activePlayerIndex];

	for(int i = 0; i < lvl->entityCount; i++)
	{
		Entity* entity = SortedEntities[i]; // Lokal kopia av aktuell entitet för beräkning och ritning.

		if(entity->active == false)
		{
			continue;
		}

		SpriteRenderInfo sprite = GetSprite_FromEntityState(entity, data->spriteBuffer, data->ticks_total);

		if(HasBehaviour(entity, Behaviour::IS_PETRIFIED))
		{
			sprite = GetSprite(SPRITE_ID::Rock, data->spriteBuffer);
		}

		float x_animated = std::lerp(entity->x_prev, entity->x, entity->progress_01); // Interpolerad x-position för mjuk rörelse.
		float y_animated = std::lerp(entity->y_prev, entity->y, entity->progress_01); // Interpolerad y-position för mjuk rörelse.

		float ground_y = y_animated;

		if(entity->action == Actions::MOVING && HasBehaviour(entity, Behaviour::JUMPS) && !HasBehaviour(entity, Behaviour::IS_PUSHING))
		{
			y_animated -= 0.5 * sinf(entity->progress_01 * 3.14);
		}

		Sprite* dropshadow = &data->spriteBuffer[(int)SPRITE_ID::Dropshadow];

		RenderSprite_OnTile(dropshadow, lvl, renderer, &data->camera, x_animated, ground_y, 1, 0.4, false);

		if(entity == activeEntity)
		{
			SpriteRenderInfo selection_marker = GetSprite(SPRITE_ID::selection_marker, data->spriteBuffer);
			RenderSprite_OnTile(selection_marker, lvl, renderer, &data->camera, x_animated, ground_y);
		}

		RenderSprite_OnTile(sprite, lvl, renderer, &data->camera, x_animated, y_animated, 1, 1, false);

		}

	}
