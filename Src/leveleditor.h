#pragma once

#include "camera.h" // Camera används för förhandsvisningen i rutnätet.
#include "input.h" // Input används för placering och borttagning med musen.
#include "spriteLibrary.h" // Sprite behövs för editorpanelens bildknappar.

struct Editor
{
	ENTITY_ID object_to_place_id; // Objekt-ID:t som placeras när användaren klickar.
};

namespace EDITOR
{
	void DrawObjectPanel(Editor* editor, Sprite* spriteBuffer); // Ritar valbara objekt i editorpanelen.
	void PlaceObject(const int x, const int y, Editor* editor, LevelData* level, CommandBuffer* commandBuffer); // Lägger in valt objekt på cellen.
	void Update(Editor* editor, Input* input, LevelData* level,  CommandBuffer* commandBuffer); // Hanterar editorlägets musklick.
	void DrawPreview(Editor* editor, Input* input, SDL_Renderer* renderer, LevelData* level, Camera* camera, Sprite* spriteBuffer); // Ritar transparent objekt under musen.
}
