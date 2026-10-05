#include <cassert>

#include "pauseMenu.h"
#include "SDL3/SDL_scancode.h"
#include "arena.h"
#include "button.h"
#include "common.h"
#include "game.h"
#include "gameState.h"
#include "input.h"
#include "rendering.h"
#include "spriteLibrary.h"


void InitializePauseMenu(
    PauseMenu* menu,
    Sprite* spriteBuffer,
    FontAtlas* font,
    Memory::Arena* arena_main)
{
    assert(menu->initialized == false);

    menu->button_count = 6;
    menu->buttons = ALLOC_ARRAY(arena_main, Button, menu->button_count);

    SetupButton(
        &menu->buttons[0],
        ButtonType::RETURN_TO_GAME,
        spriteBuffer,
        {
            SCREEN_WIDTH / 2.0,
            (SCREEN_HEIGHT / 2.0) - 250,
            300,
            100
        },
        Alignment::Centered,
        font,
        "Return to Game");

    SetupButton(
        &menu->buttons[1],
        ButtonType::PREVIOUS_LEVEL,
        spriteBuffer,
        {
            (SCREEN_WIDTH / 2.0) - 200,
            (SCREEN_HEIGHT / 2.0) - 100,
            80,
            80
        },
        Alignment::Centered,
        font,
        "<");

    SetupButton(
        &menu->buttons[2],
        ButtonType::SELECT_LEVEL,
        spriteBuffer,
        {
            SCREEN_WIDTH / 2.0,
            (SCREEN_HEIGHT / 2.0) - 100,
            300,
            100
        },
        Alignment::Centered,
        font,
        "Select Level 1");

    SetupButton(
        &menu->buttons[3],
        ButtonType::NEXT_LEVEL,
        spriteBuffer,
        {
            (SCREEN_WIDTH / 2.0) + 200,
            (SCREEN_HEIGHT / 2.0) - 100,
            80,
            80
        },
        Alignment::Centered,
        font,
        ">");

    SetupButton(
        &menu->buttons[4],
        ButtonType::RETURN_TO_MENU,
        spriteBuffer,
        {
            SCREEN_WIDTH / 2.0,
            (SCREEN_HEIGHT / 2.0) + 50,
            300,
            100
        },
        Alignment::Centered,
        font,
        "Return to Menu");

    SetupButton(
        &menu->buttons[5],
        ButtonType::QUIT,
        spriteBuffer,
        {
            SCREEN_WIDTH / 2.0,
            (SCREEN_HEIGHT / 2.0) + 175,
            300,
            100
        },
        Alignment::Centered,
        font,
        "Quit Game");

    for (int i = 0; i < menu->button_count; i++)
    {
        menu->buttons[i].is_dynamic = true;
    }

    menu->activeButtonIndex = 0;
    menu->selectedLevel = 0;
    menu->paused = false;
    menu->startLevel = false;
    menu->initialized = true;
}


void UpdatePauseMenu(GameData* data)
{
    PauseMenu* menu = &data->scenes.pauseMenu;
    Input* input = &data->input;

    menu->activeButtonCount =
        GetActiveButtonCount(menu->buttons, menu->button_count);

    if (menu->activeButtonCount == 0)
    {
        return;
    }

    menu->activeButtons =
        ALLOC_ARRAY(
            data->arena_scratch,
            Button*,
            menu->activeButtonCount);

    int index = 0;

    for (int i = 0; i < menu->button_count; i++)
    {
        Button* button = &menu->buttons[i];

        if (button->is_active)
        {
            menu->activeButtons[index] = button;
            index += 1;
        }
    }

    int* buttonIndex = &menu->activeButtonIndex;

    bool mouseMoving = input->mouse_magnitude > 0.1;

    if (mouseMoving)
    {
        for (int i = 0; i < menu->activeButtonCount; i++)
        {
            Button* button = menu->activeButtons[i];

            if (IsHoveredOver(
                button,
                input->mouse_x,
                input->mouse_y))
            {
                *buttonIndex = i;
                break;
            }
        }
    }

    bool up = KeyPressed(input, SDL_SCANCODE_UP) || KeyPressed(input, SDL_SCANCODE_W);
    bool down = KeyPressed(input, SDL_SCANCODE_DOWN) || KeyPressed(input, SDL_SCANCODE_S);

    if (up || down)
{
    int direction = down ? 1 : -1;

    *buttonIndex += direction + menu->activeButtonCount;
    *buttonIndex = *buttonIndex % menu->activeButtonCount;

    // Här skippar vi VÄNSTER & HÖGER knapp när vi navigerar verticalt.
    if (*buttonIndex == 1)
    {
        *buttonIndex += direction + menu->activeButtonCount;
        *buttonIndex = *buttonIndex % menu->activeButtonCount;
    }

    if (*buttonIndex == 3)
    {
        *buttonIndex += direction + menu->activeButtonCount;
        *buttonIndex = *buttonIndex % menu->activeButtonCount;
    }
}

    Button* selected = menu->activeButtons[*buttonIndex];

    if (selected == &menu->buttons[2])
    {
        bool left = KeyPressed(input, SDL_SCANCODE_LEFT) || KeyPressed(input, SDL_SCANCODE_A);
        bool right = KeyPressed(input, SDL_SCANCODE_RIGHT) || KeyPressed(input, SDL_SCANCODE_D);

        if (left)
        {
            if (menu->selectedLevel > 0)
            {
                menu->selectedLevel--;
            }
        }

        if (right)
        {
            if (menu->selectedLevel <
                data->scenes.gameplay.levelCount - 1)
            {
                menu->selectedLevel++;
            }
        }
    }

    if (selected == nullptr)
    {
        return;
    }

    bool pressed = false;

    if (KeyPressed(input, SDL_SCANCODE_RETURN))
    {
        pressed = true;
    }

    if (IsHoveredOver(
        selected,
        input->mouse_x,
        input->mouse_y))
    {
        if (MousePressed(input, MouseButtons::LEFT))
        {
            pressed = true;
        }
    }

    if (!pressed)
    {
        return;
    }

    switch (selected->type)
    {
    case ButtonType::RETURN_TO_GAME:
        menu->paused = false;
        break;

    case ButtonType::PREVIOUS_LEVEL:
        if (menu->selectedLevel > 0)
        {
            menu->selectedLevel--;
        }
        break;

    case ButtonType::SELECT_LEVEL:
        data->scenes.gameplay.currentLevelIndex =
            menu->selectedLevel;

        menu->startLevel = true;
        menu->paused = false;
        break;

    case ButtonType::NEXT_LEVEL:
        if (menu->selectedLevel <
            data->scenes.gameplay.levelCount - 1)
        {
            menu->selectedLevel++;
        }
        break;

    case ButtonType::RETURN_TO_MENU:
        menu->paused = false;
        ChangeScene(data, SCENE_TYPES::MAINMENU);
        break;

    case ButtonType::QUIT:
        data->running = false;
        break;

    default:
        break;
    }
}


void DrawPauseMenu(
    GameData* data,
    SDL_Renderer* renderer)
{
    PauseMenu* menu = &data->scenes.pauseMenu;

    // Darken the current level.
    RenderSprite_World(
        GetSprite(SPRITE_ID::black_1x1, data->spriteBuffer),
        renderer,
        &data->camera,
        0,
        0,
        SCREEN_WIDTH,
        0.5f);

    // Update the selected level button text.
    static const char* levelText[] =
    {
        "Select Level 1",
        "Select Level 2",
        "Select Level 3",
        "Select Level 4",
        "Select Level 5",
        "Select Level 6",
        "Select Level 7",
        "Select Level 8",
        "Select Level 9"
    };

    menu->buttons[2].text =
        levelText[menu->selectedLevel];

    for (int i = 0; i < menu->activeButtonCount; i++)
    {
        Button* button = menu->activeButtons[i];

        if (button->is_dynamic)
        {
            RenderButton_Dynamic(
                button,
                i == menu->activeButtonIndex,
                renderer);
        }
        else
        {
            RenderButton(
                button,
                i == menu->activeButtonIndex,
                renderer);
        }
    }

    menu->activeButtonCount = 0;
}