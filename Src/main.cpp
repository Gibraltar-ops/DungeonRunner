#include <windows.h> // Windows API används för DLL-hantering, filkopiering och Sleep.
#include <fileapi.h> // FILETIME och filfunktioner används för att övervaka DLL:ens ändringstid.
#include <cstdio> // printf används för fel- och statusmeddelanden.
#include <fstream> // Binära strömmar används för att spara och återställa spelminne.
#include "SDL3/SDL_init.h" // SDL_Init och SDL_Quit initierar/stänger SDL.
#include "SDL3/SDL_events.h" // SDL_Event och SDL_PollEvent hanterar programhändelser.
#include "SDL3/SDL_timer.h" // Tidtagning och SDL_Delay används för bildfrekvensbegränsning.
#include "SDL3/SDL_render.h" // SDL-fönster, renderer och renderingstyper används i huvudloopen.
#include "SDL3_TTF/SDL_ttf.h"
#include "command.h" // CommandBuffer allokeras i det persistenta spelminnet.
#include "common.h" // Minnesbudget och måltid per bildruta används här.
#include "arena.h" // Minnesarenor skapas för allt persistent speldata.
#include "entity.h" // Position används av spelets inputringbuffert.
#include "gameState.h" // GameData delas med den hot-reloadade spel-DLL:en.
#include "image.h" // Ingår för bildrelaterade typer; filen är för närvarande tom.
#include "spriteLibrary.h" // Sprite används när spritebufferten dimensioneras.




SDL_Window* window; // Programmets SDL-fönster.
SDL_Renderer* renderer; // Renderaren som ritar till fönstret.

Uint64 NOW = 0; // Tidstämpel för aktuell bildruta i nanosekunder.
Uint64 PREV = 0; // Tidstämpel för föregående bildruta i nanosekunder.

///Pointer cannot change after initialization
constexpr const char* NAME_OF_DLL = "Heartburner_game.dll";
constexpr const char* NAME_OF_TEMP_DLL = "Hearburner_temp.dll";

///typedef takes us to structure of specific functions allowing easy storage
typedef void (*Function_Initialize) (GameData* data, SDL_Window* window, SDL_Renderer* renderer);
typedef bool (*Function_HandleEvents) (GameData* data, SDL_Event event);
typedef void (*Function_Update) (GameData* data, float dt);
typedef void (*Function_Draw) (GameData* data, SDL_Renderer* renderer);
typedef void (*Function_OnQuit) (SDL_Renderer* renderer);


///Char pointers som pekar till exakta namnen i game.h
constexpr const char* NAME_OF_FUNC_INIT = "Initialize";
constexpr const char* NAME_OF_FUNC_HANDLE_EVENT = "HandleEvents";
constexpr const char* NAME_OF_FUNC_UPDATE = "Update";
constexpr const char* NAME_OF_FUNC_DRAW = "Draw";
constexpr const char* NAME_OF_FUNC_QUIT = "OnQuit";




///Structen håller alla data som behövs för att arbeta med vår .DLL
struct DLL_INFO
{
	HMODULE dll;

	///"Containing a 64-bit value representing the number of 100-nanosecond intervals since Jan 1, 1601 (UTC)"
	FILETIME timestamp;
	Function_Initialize initialize;
	Function_HandleEvents handleEvents;
	Function_Update update;
	Function_Draw draw;
	Function_OnQuit quit;
};

///Timestampen används för att kolla om vår DLL har blivit omkompilerad efter att vi startade spelet
FILETIME GetTimestamp()
{
	WIN32_FIND_DATA data;
	HANDLE handle = FindFirstFile (NAME_OF_DLL, &data);
	FILETIME time_of_last_change = data.ftLastWriteTime;
	FindClose(handle);
	return time_of_last_change;
}


///Hittar kompilerad DLL, skapar en kopia(NAME_OF_TEMP_DLL) och sparar DLL:en i DLL_INFO structen
///Våra funktionspointers i DLL_INFO pekar vi på funktionerna vi taggat med __declspec(dllexport) i game.h
// Denna funktion hämtar spelets kod från en separat fil (DLL).
// Det gör att vi kan ändra spelet medan det körs utan att behöva starta om programmet.
bool LoadDLL(DLL_INFO* info, int depth = 0)
{
	printf("loading DLL");

	if(depth > 20)
	{
		///Om nedanstående funktion att hämta DLLen failar, testa att hämta max 20 ggr innan den stoppas
		printf("failed to write temp DLL");
		return false;
	}

	if (depth == 0)
	{
		DeleteFile(NAME_OF_TEMP_DLL);
	}

	bool success = CopyFile(NAME_OF_DLL, NAME_OF_TEMP_DLL, false);

	if (!success)
	{

		///Om funktionen failar, vänta 50ms och testa att kalla funktionen igen med ett ökat depth +1 (för att hitta DLLen tills slut)
		Sleep(50);
		return LoadDLL(info, depth + 1);
	}

	info->dll = LoadLibrary(NAME_OF_TEMP_DLL);

	if(info->dll == nullptr)
	{
		printf("could not load dll");
		return false;
	}

	info->initialize = (Function_Initialize)GetProcAddress(info->dll, NAME_OF_FUNC_INIT);
	info->handleEvents = (Function_HandleEvents)GetProcAddress(info->dll, NAME_OF_FUNC_HANDLE_EVENT);
	info->update = (Function_Update)GetProcAddress(info->dll, NAME_OF_FUNC_UPDATE);
	info->draw = (Function_Draw)GetProcAddress(info->dll, NAME_OF_FUNC_DRAW);
	info->quit = (Function_OnQuit)GetProcAddress(info->dll, NAME_OF_FUNC_QUIT);

	info->timestamp = GetTimestamp();

	return true;
}


///Accepterar en pointer till DLL_INFO structen, rensar sedan minnet där HMODULE(Pointer till vår DLL.) 
void UnloadDLL(DLL_INFO* info)
{
	FreeLibrary(info->dll);
	info->dll = nullptr;
	DeleteFile(NAME_OF_TEMP_DLL);
}


/// Används för att tagga ett memory block och returnerar en pointer till första bytes in minnet så att vår memory arena kan referera den
/// Om malloc inte lyckas returneras nullptr och programmet stänger av sig 
void* AllocateGameMemory()
{
	void* blob = malloc(GAME_MEMORY_ALLOWANCE);

	if(blob == nullptr)
	{
		printf("fatal error: could not allocate memory");
		return nullptr;
	}

	printf("memory succefully allocated");
	return blob;
}

///Initierar SDL med fönster och renderer
// Förbereder SDL, vilket är motorn som ritar fönstret och hanterar inmatning.
void SDL_Setup()
{
	SDL_Init(SDL_INIT_EVENTS);

	TTF_Init();
	window = SDL_CreateWindow("pilot", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer = SDL_CreateRenderer(window, NULL);
}


///Hämtar en pointer till en float och sätter den till kalkylerad deltatime
void CalculateDeltaTime(float* dt, float scaler)
{
	NOW = SDL_GetTicksNS();
	*dt = NOW - PREV;
	*dt = SDL_NS_TO_SECONDS(*dt);
	*dt *= scaler;
	PREV = NOW;
}

///Hämtar Filetime och jämför, har den ändras laddar vi ur den gamla DLLen med UnloadDLL och skapar sedan en ny med LOADDLL
void DLL_CheckStatus(DLL_INFO* dll)
{
	FILETIME timestamp = GetTimestamp();
	bool is_timestamp_changed = CompareFileTime(&dll->timestamp, &timestamp) != 0;

	if (is_timestamp_changed)
	{
		UnloadDLL(dll);
		LoadDLL(dll);
	}
}

// Beräknar återstående tid innan bildrutan når målet för 60 FPS.
void CalculateRemainingFrameTime_MS(double* milliseconds)
{
	Uint64 frame_end_time_ns = SDL_GetTicksNS();
	double frame_time_spent_ns = frame_end_time_ns - PREV;
	double frame_time_spent_ms = frame_time_spent_ns / 1e6;
	*milliseconds = FRAME_TIME_MS - frame_time_spent_ms;
}

// Skriver hela den persistenta spelarenan till en binär fil för snabb återställning.
void StoreGameState(Memory::Arena* arena)
{
	std::ofstream file("temp_state.bin", std::ios::binary);
	file.write(reinterpret_cast<const char*>(arena->base), arena->size);
}

// Läser tillbaka en tidigare sparad arena och återställer därmed spelstatusen.
void RetrieveGameState(Memory::Arena* arena)
{
	std::ifstream file("temp_state.bin", std::ios::binary);
	file.read(reinterpret_cast<char*>(arena->base), arena->size);
}


///Funktioner kallas på från vår DLL_INFO struct med function pointers som sätts i LoadDLL()
// Spelets huvudloop. Här snurrar programmet runt, runt så länge spelet är igång.
// Varje varv räknar den ut vad som händer, kollar knappar och ritar en ny bild.
int main()
{
	void* game_memory = AllocateGameMemory(); // Det råa minnesblock som alla spelarenor placeras i.
	if(game_memory == nullptr)
	{
		return 1;
	}

	SDL_Setup();

	Memory::Arena* arena_main = new Memory::Arena(); // Huvudarena som äger det persistenta spelminnet.

	Memory::Initialize(arena_main, game_memory, GAME_MEMORY_ALLOWANCE);

	GameData* gameData = ALLOC(arena_main, GameData); // Simplifierat makro för att allokera för arena_main
	gameData->arena_main = arena_main;
	gameData->ticks_total = ALLOC(arena_main, uint64_t);

	gameData->editor_data.fps_buffer_count = 500;
	gameData->transition.fade_time_duration = 1.0f;


	size_t INPUT_ARENA_SIZE = 0; // Beräknad storlek för tangent-, mus- och tidbuffertar.
	INPUT_ARENA_SIZE += sizeof(bool) * SDL_SCANCODE_COUNT * 2;
	INPUT_ARENA_SIZE += sizeof(float) * SDL_SCANCODE_COUNT;
	INPUT_ARENA_SIZE += 128;
	gameData->arena_input = Memory::CreateSubArena(arena_main, INPUT_ARENA_SIZE);

	gameData->input.keys_current = ALLOC_ARRAY(gameData->arena_input, bool, SDL_SCANCODE_COUNT);
    gameData->input.keys_previous = ALLOC_ARRAY(gameData->arena_input, bool, SDL_SCANCODE_COUNT);
    gameData->input.keys_held_time = ALLOC_ARRAY(gameData->arena_input, float, SDL_SCANCODE_COUNT);

	int mouseButtonCount = 3; // Antal musknappar som har egna nedhållningstimers.
	gameData->input.mouse_held_time = (float*)Memory::Allocate(gameData->arena_input, sizeof(float) * mouseButtonCount);

	int SPRITE_COUNT = 256; // Reserverat antal platser i spritebufferten.
	size_t IMAGE_ARENA_SIZE = MEGABYTES(1);
	gameData->arena_images = Memory::CreateSubArena(arena_main, IMAGE_ARENA_SIZE);
	gameData->spriteBuffer = ALLOC_ARRAY(gameData->arena_images, Sprite, SPRITE_COUNT);
	gameData->tilesetBuffer = ALLOC_ARRAY(gameData->arena_images, Tileset, (int)TILESETS::COUNT);
	gameData->arena_levels = Memory::CreateSubArena(arena_main, MEGABYTES(3));
	gameData->arena_entities = Memory::CreateSubArena(gameData->arena_levels, MEGABYTES(1));
	gameData->arena_commands = Memory::CreateSubArena(gameData->arena_levels, MEGABYTES(1));

	Gameplay* gameplay = &gameData->scenes.gameplay;

	gameplay->commandBuffer =(CommandBuffer*)Memory::Allocate(arena_main, sizeof(CommandBuffer));
	gameplay->commandBuffer->capacity = 2000;
	size_t COMMAND_SIZE = sizeof(AnyCommand) * gameplay->commandBuffer->capacity;
	gameplay->commandBuffer->allCommands = (AnyCommand*)Memory::Allocate(gameData->arena_commands, COMMAND_SIZE);

	gameplay->levelCount = 6;
	gameplay->currentLevelIndex = 0;
	gameplay->levels = (LevelData*)Memory::Allocate(gameData->arena_levels, sizeof(LevelData) * gameplay->levelCount);

	gameplay->input_buffer_capacity = 50;
	size_t RING_BUFFER_SIZE = sizeof(Position) * gameplay->input_buffer_capacity;
	gameplay->input_buffer = (Position*)Memory::Allocate(gameData->arena_levels, RING_BUFFER_SIZE);

	gameData->arena_scratch = Memory::CreateSubArena(arena_main, KILOBYTES(256));



	MMRESULT result = timeBeginPeriod(1);
		if(result == TIMERR_NOCANDO)
		{
			printf("could not increase timer resolution");
			Sleep(5000);
			return 3;
		}


	DLL_INFO dll; // Funktionspekare och metadata för den laddade spel-DLL:en.
	bool dll_successfully_loaded = LoadDLL(&dll); // Anger om DLL:en och dess exporter kunde laddas.

	if (dll_successfully_loaded == false)
	{
		return 2;
	}


	dll.initialize(gameData, window, renderer);

	gameData->running = true; // Styr huvudloopens livslängd. Kontrolleras i gamestate
	float dt; // Delta time för den aktuella bildrutan i sekunder.
	float dt_scaler = 1;
	gameData->dt = &dt;
	gameData->dt_scaler = &dt_scaler;



	///////////// HÄR ÄR GAMELOOPEN ///////////////////////

	while (gameData->running)
	{
		DLL_CheckStatus(&dll);

		Reset(gameData->arena_scratch);

		CalculateDeltaTime(&dt, dt_scaler);

		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			gameData->running = dll.handleEvents(gameData, event);
			if (gameData->running == false)
			{
				break;
			}


			// Save/load funktion till/från vår memory arena
			if(event.type == SDL_EVENT_KEY_DOWN)
			{
				if(event.key.key == SDLK_F9)
				{
					StoreGameState(arena_main);
				}
				if(event.key.key == SDLK_F10)
				{
					RetrieveGameState(arena_main);
				}
			}
		}

		//// Hämta input från keys + mus
		gameData->input.keys_current = SDL_GetKeyboardState(nullptr);

		float* delta_x = &gameData->input.mouse_x_delta;
		float* delta_y = &gameData->input.mouse_y_delta;

		*delta_x = gameData->input.mouse_x;
		*delta_y = gameData->input.mouse_y;


		gameData->input.mouse_current = SDL_GetMouseState(&gameData->input.mouse_x, &gameData->input.mouse_y);

		*delta_x = gameData->input.mouse_x - *delta_x;
		*delta_y = gameData->input.mouse_y - *delta_y;

		float dx = *delta_x;
		float dy = *delta_y;
		gameData->input.mouse_magnitude = std::sqrt(dx * dx + dy * dy);

		// Skriver ut var musen är på skärmen, kan kommenteras in vid behov
		// printf("x: %.4f y: %.4f \n", *delta_x, *delta_y);
		// printf("%.4f \n", gameData->input.mouse_magnitude);


		dll.update(gameData, dt);

		UpdateKeys(&gameData->input, dt);
		UpdateMouse(&gameData->input, dt);

		dll.draw(gameData, renderer);

		double time_to_sleep_ms;
		CalculateRemainingFrameTime_MS(&time_to_sleep_ms);

		if (time_to_sleep_ms > 0)
		{
			if(time_to_sleep_ms > 1)
			{
				SDL_Delay(time_to_sleep_ms - 1);
			}

			while (time_to_sleep_ms > 0)
			{
				CalculateRemainingFrameTime_MS(&time_to_sleep_ms);
			}
		}
		else
		{
			printf ("missed frame \n");
		}

	}


dll.quit(renderer);
SDL_Quit();
return 0;

}

