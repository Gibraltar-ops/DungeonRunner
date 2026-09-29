#pragma once

#include "levels.h" // LevelData behövs för rutnätets dimensioner.

struct Camera
{
	float camera_x; // Kamerans horisontella förskjutning i världspixlar.
	float camera_y; // Kamerans vertikala förskjutning i världspixlar.
};

namespace camera
{
	///Specifies a position on the game board and get back the actual position in the game window
void GridToWorld(float* x, float* y, const LevelData* lvl); // Översätter en rutposition till en position i spelfönstret.

	///Takes any point in space and finds what cell tihs belongs to on the game board
void WorldToGrid(float x_world, float y_world, int* x, int* y, const LevelData* lvl); // Översätter mus-/världsposition till en rutcell.

	///Bool check to see if point is inside grid
bool GetIsPointInsideGrid(float x, float y, const LevelData* lvl); // Returnerar om punkten hamnar inom nivåns rutnät.
};
