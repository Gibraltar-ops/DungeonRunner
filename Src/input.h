#pragma once

#include <cstdint>
#include "SDL3/SDL_scancode.h" // SDL_Scancode och antal tangenter används för tangentinput.
#include "SDL3/SDL_mouse.h" // SDL:s musknappsflaggor används för musinput.

// Samlar all information om vad spelaren gör med tangentbordet och musen.
struct Input
{
	const bool* keys_current; // Vilka tangenter som är nedtryckta just nu.
	const bool* keys_previous;// Vilka tangenter som var nedtryckta förra gången vi kollade.
	float* keys_held_time;    // Hur länge varje tangent har hållits nedtryckt.

	SDL_MouseButtonFlags mouse_current; // Vilka musknappar som är nedtryckta nu.
	SDL_MouseButtonFlags mouse_previous;// Vilka musknappar som var nedtryckta förra gången.

	float* mouse_held_time; // Hur länge varje musknapp har hållits nedtryckt.
	float mouse_x;          // Var muspekaren är på skärmen i sidled (X).
	float mouse_y;          // Var muspekaren är på skärmen i höjdled (Y).

	float mouse_x_delta;    // Hur mycket musen har flyttats i sidled sedan sist.
	float mouse_y_delta;    // Hur mycket musen har flyttats i höjdled sedan sist.
	double mouse_magnitude; // Den totala "kraften" eller hastigheten i musrörelsen.
};

// Enkel lista för att hålla koll på musknapparna.
enum class MouseButtons
{
	LEFT = 0,   // Vänsterklick.
	MIDDLE = 1, // Mittenklick (rullhjulet).
	RIGHT = 2,  // Högerklick.
};

bool MousePressed(const Input* input, MouseButtons button); // Kollar om man precis tryckte ner en musknapp.
bool MouseReleased(const Input* input, MouseButtons button); // Kollar om man precis släppte upp en musknapp.
bool MouseHeld_ForTime(const Input* input, MouseButtons button, float min_length); // Kollar om en musknapp hållits ner en viss tid.
void UpdateMouse(Input* input, float dt); // Håller mus-informationen uppdaterad.

bool AnyKeyPressed(const Input* input); // Kollar om man trycker på NÅGON tangent överhuvudtaget. 
bool KeyPressed(const Input* input, SDL_Scancode key); // Kollar om man precis tryckte ner en specifik tangent.
bool KeyHeld(const Input* input, SDL_Scancode key); // Kollar om man håller ner en specifik tangent.
bool KeyReleased(const Input* input, SDL_Scancode key); // Kollar om man precis släppte en specifik tangent.
bool KeyHeld_ForTime(const Input* input, SDL_Scancode key, float min_length); // Kollar om en tangent hållits ner en viss tid.
void UpdateKeys(Input* input, float dt); // Håller tangentbords-informationen uppdaterad.
void ResetKeyHeldTime(Input* input, SDL_Scancode key); // Nollställer tidtagningen för en tangent.
void ResetAll(Input*); // Nollställer all inmatningsdata.
