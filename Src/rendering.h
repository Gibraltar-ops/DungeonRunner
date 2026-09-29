#pragma once

#include "SDL3/SDL_render.h" // SDL_Renderer används för att rita sprites.
#include "image.h" // Ingår för bildrelaterade typer; filen är för närvarande tom.
#include "camera.h" // Camera används för att förskjuta världen vid ritning.
#include "levels.h" // LevelData behövs för rutnätskoordinater.
#include "spriteLibrary.h" // Sprite innehåller textur och pivotpunkt för ritning.


struct Button;
struct FontAtlas;

// Bestämmer hur text ska justeras.
enum class Alignment
{
	Left,     // Vänsterställd.
	Centered  // Centrerad.
};

// Ritar en enskild bakgrundsbit (tile) på skärmen.
void RenderTile(Sprite* tileset, int cell_id, LevelData* level, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale, float alpha);

// Ritar en bild (sprite) någonstans i spelvärlden.
void RenderSprite_World(SpriteRenderInfo tileset, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale = 1, float alpha = 1, bool flipped = false);

// Ritar en bild exakt ovanpå en spelruta.
void RenderSprite_OnTile(SpriteRenderInfo spriteInfo, LevelData* level, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale = 1, float alpha = 1, bool flipped = false);

// Ritar en knapp i menyn.
void RenderButton(Button* button, bool is_selected, SDL_Renderer* renderer);

// Ritar en knapp som kan ändra storlek eller utseende dynamiskt.
void RenderButton_Dynamic(Button* button, bool is_selected, SDL_Renderer* renderer);

// Skriver ut text på skärmen med ett specifikt typsnitt.
void RenderText(FontAtlas* atlas, const char* text, SDL_Renderer* renderer, Camera* camera, const float x, const float y, Alignment alignment);