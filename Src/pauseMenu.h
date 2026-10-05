#pragma once

#include "gameState.h"

void InitializePauseMenu(
    PauseMenu* menu,
    Sprite* spriteBuffer,
    FontAtlas* font,
    Memory::Arena* arena_main);

void UpdatePauseMenu(GameData* data);

void DrawPauseMenu(
    GameData* data,
    SDL_Renderer* renderer);