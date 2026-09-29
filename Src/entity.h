#pragma once

#include <cstdint> // Ger heltalstyper med bestämd storlek, exempelvis uint32_t.
#include <cassert> // assert används för att kontrollera giltiga riktningar.

struct LevelData;     // Framåtdeklaration: används bara via pekare i funktionssignaturer.
struct CommandBuffer; // Framåtdeklaration: behövs för kommandon kopplade till entiteter.
struct Gameplay;

// Behaviour (beteende) bestämmer vad en sak i spelet kan göra eller hur den reagerar.
// Det fungerar som små inställningar man kan slå på eller av.
enum Behaviour : uint32_t
{
	NONE = 0,
	CAN_MOVE = 1 << 0,         // Kan den här saken flytta på sig?
	IS_PLAYER = 1 << 1,        // Är det här spelaren?
	RESPOND_TO_INPUT = 1 << 2, // Ska den här lyssna på när man trycker på knappar?
	IS_PETRIFIED = 1 << 3,     // Är den förstenad? (Då kan den inte göra något)
	CAN_ROTATE = 1 << 4,       // Kan den rotera/vända sig om?
	UNPUSHABLE = 1 << 5,       // Går det att knuffa på den här saken?
	JUMPS = 1 << 6,            // Gör den ett hopp när den rör sig?
	IS_PUSHING = 1 << 7,        // Håller den på att knuffa något just nu?
	FOLLOWS_PLAYER = 1 << 8    // Försöker entiteten följa efter spelaren? 
};


// Actions beskriver vad en entitet gör precis just nu.
enum class Actions
{
	NONE = 0, 
	MOVING = 1,   // Den är mitt i en rörelse.
	ROTATING = 2  // Den håller på att vända sig.
};


// Vilket håll tittar entiteten åt?
enum class Direction
{
	RIGHT,
	LEFT,
	UP,
	DOWN,
};

// En hjälpfunktion för att räkna ut riktning baserat på X och Y-värden.
inline Direction DirectionFromXY(int xDir, int yDir)
{
	assert(xDir * yDir == 0);

	if(xDir == 1) {return Direction::RIGHT; }
	if(xDir == -1){return Direction::LEFT;  }
	if(yDir == 1) {return Direction::DOWN;    }
	else 		  {return Direction::UP;  }
}

// Vilken typ av figur eller objekt är det här?
enum class ENTITY_ID : uint8_t
{
	MEDUSA = 0,
	DEMON = 1,
	ROCK = 2,
	SIREN = 3,
	GOLEM = 4,
	ENEMY = 5,
};

// En enkel punkt med en position (X för sida till sida, Y för upp och ner).
struct Position
{
	int x; // Horisontell position.
	int y; // Vertikal position.
};

// Detta är själva "saken" i spelet. Det kan vara spelaren, ett monster eller en sten.
struct Entity
{
	Actions action; // Vad gör den just nu?
	ENTITY_ID id; // Vilken sorts figur är det?
	bool active; // Är den här saken aktiv i spelet just nu?

	Direction facing_current;    // Hållet den tittar åt nu.
	Direction facing_previous; // Hållet den tittade åt precis innan.

	int strength; // Hur stark den är (hur många saker den kan knuffa samtidigt).
	int x;        // Var den står i sidled (i rutnätet).
	int y;        // Var den står i höjdled (i rutnätet).

	///Variabler för animationer (glidande rörelser)
	int x_prev;          // Var den stod innan den flyttade sig.
	int y_prev;          // Var den stod innan den flyttade sig.
	float progress_01;   // Hur långt den har kommit i sin glid-animation (0 till 1).

	Behaviour behaviour; // Vilka egenskaper har den här figuren?

};

// Hämtar den figur som spelaren styr just nu.
Entity* GetActiveEntity(Gameplay* gameplay);

/// Kollar om figuren håller på att flytta på sig.
bool IsMoving(Entity* e); 

// Kollar om en figur har en viss egenskap (t.ex. om den är en spelare).
bool HasBehaviour(Entity* entity, Behaviour flags); 
// Ger en figur sina grundinställningar baserat på vad den är för sort.
void InitializeBaseBehaviour(Entity* entity); 
// Byter ut alla egenskaper på en gång.
void SetBehaviour(Entity* entity, Behaviour flags); 
// Lägger till en ny egenskap.
void AddBehaviour(Entity* entity, Behaviour flags); 
// Tar bort en egenskap.
void RemoveBehaviour(Entity* entity, Behaviour flags); 


// Händer precis efter att en figur har flyttat på sig.
void PostMove(Entity* entity, LevelData* level, CommandBuffer* commandBuffer); 
// Händer efter att en figur har vänt sig om.
void PostRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to); 
// Händer precis innan en figur vänder sig om.
void PreRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to); 

// Kollar om figuren håller på med någon form av aktivitet (rör sig eller vänder sig).
bool IsActing(Entity* e); 
