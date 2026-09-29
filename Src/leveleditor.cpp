#include "leveleditor.h" // Editor-gränssnittets deklarationer implementeras här.
#include "camera.h" // Musens världsläge översätts till rutnätskoordinater.
#include "imgui/imgui.h" // ImGui används för objektens bildknappar.
#include "levels.h" // Celler och entiteter ändras vid placering/borttagning.
#include "rendering.h" // Förhandsvisningen ritas som en transparent sprite.
#include "spriteLibrary.h" // Valda objekt hämtar sin sprite från bufferten.
#include "command.h" // Används för att få kunna använda AddCommand

namespace EDITOR
{
	// Ritar editorpanelens knappar och sparar användarens objektval.
	void DrawObjectPanel(Editor* editor, Sprite* spriteBuffer)
	{
		ImGui::Begin("objects");
		ImVec2 size = {32, 32}; // Storleken på varje spriteknapp i panelen.
	
		if(ImGui::ImageButton("Rock", (ImTextureID)GetSpriteFromID(ENTITY_ID::ROCK, spriteBuffer)->texture, size))
		{
			editor->object_to_place_id = ENTITY_ID::ROCK;
		}

		ImGui::SameLine();
		 if(ImGui::ImageButton("Demon", (ImTextureID)GetSpriteFromID(ENTITY_ID::DEMON, spriteBuffer)->texture, size))
		{
			editor->object_to_place_id = ENTITY_ID::DEMON;
		}

		ImGui::SameLine();
		 if(ImGui::ImageButton("Medusa", (ImTextureID)GetSpriteFromID(ENTITY_ID::MEDUSA, spriteBuffer)->texture, size))
		{
			editor->object_to_place_id = ENTITY_ID::MEDUSA;
		}

		ImGui::End();
	}

	// Placerar mark/vägg direkt i cellagret eller skapar en entitet på cellen.
	void PlaceObject(const int x, const int y, Editor* editor, LevelData* level,  CommandBuffer* commandBuffer)
	{
		{
			AddCommand add (x, y, editor->object_to_place_id);
			Push(commandBuffer, add, level);
		}
	}

	// Visar valt objekt genomskinligt vid den cell där muspekaren befinner sig.
	void DrawPreview(Editor* editor, Input* input, SDL_Renderer* renderer, LevelData* level, Camera* camera, Sprite* spriteBuffer)
	{
		int x; 
		int y; 
		camera::WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
		Sprite* preview = GetSpriteFromID(editor->object_to_place_id, spriteBuffer);
		if (preview != nullptr)
		{
			RenderSprite_OnTile(preview, level, renderer, camera, x, y, 1, 0.5, false);
		}
	}

	// Placerar med vänsterklick och tar bort entiteter med högerklick inom rutnätet.
	void Update(Editor* editor, Input* input, LevelData* level, CommandBuffer* buffer)
	{
		if(MousePressed(input, MouseButtons::LEFT))
		{
			if(camera::GetIsPointInsideGrid(input->mouse_x, input->mouse_y, level))
			{
				int x;
				int y;
				camera::WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
				PlaceObject(x, y, editor, level, buffer);
			}
		}

		else if(MousePressed(input, MouseButtons::RIGHT))
		{
			if(camera::GetIsPointInsideGrid(input->mouse_x, input->mouse_y, level))
			{
				int x;
				int y;
				camera:: WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
				Entity* entity = GetEntity(level, x, y);
				if (entity == nullptr)
				{
					return;
				}
				RemoveCommand remove(entity);
				Push(buffer, remove, level);
			}
		}
	}

}
