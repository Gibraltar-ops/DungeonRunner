#include "entity.h" // Entitetsstrukturer, beteendeflaggor och deklarerade funktioner.
#include "command.h" // Följdreaktioner sparas som undo-/redo-kommandon.
#include "levels.h" // Raycast använder nivåns celler och entitetslista.
#include "gameState.h"


// Kollar om en entitet agerar på ett eller annat sätt 
bool IsActing(Entity* e)
{
	// loopar över varje entitet och kollar hur enum Actions ser ut, check för att se hur snabbt progress_01 skall röra sig framåt
	if(e->active == false) return false;


	return e->action != Actions::NONE;
}

// Jämför nuvarande och föregående position för att se om en animation pågår.
bool IsMoving(Entity* e)
{
	return e->x != e->x_prev || e->y != e->y_prev;
}

// Kontrollerar att varje begärd beteendeflagga är aktiv på entiteten.
bool HasBehaviour(Entity* entity, Behaviour flags)
{
	return (entity->behaviour & flags) == flags;
}

// Sätter startbeteenden och styrka baserat på entitetens objekt-ID.
void InitializeBaseBehaviour(Entity* entity)
{
	assert(entity->active);
	switch (entity->id)
	{
	default:
		SetBehaviour(entity, NONE);
		break;

	case ENTITY_ID::DEMON:
		SetBehaviour(entity, (Behaviour)(CAN_ROTATE | CAN_MOVE | IS_PLAYER | RESPOND_TO_INPUT));
		entity->strength = 1;
		break;

	case ENTITY_ID::GOLEM:
		SetBehaviour(entity, (Behaviour)CAN_MOVE);
		AddBehaviour(entity, Behaviour::UNPUSHABLE);
		entity->strength = 1;
		break;

	case ENTITY_ID::MEDUSA:
		SetBehaviour(entity, (Behaviour)(CAN_ROTATE | CAN_MOVE | IS_PLAYER | RESPOND_TO_INPUT));
		AddBehaviour(entity, Behaviour::JUMPS);
		entity->strength = 1;
		break;

	case ENTITY_ID::SIREN:
		SetBehaviour(entity, (Behaviour)(CAN_MOVE | IS_PLAYER | RESPOND_TO_INPUT));
		entity->strength = 0;
		break;


	case ENTITY_ID::ROCK:
		SetBehaviour(entity, (Behaviour)CAN_MOVE);
		break;

	case ENTITY_ID::ENEMY:
		SetBehaviour(entity, (Behaviour)(CAN_MOVE | FOLLOWS_PLAYER));
		entity->strength = 1;
		break;
	}
}

// Ersätter entitetens hela uppsättning beteendeflaggor.
void SetBehaviour(Entity* entity, Behaviour flags)
{
	entity->behaviour = flags;
}

// Ändrar entitetens beteendeflaggor enligt den angivna masken.
void AddBehaviour(Entity *entity, Behaviour flags)
{
	entity->behaviour = (Behaviour)(entity->behaviour | flags);
}

// Tar bort de flaggor som anges i masken från entiteten.
void RemoveBehaviour(Entity* entity, Behaviour flags)
{
	entity->behaviour = (Behaviour)(entity->behaviour & ~flags);
}




// Kör objektspecifika effekter efter en förflyttning, till exempel Medusas blick.
void PostMove(Entity* entity, LevelData* level, CommandBuffer* commandBuffer)
{
	if(entity->id == ENTITY_ID::MEDUSA)
	{
		Entity* entity_looked_at = RaycastFirstEntity(entity->x, entity->y, entity->facing_current, level); // Första synliga entiteten framför Medusa.
			if(entity_looked_at != nullptr)
			{
				if (!HasBehaviour(entity_looked_at, Behaviour::IS_PETRIFIED))
				{
					ModifyBehaviourCommand modify(entity_looked_at, Behaviour::IS_PETRIFIED, ModifyBehaviourCommand::ADD);
					Push(commandBuffer, modify, level);
				}
			}
	}
}

// Kör effekter efter en rotation i den nya riktningen.
void PostRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to)
{
	if(from == to)
	{
		return;
	}

	if(entity->id == ENTITY_ID::MEDUSA)
	{
		Entity* entity_looked_at = RaycastFirstEntity(entity->x, entity->y, to, level);
		
			if(entity_looked_at != nullptr)
			{
				if(!HasBehaviour(entity_looked_at, Behaviour::IS_PETRIFIED))
				{
					ModifyBehaviourCommand modify(entity_looked_at, Behaviour::IS_PETRIFIED, ModifyBehaviourCommand::ADD);
					Push(commandBuffer, modify, level);
				}
			}
	}

}

// Tar bort effekter från den gamla riktningen innan entiteten roterar.
void PreRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to)
{
	if (from == to)
	{
		return;
	}

	if(entity->id == ENTITY_ID::MEDUSA)
	{
		Entity* entity_previously_looked_at = RaycastFirstEntity(entity->x, entity->y, from, level);

		if (entity_previously_looked_at != nullptr)
		{
			if(HasBehaviour(entity_previously_looked_at, Behaviour::IS_PETRIFIED))
			{
				ModifyBehaviourCommand modify(entity_previously_looked_at, Behaviour::IS_PETRIFIED, ModifyBehaviourCommand::REMOVE);
				Push(commandBuffer, modify, level);
			}
		}
	}
}
