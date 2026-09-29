#include <cstring> // För C-sträng/minneshjälp.
#include <cmath>
#include "game.h" // Exporterade spelfunktioner och TryMove deklareras här.
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_blendmode.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "arena.h"
#include "audioSystem.h"
#include "button.h"
#include "common.h" // Rörelsehastighetheter och Undo.
#include "entity.h" // Entiteternas beteenden, riktning och rörelse används i spellogiken.
#include "fontLibrary.h"
#include "gameState.h"
#include "imgui/imgui.h" // ImGui-kontexten sparas vid initialisering.
#include "input.h" // Tangentstatus används för spelarens handlingar.
#include "image.h" // Gammal include, vågar ej ta bort.
#include "levels.h"
#include "leveleditor.h"
#include "mainmenu.h"
#include "pathfinding.h"
#include "rendering.h"
#include "tilesetLibrary.h"
#include "rendering.h" // Renderingsrelaterade types används av spellogiken.
#include "spriteLibrary.h" // Sprites laddas vid spelstart.
#include "levelRenderer.h" // Tillgång till nivå och entiteter ritas varje bildruta.
#include "command.h" // Här hämtas movement commands.
#include "dev_gui.h" // För att få tillgång till DevGui.
#include "pathfinding.h" // För att komma åt vår A* algoritm för finden



extern "C"
{
void InitializeGame(Gameplay* gameplay, Arena* arena_levels, Tileset* tilesetBuffer)
{
		assert(gameplay->initialized == false);
		gameplay->currentLevelIndex = 0;
		CreateLevel(arena_levels, &gameplay->levels[0], &tilesetBuffer[(int)TILESETS::Main],"assets/levels/level_01.tmj");
		CreateLevel(arena_levels, &gameplay->levels[1], &tilesetBuffer[(int)TILESETS::Main],"assets/levels/level_02.tmj");
    CreateLevel(arena_levels, &gameplay->levels[2], &tilesetBuffer[(int)TILESETS::Main],"assets/levels/level_03.tmj");
		gameplay->initialized = true;
}

	// Initierar utvecklarverktyg, sprites och nivådata efter att DLL:en laddats.

// Initialize-funktionen körs en gång när spelet startar. 
// Den laddar in ljud, bilder, typsnitt och banor i datorns minne så att de kan användas snabbt.
void Initialize(GameData* data, SDL_Window* window, SDL_Renderer* renderer)
{
		*data->ticks_total = 0;

    SDL_Log("DEV::Initialize");
    DEV::Initialize(window, renderer);

    SDL_Log("Initialize Audio");
    InitializeAudioSystem(&data->audio, data->arena_main);

    SDL_Log("Loading SFX");
    AssetManagement::LoadAllSFX(&data->audio);
    
    SDL_Log("Loading Sprites");
    AssetManagement::LoadAllSprites(data->spriteBuffer, renderer);
    
    SDL_Log("ImGui Context setup");
    data->imGui_context = ImGui::GetCurrentContext();

    SDL_Log("Loading font");
    AssetManagement::LoadFont(renderer, "assets/fonts/ByteBounce.ttf", &data->font, 48);
    
    SDL_Log("Loading tilesets");
    AssetManagement::LoadAllTilesets(data->tilesetBuffer, data->arena_images); 
    
    SDL_Log("Handling screenfade");
    SDL_Texture* blackfade = GetSprite(SPRITE_ID::black_1x1, data->spriteBuffer)->texture;
    SDL_SetTextureBlendMode(blackfade, SDL_BLENDMODE_BLEND);
    
    SDL_Log("Initiating game");
    InitializeGame(&data->scenes.gameplay, data->arena_levels, data->tilesetBuffer);

    SDL_Log("Initiating Menu");
    InitializeMenu(&data->scenes.mainMenu, data->spriteBuffer, &data->font, data->arena_main);

    PlaySong(SONG_ID::THEME);
    
    SDL_Log("Changing scene");
    ChangeScene(data, SCENE_TYPES::MAINMENU); // SKALL BYTAS TILL MAINMENU NÄR DU FÅR ORDNING PÅ SKITEN // NU HAR JAG FÅTT ORDNING PÅ SKITEN
}

void StartLevel(Gameplay* gameplay, Arena* arena_commands, Arena* arena_entities)
{
	ResetCommandBuffer(gameplay->commandBuffer); // Resettar commands mellan nivåer
	Reset(arena_commands); // Dubbelkollar att vi inte har några commands från en tidigare level i vår arena_command
	CreateEntities(&gameplay->levels[gameplay->currentLevelIndex], arena_entities); // Skapar entities för nivån bestämt från currentLevelIndex
	gameplay->activePlayerIndex = 0;
}

void ChangeScene(GameData* data, SCENE_TYPES new_scene)
{
		assert(new_scene != data->scene_current); // Assertar att vi inte försöker ändra scenen till scenen som vi redan är i
		data->scene_previous = data->scene_current; // Lagrar current_scene i previous_scene
		data->scene_current = new_scene; // Uppdaterar till ny current_scene
		data->transition.state = data->scene_previous == SCENE_TYPES::NONE ? Transition::FadeFrom : Transition::FadeTo; // ? operator för att bestämma om vi behöver fadea in eller ut
		data->transition.fade_time_elapsed = 0; 
		switch (data->scene_current)
		{
		case SCENE_TYPES::TITLESCREEN:
			data->transition.fade_time_duration = 1;
				break;

		case SCENE_TYPES::MAINMENU:
				break;

		case SCENE_TYPES::GAME: 
			{
				data->transition.fade_time_duration = 0.5f;
				Gameplay* gameplay = &data->scenes.gameplay;
				assert(gameplay->initialized);
				StartLevel(gameplay, data->arena_commands, data->arena_entities);
				break; 
			}

		case SCENE_TYPES::CREDITS:
				break;

		case SCENE_TYPES::NONE:
			assert(false); // Vi råkar byta till ingen scen = krasch
			break;
		}
}

	// Skickar händelser till ImGui och avslutar spelet när Escape trycks.
bool HandleEvents(GameData *data, SDL_Event event)
{

		DEV::ProcessEvents(&event);

		if(event.type == SDL_EVENT_QUIT)
		{
			return false;
		}

		if(event.type == SDL_EVENT_KEY_DOWN)
		{
			if(event.key.key == SDLK_ESCAPE)
			{
				return false;
			}
		}

		return true;
}

void UpdateTitlescreen(TitleScreen* titlescreen, const float dt)
{
}

// Detta är huvudfunktionen för när man spelar en bana.
// Den håller koll på om man trycker på knappar, flyttar gubben, ångrar drag 
// och om man har lyckats klara banan.
void UpdateGame(Gameplay* gameplay, Input* input, Arena* arena_scratch, Arena* arena_commands, Arena* arena_entities, const float dt)
{
	// Starta om nivå med R 
    if(KeyPressed(input, SDL_SCANCODE_R))
    { 
      StartLevel(gameplay, arena_commands, arena_entities);
      return;
    }

    
    float undo_speed_up = std::lerp(1.0, 0.15, (gameplay->commandBuffer->head - gameplay->commandBuffer->index) * (1.0/30.0));

    if(undo_speed_up < 0.15)
    {
      undo_speed_up = 0.15;
    }
    if(KeyPressed(input, SDL_SCANCODE_Z) || KeyHeld_ForTime(input, SDL_SCANCODE_Z, UNDO_REPEAT_TIME * undo_speed_up))
    {
      ResetKeyHeldTime(input, SDL_SCANCODE_Z);
      if(KeyHeld(input, SDL_SCANCODE_LSHIFT)){
        Redo(gameplay->commandBuffer, GetCurrentLevel(gameplay));
      }
      else{
        Undo(gameplay->commandBuffer, GetCurrentLevel(gameplay));
      }
    }
   
    if(KeyPressed(input,SDL_SCANCODE_RIGHT) || KeyHeld_ForTime(input,SDL_SCANCODE_RIGHT, (1.0f / MOVE_SPEED)))
    {
      ResetKeyHeldTime(input, SDL_SCANCODE_RIGHT);
      gameplay->input_buffer[gameplay->input_buffer_write_count++ % gameplay->input_buffer_capacity] = {1, 0};
    }
    if(KeyPressed(input,SDL_SCANCODE_LEFT) || KeyHeld_ForTime(input,SDL_SCANCODE_LEFT, (1.0f / MOVE_SPEED)))
    {
      ResetKeyHeldTime(input, SDL_SCANCODE_LEFT);
      gameplay->input_buffer[gameplay->input_buffer_write_count++ % gameplay->input_buffer_capacity] = {-1, 0};
    }
    if(KeyPressed(input,SDL_SCANCODE_UP) || KeyHeld_ForTime(input,SDL_SCANCODE_UP, (1.0f / MOVE_SPEED)))
    {
      ResetKeyHeldTime(input, SDL_SCANCODE_UP);
      gameplay->input_buffer[gameplay->input_buffer_write_count++ % gameplay->input_buffer_capacity] = {0, -1};
    }
    if(KeyPressed(input,SDL_SCANCODE_DOWN) || KeyHeld_ForTime(input,SDL_SCANCODE_DOWN, (1.0f / MOVE_SPEED)))
    {
      ResetKeyHeldTime(input, SDL_SCANCODE_DOWN);
      gameplay->input_buffer[gameplay->input_buffer_write_count++ % gameplay->input_buffer_capacity] = {0, 1};
    }
    
    
    bool are_entities_acting = false;
    LevelData *level = GetCurrentLevel(gameplay);
    Entity* entityBuffer = level->entityBuffer;

    for (int i = 0; i < level->entityCount; i++)
    {
      if(IsActing(&entityBuffer[i])){
        are_entities_acting = true;
        break;
      }
    }

    for (int i = 0; i < level->goalCount; i++) 
    {
      Entity* entity = GetEntity(level, level->goals[i].x, level->goals[i].y);
      if(entity != nullptr && !IsActing(entity)){
        level->goals[i].blink_timer += dt;
      }
      else
      {
        level->goals[i].blink_timer = 0;
      }
    }

    if(level->goalCount > 0)
    {
      int goals_reached = 0;
      for (int i = 0; i < level->goalCount; i++) 
      {
        Goal goal = level->goals[i];
        Entity* entity = GetEntity(level, goal.x, goal.y);
        if(entity == nullptr)
        {
          continue;
        }
        else if(HasBehaviour(entity, Behaviour::IS_PLAYER))
        {
          goals_reached++;
        }
      }
      if(goals_reached == level->goalCount)
      {
        gameplay->currentLevelIndex++;
        StartLevel(gameplay, arena_commands, arena_entities);
        return;
      }      
    }

    for (int i = 0; i < level->entityCount; i++)
    {
      Entity* entity = &entityBuffer[i];
      if(!entity->active) continue;
        switch(entity->action){
        case Actions::NONE:
          continue;
        case Actions::MOVING:
          entity->progress_01 += MOVE_SPEED * dt;
            break;
        case Actions::ROTATING:
          entity->progress_01 += 8 * dt;
          break;
      }
    }
    for (int i = 0; i < level->entityCount; i++)
    {
      Entity* entity = &entityBuffer[i];
      if(entity->progress_01 >= 1){
        entity->x_prev = entity->x;
        entity->y_prev = entity->y;
        entity->facing_previous = entity->facing_current;
        entity->action = Actions::NONE;
        entity->progress_01 = 0;
        if(HasBehaviour(entity, Behaviour::IS_PUSHING)){
          RemoveBehaviour(entity, Behaviour::IS_PUSHING);
        }
      }
    }
    
    int player_count = 0;
    gameplay->player_count= 0;

    for (int i = 0; i < level->entityCount; i++) 
    {
      if(entityBuffer[i].active == false)
      {
        continue;
      }
      if(HasBehaviour(&level->entityBuffer[i], (Behaviour)(IS_PLAYER)))
      {
        gameplay->player_count++;
      }
    }

    int index = 0;
    gameplay->activePlayerBuffer = (Entity**)Memory::Allocate(arena_scratch, sizeof(Entity*) * gameplay->player_count);
    for (int i = 0; i < level->entityCount; i++) {
      if(entityBuffer[i].active == false)
      {
        continue;
      }
      if(HasBehaviour(&level->entityBuffer[i], (Behaviour)(IS_PLAYER)))
      {
        gameplay->activePlayerBuffer[index++] = &level->entityBuffer[i];
      }
    }

    if(are_entities_acting == false && KeyPressed(input, SDL_SCANCODE_X) && gameplay->player_count > 0)
    {
      SwapActiveEntityCommand swap(&gameplay->activePlayerIndex, gameplay->player_count);
      Push(gameplay->commandBuffer, swap, GetCurrentLevel(gameplay));
      gameplay->commandBuffer->timestamp += 1;
    }

    Entity* entity = GetActiveEntity(gameplay);
        
    if(are_entities_acting)
    {
      return;
    }

    for (int i = 0; i < level->entityCount; i++)
    {
      Entity* enemy = &level->entityBuffer[i];

      if (enemy->active && enemy->id == ENTITY_ID::ENEMY)
      {
        // Kollar om fiende och spelare står på samma X / Y koordinat
        if (enemy->x == entity->x && enemy->y == entity->y)
        {
          if(!IsActing(enemy) && !IsActing(entity))
          {
            SDL_Log("Restarting");
            StartLevel(gameplay, arena_commands, arena_entities);
            return;
          }
        }     
      }

    }

    if(gameplay->input_buffer_read_count == gameplay->input_buffer_write_count)
    {
      return;
    }


	   if (entity == nullptr)
		{
	    gameplay->input_buffer_read_count = gameplay->input_buffer_write_count;
	    return;
		}
    
    if(!HasBehaviour(entity, (Behaviour)(RESPOND_TO_INPUT | CAN_MOVE)))
    {
      return;
    }
    
    if(HasBehaviour(entity, Behaviour::IS_PETRIFIED))
    {
      return;
    }

    int xDir = gameplay->input_buffer[gameplay->input_buffer_read_count % gameplay->input_buffer_capacity].x;
    int yDir = gameplay->input_buffer[gameplay->input_buffer_read_count % gameplay->input_buffer_capacity].y;

    Direction new_facing = DirectionFromXY(xDir, yDir);
    if(new_facing != entity->facing_current)
    { 
      RotateCommand rotate(entity, entity->facing_current, new_facing);
      Push(gameplay->commandBuffer, rotate, level);
      // gameplay->input_buffer_read_count++;
      return;
    }

    if(!IsActing(entity))
    {
      bool moved = TryMove(entity, level, gameplay->commandBuffer, xDir, yDir, entity->strength);
      if(moved){
        PlaySFX(SFX_ID::JUMP);

        for(int i = 0; i < level->entityCount; i++) // Loopar igenom alla entiteter på banan
        {
        	Entity* enemy = &level->entityBuffer[i];

        	// Kollar om entity är en aktiv fiende som skall följa spelaren
        	if(enemy->active && HasBehaviour(enemy, FOLLOWS_PLAYER))
        	{
        		int nextX, nextY;

        		// Anropar A*-Funktion
        	if(FindNextStepAStar(enemy, entity, level, &nextX, &nextY))
        	{

        		// Räknar ut rikningen mot spelaren 
        		int exDir = nextX - enemy->x;
        		int eyDir = nextY - enemy->y;

            if(exDir < 0)
            {
              enemy->facing_current = Direction::LEFT;
            }
            else if (exDir > 0)
            {
              enemy->facing_current = Direction::RIGHT;
            }

        		TryMove(enemy, level, gameplay->commandBuffer, exDir, eyDir, enemy->strength); // Försökoer om möjligt flytta fienden
        	}
        	}
        }
      }
      gameplay->commandBuffer->timestamp += 1;
      gameplay->input_buffer_read_count++;
    }

  }

	// Uppdaterar editor, undo/redo, inputkö, animationer och spelarhandlingar.
// Update-funktionen körs hela tiden (många gånger per sekund).
// Den bestämmer vilken del av spelet som ska vara aktiv (meny, bana, startskärm)
// och ser till att saker rör sig mjukt.
void Update(GameData* data, float dt)
{

		*data->ticks_total += 1;

		TitleScreen* titlescreen = &data->scenes.titlescreen;
		Gameplay* gameplay = &data->scenes.gameplay;
		EditorData* editorData = &data->editor_data;
		Transition* transition = &data->transition;

		/// Edit_level mode
		if(KeyPressed(&data->input, SDL_SCANCODE_F2))
		{
			editorData->edit_level = !editorData->edit_level;
		}

		if(editorData->edit_level)
		{
			EDITOR::Update(&editorData->editor, &data->input, GetCurrentLevel(gameplay), gameplay->commandBuffer);
		}

		if(KeyPressed(&data->input, SDL_SCANCODE_5))
		{
			ChangeScene(data, SCENE_TYPES::TITLESCREEN);
			return;
		}

		if(transition->state != Transition::Inactive)
		{
			transition->fade_time_elapsed += dt;
			if(transition->fade_time_elapsed >= transition->fade_time_duration)
			{
				transition->fade_time_elapsed = 0;
				switch (transition->state)
				{
				case Transition::Inactive:
					break;

				case Transition::FadeTo:
					transition->state = Transition::FadeFrom;
					break;

				case Transition::FadeFrom:
					transition->state = Transition::Inactive;
					break;
				}
			}
		}

		switch(data->scene_current)
		{
		case SCENE_TYPES::TITLESCREEN:
			UpdateTitlescreen(titlescreen, dt);
			if(AnyKeyPressed(&data->input))
			{
				if(transition->state == Transition::FadeTo || transition->state == Transition::Inactive)
				{
					ChangeScene(data, SCENE_TYPES::GAME);
				}
			}
				break;

		case SCENE_TYPES::MAINMENU:
			UpdateMenu(data);
			break;

		case SCENE_TYPES::GAME:
			UpdateGame(gameplay, &data->input, data->arena_scratch, data->arena_commands, data->arena_entities, dt);
			break;

		case SCENE_TYPES::CREDITS:
			break;
			
		case SCENE_TYPES::NONE:
			assert(false);
			break;

		}	
}

// Försöker flytta entiteten en cell och säger till objekt framför den att den vill flytta objektet ett snäpp framåt.
// TryMove är hjärnan bakom hur figurer flyttar sig.
// Den kollar: Finns det plats? Står det något i vägen? Kan vi knuffa det som står i vägen?
// Om allt går bra flyttas figuren och draget sparas i "ångra-minnet".
bool TryMove(Entity* mover, LevelData* level, CommandBuffer* cmd_buffer, int xDir, int yDir, int strength)
{

if(strength < 0)
{
	return false;
}

	if(HasBehaviour(mover, CAN_MOVE) == false)
	{
		return false;
	}

	int test_x = mover->x + xDir; // Kolumnen som rörelsen försöker gå in i.
	int test_y = mover->y + yDir; // Raden som rörelsen försöker gå in i.

	Entity* stepInto_entity = GetEntity(level, test_x, test_y); // Eventuell blockerande entitet på målcellen.

	if (stepInto_entity == nullptr)
	{
		if(IsWalkable(test_x, test_y, level))
		{
			MoveCommand mv(mover, xDir, yDir);
	
			Push(cmd_buffer, mv, level);
			return true;
		}

		return false;

	}


	// Fienden ska kunna gå in i spelarens ruta för att fånga hen
	if (HasBehaviour(mover, FOLLOWS_PLAYER) && HasBehaviour(stepInto_entity, IS_PLAYER))
	{
		MoveCommand mv(mover, xDir, yDir);
		Push(cmd_buffer, mv, level);
		return true;
	}

  if (HasBehaviour(mover, IS_PLAYER) && HasBehaviour(stepInto_entity, FOLLOWS_PLAYER))
  {
    MoveCommand mv(mover, xDir, yDir);
    Push(cmd_buffer, mv, level);
    return true;
  }


	if(HasBehaviour(stepInto_entity, CAN_MOVE) && !HasBehaviour(stepInto_entity, UNPUSHABLE))
	{
		if(TryMove(stepInto_entity, level, cmd_buffer, xDir, yDir, --strength))
		{
			MoveCommand mv(mover, xDir, yDir);
			mv.type = CMD_TYPE::MOVE;
			mv.entity = mover;
			mv.xDir = xDir;
			mv.yDir = yDir;
			AddBehaviour(mover, Behaviour::IS_PUSHING);
			Push(cmd_buffer, mv, level);
			return true;
		}
	}

	return false;
}


// Switch funktion för att byta mellan de olika scenerna
void DrawScene(GameData* data, SCENE_TYPES scene, SDL_Renderer* renderer)
{
		switch(scene)
		{
		case SCENE_TYPES::TITLESCREEN:
			{
				Sprite* background = GetSprite(SPRITE_ID::titlescreen_background, data->spriteBuffer);
				RenderSprite_World(background, renderer, &data->camera, 0, 0);
			}
			break;

		case SCENE_TYPES::MAINMENU:
			DrawMenu(&data->scenes.mainMenu, renderer, data->spriteBuffer, &data->input);
			break;

		case SCENE_TYPES::GAME:
			RenderLevel (data, renderer);
			RenderEntities(data,renderer);
			break;

		case SCENE_TYPES::CREDITS:
			break;

		case SCENE_TYPES::NONE:
			assert(false);
			break;
		}
}

// Rensar bildytan och ritar nivå, entiteter och utvecklargränssnitt.
void Draw(GameData* data, SDL_Renderer* renderer)
{

		DEV::PreDraw(data->imGui_context);

		SDL_SetRenderDrawColor(renderer, 34, 34, 34, 255);
		SDL_RenderClear(renderer);



		switch (data->transition.state)
		{
			// Transition är inaktiv - Under en scen
		case Transition::Inactive:
			DrawScene(data, data->scene_current, renderer);
			break;

			// Transition är aktiv, fadear in till sprite black_1x1
		case Transition::FadeTo:
			{
				DrawScene(data, data->scene_previous, renderer);
				float alpha = data->transition.fade_time_elapsed / data->transition.fade_time_duration;
				RenderSprite_World(GetSprite(SPRITE_ID::black_1x1, data->spriteBuffer), renderer, &data->camera, 0, 0, SCREEN_WIDTH, alpha);
				break;
			}

		case Transition::FadeFrom:
			{
				// Transition är aktiv, fadear ut från black_1x1 till scene_current 
				DrawScene(data, data->scene_current, renderer);
				float alpha = 1 - data->transition.fade_time_elapsed / data->transition.fade_time_duration;
				RenderSprite_World(GetSprite(SPRITE_ID::black_1x1, data->spriteBuffer), renderer, &data->camera, 0, 0, SCREEN_WIDTH, alpha);
				break;
			}
		}

		DEV::Draw(data, renderer);
		SDL_RenderPresent(renderer);
}

// Lossar SDL-renderaren när programmet avslutas.
void OnQuit(SDL_Renderer* renderer)
{
		SDL_DestroyRenderer(renderer);
}

}