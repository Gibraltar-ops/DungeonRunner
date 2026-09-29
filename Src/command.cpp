#include <cstdint> // uint32_t används för kommandonas tidsstämplar.
#include "command.h" // Deklarationerna för kommandohistorik och kommandotyper.
#include "entity.h" // Entitetsreaktioner körs vid rörelse och rotation.
#include "levels.h" // LevelData skickas till de reaktioner som ändrar nivån.

enum class FromRedo {No, Yes};

// Utför ett kommando och följdreaktioner.
void Execute(AnyCommand cmd, LevelData* level, CommandBuffer* commandBuffer, FromRedo fromRedo = FromRedo::No)
{
	switch(cmd.command.type)
	{
	case CMD_TYPE::NONE:
		break;

	case CMD_TYPE::MOVE:
	{
		MoveCommand mv = cmd.move; // Rörelsedatan som ska appliceras på entiteten.
		mv.entity->x_prev = mv.entity->x;
		mv.entity->y_prev = mv.entity->y;
		mv.entity->x += mv.xDir;
		mv.entity->y += mv.yDir;

		if(fromRedo == FromRedo::Yes)
		{
			mv.entity->progress_01 = 1;
		}

		mv.entity->action = Actions::MOVING;

		if(fromRedo == FromRedo::No)
		{
			PostMove(mv.entity, level, commandBuffer);
		}

		break;
	}

	case CMD_TYPE::ROTATE:
		{
			RotateCommand* rotate = &cmd.rotate; // Rotationsdatan som ska appliceras.
			
			if (!HasBehaviour(rotate->entity, CAN_ROTATE))
			{
				break;
			}

			if (fromRedo == FromRedo::Yes)
			{
				rotate->entity->progress_01 = 1;
			}

			rotate->entity->action = Actions::ROTATING;

			if(fromRedo == FromRedo::No)
			{
				PreRotation(rotate->entity, level, commandBuffer, rotate->from, rotate->to);
			}

			rotate->entity->facing_previous = rotate->from;
			rotate->entity->facing_current = rotate->to;

			if(fromRedo == FromRedo::No)
			{
				PostRotation(rotate->entity, level, commandBuffer, rotate->from, rotate->to);
			}
		
		break;
		
		}


	case CMD_TYPE::MODIFY_BEHAVIOUR:
		{
			ModifyBehaviourCommand modify = cmd.modify; // Flaggan och åtgärden som ska ändra entiteten.
			if(modify.mode == ModifyBehaviourCommand::ADD)
			{
				AddBehaviour(modify.entity, modify.flag);
			}
			else
			{
				RemoveBehaviour(modify.entity, modify.flag);
			}
			break;
		}

	case CMD_TYPE::ADD:
		{
			AddCommand* add = &cmd.add;
			AddEntity(add->id, add->x, add->y, level);
			break;

		}

	case CMD_TYPE::REMOVE:
		{
			RemoveCommand* remove = &cmd.remove;
			RemoveEntity(remove->x, remove->y, level);
			break;
		}

	case CMD_TYPE::SWAP_ACTIVE:
		{
			SwapActiveEntityCommand* swap = &cmd.swap_active;
			*swap->value_to_change = swap->index_previous;
			break;
		}
	}
}

// Sparar ett nytt kommando i historiken och kör det direkt.
void Push(CommandBuffer* buffer, AnyCommand cmd, LevelData* level)
{

	assert(cmd.command.type != CMD_TYPE::NONE);

	buffer->allCommands[buffer->index] = cmd;
	buffer->allCommands[buffer->index].command.timestamp = buffer->timestamp;
	buffer->index++;
	buffer->head = buffer->index;
	Execute(cmd, level, buffer);
}

// Återställer senaste kommandogruppen med samma tidsstämpel.
void Undo(CommandBuffer* buffer, LevelData* level)
{
	if(buffer->index == 0)
	{
		return;
	}

	buffer->index--;

	AnyCommand cmd = buffer->allCommands[buffer->index]; // Kommandot som ska återställas.
	uint32_t timestamp = cmd.command.timestamp; // Gruppnyckeln för samhörande kommandon.

	switch(cmd.command.type)
	{
	case CMD_TYPE::NONE:
		break;

	case CMD_TYPE::MOVE:
		{
		MoveCommand mv = cmd.move;
		mv.entity->x -= mv.xDir;
		mv.entity->y -= mv.yDir;
		mv.entity->progress_01 = 1;
		}

		break;

	case CMD_TYPE::ROTATE:
		{
			RotateCommand rotate = cmd.rotate;
			if(!HasBehaviour(rotate.entity, CAN_ROTATE))
			{
				break;
			}

		rotate.entity->facing_current = rotate.from;

		break;

		}

	case CMD_TYPE::MODIFY_BEHAVIOUR:
		{
			ModifyBehaviourCommand modify = cmd.modify;
			if(modify.mode == ModifyBehaviourCommand::ADD)
			{
				RemoveBehaviour(modify.entity, modify.flag);
			}
			else
			{
				AddBehaviour(modify.entity, modify.flag);
			}

			break;
		}

	case CMD_TYPE::ADD:
		{
			AddCommand* add = &cmd.add;
			RemoveEntity(add->x, add->y, level);

			break;
		}

	case CMD_TYPE::REMOVE:
		{
			RemoveCommand* remove = &cmd.remove;
			AddEntity(remove->storedID, remove->x, remove->y, level);
			Entity* entity = GetEntity(level, remove->x, remove->y);
			SetBehaviour(entity, remove->storedBehaviour);

			break;
		}

	case CMD_TYPE::SWAP_ACTIVE:
		{
			SwapActiveEntityCommand* swap = &cmd.swap_active;
			*swap->value_to_change = swap->index_previous;
			break;
		}
	}

	if(buffer->index > 0)
	{
		if(buffer->allCommands[buffer->index - 1].command.timestamp == timestamp)
		{
			Undo(buffer, level);
		}
	}

}

// Kör om nästa kommandogrupp med samma tidsstämpel.
void Redo(CommandBuffer* buffer, LevelData* level)
{
	if(buffer->index == buffer->head)
	{
		return;
	}
	AnyCommand cmd = buffer->allCommands[buffer->index]; // Nästa kommando i redo-historiken.
	if(cmd.command.type == CMD_TYPE::NONE)
	{
		return;
	}

	Execute(cmd, level, buffer, FromRedo::Yes);

	buffer->index++;

	uint32_t timestamp = cmd.command.timestamp;

	if(buffer->index != buffer->head)
	{
		AnyCommand nextCommand = buffer->allCommands[buffer->index];
		if(nextCommand.command.timestamp == timestamp)
		{
			Redo(buffer, level);
		}
	}
	
}

void ResetCommandBuffer(CommandBuffer *buffer)
{
	buffer->index = 0;
	buffer->head = 0;
	buffer->timestamp = 0;
}
