#pragma once

#include "SDL3/SDL_render.h" // SDL_Window och SDL_Renderer används av spelets exporterade funktioner.
#include "gameState.h" // GameData innehåller allt persistent speldata.
#include "command.h" // CommandBuffer behövs av TryMove.


extern "C" 
{
	// Dessa funktioner är själva kopplingen mellan spelets motor och spelets logik.
	// __declspec(dllexport) betyder att de görs tillgängliga för huvudprogrammet.

	__declspec(dllexport) void Initialize(GameData* data, SDL_Window* window, SDL_Renderer* renderer); // Startar upp spelet och laddar in allt som behövs.
	__declspec(dllexport) bool HandleEvents(GameData* data, SDL_Event event); // Tar hand om saker som händer, t.ex. om man stänger fönstret.
	__declspec(dllexport) void Draw(GameData* data, SDL_Renderer* renderer); // Ritar ut allt på skärmen.
	__declspec(dllexport) void Update(GameData* data, float dt); // Räknar ut allt som händer i spelet (rörelser, logik etc).
	__declspec(dllexport) void OnQuit(SDL_Renderer* renderer); // Städar upp efter sig när spelet stängs av.

void ChangeScene(GameData* data, SCENE_TYPES new_scene); // Byter mellan olika menyer eller banor.
bool TryMove(Entity* mover, LevelData* level, CommandBuffer* cmd_buffer, int xDir, int yDir, int strength); // Kollar om en figur kan gå åt ett visst håll, eller om den kan knuffa något som står i vägen.
}

