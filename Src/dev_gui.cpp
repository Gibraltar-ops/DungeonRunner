#include "dev_gui.h" // Deklarationerna för utvecklargränssnittet implementeras här.
#include "gameState.h" // GameData innehåller minnesarenor, editor och kommandohistorik.
#include "command.h" // Undo och redo används av historikreglaget.
#include "imgui/imgui_impl_sdlrenderer3.h" // ImGui:s SDL-rendererbackend ritar GUI:t.
#include "SDL3/SDL_render.h" // Fönster- och rendererinformation hämtas från SDL.
#include "imgui/imgui.h" // ImGui-widgetar och kontext används direkt.
#include "imgui/imgui_impl_sdl3.h" // ImGui:s SDL3-backend hanterar händelser och bildrutor.
#include "imgui/imgui_internal.h" // Ingår för interna ImGui-typer; används inte direkt i denna fil.
#include "leveleditor.h" // Editorpanel och förhandsvisning visas i edit-läge.
#include <string> // std::string och to_string används för arenans informationstext.

using namespace std;

// Skapar ImGui-kontexten och kopplar den till programmets SDL-fönster och renderer.
void DEV::Initialize(SDL_Window* window, SDL_Renderer* renderer)
{
	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);

	ImGuiIO& io = ImGui::GetIO(); // ImGui:s inställningar för den aktiva kontexten.
	int w, h; // Fönstrets aktuella bredd och höjd i pixlar.
	SDL_GetWindowSize(window, &w, &h);
	io.DisplaySize = ImVec2((float)w, (float)h);
}

// Vidarebefordrar varje SDL-händelse så att ImGui kan reagera på den.
void DEV::ProcessEvents(SDL_Event* event)
{
	ImGui_ImplSDL3_ProcessEvent(event);
}

// Säkerställer rätt ImGui-kontext och startar en ny GUI-bildruta.
void DEV::PreDraw(ImGuiContext* saved_context)
{

	if(ImGui::GetCurrentContext() == nullptr)
	{
		ImGui::SetCurrentContext(saved_context);
	}

	ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

// Ritar en progressbar som visar hur mycket av en minnesarena som används.
void Draw_Imgui_Arena_Usage(Arena* arena, std::string name_of_arena)
{
	float fraction = (float) arena->used / (float)arena->size; // Andel av arenans minne som redan är allokerat.

		string barText = name_of_arena;
		barText += " / " + to_string(arena->used);
		barText += " / " + to_string(arena->size);
	ImGui::ProgressBar(fraction, ImVec2(-1,0), barText.c_str());

}


// Visar ett reglage som kan flytta fram och tillbaka i undo/redo-historiken.
void Draw_History(CommandBuffer* buffer, LevelData* level)
{
	int sliderPos = buffer->index; // Önskad position i historiken, styrd av ImGui-reglaget.

	if(ImGui::SliderInt("history", &sliderPos, 0, buffer->head))
	{
		while(buffer->index > sliderPos)
		{
			Undo(buffer, level);
		}
		while(buffer->index < sliderPos)
		{
			Redo(buffer, level);
		}
	}
}

// Visar aktuell bildfrekvens beräknad från delta time.
void DrawFPS(GameData* data)
{
	EditorData* editor = &data->editor_data;

	editor->fps_buffer[editor->fps_buffer_index++] = 1.0 / *data->dt * *data->dt_scaler;
	editor->fps_buffer_index %= editor->fps_buffer_count;

	ImGui::PlotHistogram("fps", editor->fps_buffer, editor->fps_buffer_count, 0, nullptr, 0, 144.0f, ImVec2(-1,35));
}

// Ritar alla utvecklarpaneler och skickar ImGui:s färdiga draw data till SDL.
void DEV::Draw(GameData* data, SDL_Renderer* renderer)
{

	Gameplay* gameplay = &data->scenes.gameplay;

	ImGui::Begin("Dev Tools");
	ImGui::Text("memory arena usage amount");


	Draw_Imgui_Arena_Usage(data->arena_main, "all memory");
	Draw_Imgui_Arena_Usage(data->arena_images, "images");
	Draw_Imgui_Arena_Usage(data->arena_levels, "levels");
	Draw_Imgui_Arena_Usage(data->arena_commands, "commands");
	Draw_Imgui_Arena_Usage(data->arena_entities, "entities");
	Draw_Imgui_Arena_Usage(data->arena_input, "input");
	Draw_Imgui_Arena_Usage(data->arena_scratch, "scratch");

	DrawFPS(data);

	ImGui::SliderFloat("deltaTimeScaler", data->dt_scaler, 0.1, 3);

	Draw_History(gameplay->commandBuffer, GetCurrentLevel(gameplay));

	ImGui::End();

	if (data->edit_level)
	{
		EDITOR::DrawObjectPanel(&data->editorData, data->spriteBuffer);
		EDITOR::DrawPreview(&data->editorData, &data->input, renderer, GetCurrentLevel(gameplay), &data->camera, data->spriteBuffer);
	}

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
