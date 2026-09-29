#pragma once

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "fontLibrary.h"
#include "rendering.h"

struct GameData;
struct Sprite;

enum class ButtonType
{
	NONE,
	START_GAME,
	QUIT
};

struct Button 
{
	ButtonType type; 
	SDL_FRect rect;
	Sprite* sprite;
	bool is_active;
	bool is_dynamic;
	FontAtlas* font;
	const char* text;
};

void PressButton(Button* button, 
				GameData* data);

// Helper för att loopa genom alla buttons och 
int GetActiveButtonCount(Button* buttons, 
						int count);

// Kollar current mouse position - collision bounds detection
bool IsHoveredOver(Button* button, 
					float x, 
					float y);

// Används för att skapa en button
void SetupButton(Button* button, 
				ButtonType type, 
				Sprite* spriteBuffer, 
				SDL_FRect rect, 
				Alignment mode,
				FontAtlas* font = nullptr,
				const char* text = nullptr);