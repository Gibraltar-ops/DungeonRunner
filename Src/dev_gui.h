#pragma once

#include "SDL3/SDL_render.h" // SDL_Renderer används för ImGui-rendering.
#include "SDL3/SDL_video.h" // SDL_Window behövs vid ImGui-initiering.
#include "imgui/imgui_impl_sdl3.h" // SDL3-backendens eventhantering för ImGui.
#include "gameState.h" // GameData ger GUI:t åtkomst till debug-information.


namespace DEV
{
	void Initialize(SDL_Window* window, SDL_Renderer *renderer); // Initierar ImGui och dess SDL-backends.
	void ProcessEvents(SDL_Event* event); // Skickar SDL-händelser till ImGui.
	void PreDraw(ImGuiContext* saved_context); // Startar en ny ImGui-bildruta.
	void Draw(GameData* data, SDL_Renderer* renderer); // Ritar utvecklarverktyg och editorpaneler.
}
