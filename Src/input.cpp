#include <cstring> // memcpy och memset används för att kopiera/nollställa inputbuffertar.
#include "input.h" // Input-typen och dess hjälpfunktioner implementeras här.

// Returnerar sant enbart på bildrutan då en tangent går från uppsläppt till nedtryckt.
bool KeyPressed(const Input* input, SDL_Scancode key)
{
	if(input->keys_previous == nullptr)
	{
		return input->keys_current[key];
	}

	return input->keys_current[key] && !input->keys_previous[key];

}

// Loopar över hela tangentbordet och kollar om någon av tangenterna blev tryckte under en frame. Returnerar falskt annars.
bool AnyKeyPressed(const Input *input) 
{
	for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
	{
		if(KeyPressed(input, (SDL_Scancode)i))
		{
			return true;
		}
	}

	return false;

}

// Returnerar sant när en tangent varit nedtryckt minst två bildrutor.
bool KeyHeld (const Input* input, SDL_Scancode key)
{
	if(input->keys_previous == nullptr)
	{
		return false;
	}

	return input->keys_current[key] && input->keys_previous[key];

}

// Returnerar sant enbart på bildrutan då en tangent släpps.
bool KeyReleased(const Input* input, SDL_Scancode key)
{
	if(input->keys_previous == nullptr)
	{
		return false;
	}

	return !input->keys_current[key] && input->keys_previous[key];

}

// Kontrollerar om tangentens ackumulerade nedhållningstid når gränsen.
bool KeyHeld_ForTime(const Input* input, SDL_Scancode key, float min_length)
{
	return input->keys_held_time[key] >= min_length;
}


// Uppdaterar nedhållningstid för varje tangent och sparar aktuell status till nästa bildruta.
void UpdateKeys(Input* input, float dt)
{
	for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
	{
		if (input->keys_current[i])
		{
			input->keys_held_time[i] += dt;
		}
		else
		{
			input->keys_held_time[i] = 0;
		}
	}

	memcpy((void*)input->keys_previous, input->keys_current, SDL_SCANCODE_COUNT * sizeof(bool));
}

// Översätter projektets musknappsenum till SDL:s bitflagga.
SDL_MouseButtonFlags ButtonToFlag(MouseButtons button)
{
	switch(button)
	{

	case MouseButtons::LEFT:
			return SDL_BUTTON_LMASK;

	case MouseButtons::MIDDLE:
			return SDL_BUTTON_MMASK;

	case MouseButtons::RIGHT:
			return SDL_BUTTON_RMASK;

		break;		
	}
}

// Returnerar sant när vald musknapp precis har tryckts ned.
bool MousePressed(const Input* input, MouseButtons button)
{
	SDL_MouseButtonFlags flag = ButtonToFlag(button); // Bitflaggan som motsvarar den efterfrågade knappen.
	return (input->mouse_current & flag) != 0 && (input->mouse_previous & flag) == 0;
}

// Returnerar sant när vald musknapp precis har släppts.
bool MouseReleased(const Input *input, MouseButtons button)
{
	SDL_MouseButtonFlags flag = ButtonToFlag(button);
	return (input->mouse_current & flag) == 0 && (input->mouse_previous & flag) != 0;
}

// Returnerar sant när vald musknapp hålls ned över flera bildrutor.
bool MouseHeld(const Input* input, MouseButtons button)
{
	SDL_MouseButtonFlags flag = ButtonToFlag(button);
	return (input->mouse_current & flag) != 0 && (input->mouse_previous & flag) != 0;
}

// Kontrollerar den sparade nedhållningstiden för vald musknapp.
bool MouseHeld_ForTime (const Input* input, MouseButtons button, float min_length)
{
	SDL_MouseButtonFlags flag = ButtonToFlag(button);
	return input->mouse_held_time[flag] >= min_length;
}

// Nollställer en tangents timer efter att dess upprepade handling har körts.
void ResetKeyHeldTime(Input* input, SDL_Scancode key)
{
	input->keys_held_time[key] = 0;
}

// Nollställer alla tangentrelaterade inputbuffertar.
void ResetAll(Input* input)
{
	memset((void*)input->keys_current, 0, sizeof(bool) * SDL_SCANCODE_COUNT);
	memset((void*)input->keys_previous, 0, sizeof(bool) * SDL_SCANCODE_COUNT);
	memset((void*)input->keys_held_time, 0, sizeof(float) * SDL_SCANCODE_COUNT);
}

// Uppdaterar musknapparnas nedhållningstider och sparar nuvarande knappstatus.
void UpdateMouse(Input* input, float dt)
{
	if(MouseHeld(input, MouseButtons::LEFT))
	{
		input->mouse_held_time[(int)MouseButtons::LEFT] += dt;
	}
	else
		{
			input->mouse_held_time[(int)MouseButtons::LEFT] = 0;
		}

	if(MouseHeld(input, MouseButtons::MIDDLE))
	{
		input->mouse_held_time[(int)MouseButtons::MIDDLE]  += dt;
	}
	else
		{
			input->mouse_held_time[(int)MouseButtons::MIDDLE] = 0;
		}

	if(MouseHeld(input, MouseButtons::RIGHT))
	{
		input->mouse_held_time[(int)MouseButtons::RIGHT] += dt;
	}
	else
		{
			input->mouse_held_time[(int)MouseButtons::RIGHT] = 0;
		}

	input->mouse_previous = input->mouse_current;

}
