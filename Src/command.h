#pragma once

#include <cstdint> // uint8_t och uint32_t används för kommandotyp och tidsstämpel.
#include "entity.h" // Entity, Direction och Behaviour används av kommandostrukturerna.
#include "levels.h" // LevelData skickas till undo-, redo- och push-funktionerna.

// Olika typer av kommandon som spelet kan förstå och komma ihåg (för att kunna ångra dem).
enum class CMD_TYPE : uint8_t
{
	NONE = 0,
	MOVE = 1,            // Flytta på sig.
	ROTATE = 2,          // Vända på sig.
	MODIFY_BEHAVIOUR = 3, // Ändra en egenskap (t.ex. bli förstenad).
	ADD = 4,             // Skapa en ny sak i spelet.
	REMOVE = 5,          // Ta bort en sak från spelet.
	SWAP_ACTIVE = 6      // Byta vilken figur spelaren styr.
};

// Grunden för alla kommandon.
struct Command
{
	CMD_TYPE type = CMD_TYPE::NONE; // Vad för sorts kommando är det här?
	uint32_t timestamp;             // När hände det? (Används för att ångra flera saker samtidigt som hör ihop).
};

// Ett kommando för att flytta en figur.
struct MoveCommand : Command
{
	Entity* entity; // Vem ska flytta på sig?
	int xDir;       // Åt vilket håll i sidled?
	int yDir;       // Åt vilket håll i höjdled?

	MoveCommand(Entity* entity, int xDir, int yDir)
	{
		this->entity = entity;
		this->xDir = xDir;
		this->yDir = yDir;
		type = CMD_TYPE::MOVE;
	}
};

// Ett kommando för att vända en figur åt ett annat håll.
struct RotateCommand : Command
{
	Entity* entity; // Vem ska vända på sig?
	Direction from; // Vilket håll tittade den åt innan? (Viktigt för att kunna ångra!)
	Direction to;   // Vilket håll ska den titta åt nu?

	RotateCommand(Entity* entity, Direction from, Direction to)
	{
		this->entity = entity;
		this->from = from;
		this->to = to;
		type = CMD_TYPE::ROTATE;
	}
};

// Ett kommando för att ändra en figurs egenskaper mitt i spelet.
struct ModifyBehaviourCommand : Command
{
	enum Mode 
	{
		ADD,    // Lägg till egenskap.
		REMOVE  // Ta bort egenskap.
	};

	Entity* entity; // Vem gäller det?
	Behaviour flag; // Vilken egenskap ska ändras?
	Mode mode;      // Ska den läggas till eller tas bort?
	ModifyBehaviourCommand(Entity* entity, Behaviour flag, Mode mode)
	{
		this->entity = entity;
		this->flag = flag;
		this->mode = mode;
		type = CMD_TYPE::MODIFY_BEHAVIOUR;
	}
};

// Ett kommando för att placera ut en ny sak på spelplanen.
struct AddCommand : Command 
{
	int x; 
	int y;
	ENTITY_ID id;

	AddCommand(int x, int y, ENTITY_ID id)
	{
		this->x = x;
		this->y = y;
		this->id = id;
		type = CMD_TYPE::ADD;
	}
};

// Ett kommando för att ta bort något från spelplanen.
struct RemoveCommand : Command
{
	int x; 
	int y;
	Behaviour storedBehaviour;
	ENTITY_ID storedID;

	RemoveCommand(Entity* entity)
	{
		x = entity->x;
		y = entity->y;
		storedBehaviour = entity->behaviour;
		storedID = entity->id;
		type = CMD_TYPE::REMOVE;
	}
};

// Ett kommando för att byta vilken figur du kontrollerar just nu.
struct SwapActiveEntityCommand : Command
{
	int index_current;
	int index_previous;
	int* value_to_change;

	SwapActiveEntityCommand(int* activeEntityIndex, int limit)
	{
		index_previous = *activeEntityIndex;
		index_current = *activeEntityIndex + 1;
		value_to_change = activeEntityIndex;
		index_current %= limit;
		type = CMD_TYPE::SWAP_ACTIVE;
	}
};

// En behållare som kan innehålla vilken som helst av ovanstående kommandon.
union AnyCommand
{
	Command command;
	MoveCommand move;
	RotateCommand rotate;
	ModifyBehaviourCommand modify;
	AddCommand add;
	RemoveCommand remove;
	SwapActiveEntityCommand swap_active;

	AnyCommand(MoveCommand mv)
	{
		move = mv;
	};

	AnyCommand(RotateCommand rot)
	{
		rotate = rot;
	};

	AnyCommand(ModifyBehaviourCommand mod)
	{
		modify = mod;
	}

	AnyCommand(AddCommand add)
	{
		this->add = add;
	}

	AnyCommand(RemoveCommand rem)
	{
		remove = rem;
	}

	AnyCommand(SwapActiveEntityCommand swa)
	{
		swap_active = swa;
	}
};

// CommandBuffer fungerar som spelets "minne" för allt som har hänt. 
// Det är detta som gör att "Ångra"-knappen fungerar.
struct CommandBuffer
{
	AnyCommand* allCommands; // En lång lista med allt som har hänt.
	int capacity;            // Hur många saker vi kan komma ihåg totalt.
	int index;               // Var vi är i listan just nu (om vi har ångrat några steg).
	int head;                // Den senaste saken som hände.
	uint32_t timestamp;      // En klocka som tickar upp varje gång något nytt händer.
};



void Push(CommandBuffer* buffer, AnyCommand cmd, LevelData* level); // Sparar ner en ny händelse i minnet och utför den.
void Undo(CommandBuffer* buffer, LevelData* level); // Backar bandet och ångrar det senaste som hände.
void Redo(CommandBuffer* buffer, LevelData* level); // Gör om en händelse som man precis har ångrat.
void ResetCommandBuffer(CommandBuffer* buffer); // Tömmer minnet på händelser (används när man byter bana).
