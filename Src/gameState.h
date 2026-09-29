#pragma once

#include <cstdint> // Tillhandahåller heltalstyper i bestämd storlek.
#include "SDL3/SDL_rect.h" // SDL-typer som kan användas av speldata.
#include "image.h" // Ingår för bildrelaterade typer; filen är för närvarande tom.
#include "levels.h" // LevelData och nivåernas minnesarena används här.
#include "command.h" // CommandBuffer lagras som del av spelets tillstånd.
#include "imgui/imgui.h" // ImGuiContext sparas över DLL-omladdningar.
#include "input.h" // Input lagras persistent mellan bildrutor.
#include "camera.h" // Camera lagras som del av spelets tillstånd.
#include "spriteLibrary.h" // Spritebufferten används i GameData.
#include "leveleditor.h" // Editor-data används i GameData.
#include "tilesetLibrary.h" // För att få tillgång till Tileset* pointern.
#include "mainmenu.h" // För att få tillgång till mainmenu scripten så vi kan justera gamestate i main menu
#include "audioSystem.h"
#include "fontLibrary.h"

// Olika typer av scener (platser) i spelet.
enum class SCENE_TYPES : uint8_t
{
	NONE,           // Ingen scen vald ännu.
	TITLESCREEN,    // Startskärmen.
	MAINMENU,       // Huvudmenyn.
	GAME,           // Själva spelnivån där man pusslar.
	CREDITS,        // Eftertexter.
};


// Innehåller allt som rör själva spelandet.
struct Gameplay
{
	CommandBuffer* commandBuffer; // Spelets minne för att kunna ångra drag.
	LevelData* levels; // En lista med alla banor i spelet.
	int levelCount;  // Hur många banor det finns totalt.
	int currentLevelIndex; // Vilken bana vi spelar på just nu.
	Position* input_buffer; // En kö med knapptryckningar som väntar på att utföras.
	int input_buffer_capacity; // Hur många knapptryckningar vi kan ha i kön samtidigt.
	int input_buffer_write_count; // Räknare för inskrivna kommandon.
	int input_buffer_read_count; // Räknare för utförda kommandon.

	bool initialized; // Är spelsystemet färdigt att användas?

	int activePlayerIndex; // Vilken figur du styr om det finns flera.
	Entity** activePlayerBuffer; // En lista över alla figurer som spelaren kan styra.  
	int player_count; // Hur många figurer spelaren har på banan just nu.

};

// Hanterar mjuka övergångar (toningar) mellan olika scener.
struct Transition
{
	enum States
	{
		Inactive, // Ingen toning pågår.
		FadeTo,   // Skärmen blir svart.
		FadeFrom  // Skärmen blir ljus igen.
	};

	States state;
	float fade_time_elapsed; // Hur lång tid toningen har pågått.
	float fade_time_duration = 1; // Hur lång tid toningen ska ta totalt.
};

struct TitleScreen // Data för startskärmen.
{

};

struct Credits // Data för eftertexterna.
{

};

struct Scenes // Samlar alla olika scener på ett ställe.
{
	Gameplay gameplay;
	MainMenu mainMenu;
	TitleScreen titlescreen;
	Credits credits;
};

// Innehåller inställningar för baneditorn (verktyget för att bygga banor).
struct EditorData
{
	bool edit_level; // Är baneditorn igång?
	Editor editor; // Specifik data för editorn (valt verktyg etc).

	// Statistik för att se hur spelet presterar (bilder per sekund).
	float fps_buffer[500];
	int fps_buffer_index;
	int fps_buffer_count = 120;

};

// GameData är "hjärtat" i spelet. Här samlas ALL information som behövs för att köra spelet.
struct GameData
{

	// Variabler som styr vilken scen som visas och hur vi byter mellan dem.
	// -------------------------------------------------------------------------
	SCENE_TYPES scene_current;  // Scenen vi ser just nu.
	SCENE_TYPES scene_previous; // Scenen vi kom ifrån.
	Scenes scenes;              // All data för alla scener.
	Transition transition;      // Information om skärmtoningar.
	EditorData editor_data;     // Inställningar för baneditorn.
	// -------------------------------------------------------------------------

	Sprite* spriteBuffer; // En lista med alla bilder/figurer i spelet.


	Memory::Arena* arena_main; // Huvudminnet för hela spelet.
	Memory::Arena* arena_levels; // Minne reserverat för banor.
	Memory::Arena* arena_entities; // Minne reserverat för figurer på banan.
	Memory::Arena* arena_images; // Minne reserverat för bilder.

	Memory::Arena* arena_commands; // Minne reserverat för ångra-historiken.

	Memory::Arena* arena_scratch; // Ett tillfälligt minne som töms varje bildruta.

	LevelData* levels; // Banorna i spelet.

	const float* dt; // Hur lång tid en bildruta tar (används för jämna rörelser).
	float* dt_scaler; // Kan användas för att göra spelet snabbare eller långsammare (Slow motion).

	ImGuiContext* imGui_context; // Data för utvecklarverktygen.

	Input input; // Allt om vilka knappar spelaren trycker på.
	Arena* arena_input; // Minne för inmatningsdata.

	Camera camera; // Vart kameran är riktad på spelplanen.

	bool edit_level; // Gammal variabel, flyttad till EditorData egentligen.
	Editor editorData; 

	Tileset* tilesetBuffer; // Innehåller grafikbitar för att bygga banan.

	bool running; // Är spelet igång? Om denna blir falsk stängs programmet.

	AudioSystem audio; // Allt som rör ljud och musik.

	uint64_t* ticks_total; // Räknar totalt antal bildrutor sedan start.

	FontAtlas font; // Innehåller textstilen som används i spelet.

};


inline LevelData* GetCurrentLevel(Gameplay* game) // Hämtar banan vi är på just nu.
{
	return &game->levels[game->currentLevelIndex];
}

inline Entity* GetActiveEntity(Gameplay* game) // Hämtar figuren som spelaren styr just nu.
{
	if (game->player_count == 0)
	{
		return nullptr;
	}

	int safe_index = game->activePlayerIndex % game->player_count;
	return game->activePlayerBuffer[safe_index];
}
