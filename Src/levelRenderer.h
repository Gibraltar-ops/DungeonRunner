#pragma once

#include "gameState.h" // GameData innehåller aktiv nivå, sprites och kamera.

void RenderLevel (GameData* gameData, SDL_Renderer* renderer); // Ritar nivåns celler.
void RenderEntities (GameData* gameData, SDL_Renderer* renderer); // Ritar och animerar nivåns entiteter.

