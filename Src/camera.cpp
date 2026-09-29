#include "camera.h" // Funktionsdeklarationer och Camera-typen som implementeras här.
#include "common.h" // Skärm- och cellstorlekar används i koordinatomvandlingarna.

bool camera::GetIsPointInsideGrid(float x, float y, const LevelData *lvl)
{
	int x_grid; // Beräknad kolumn i rutnätet.
	int y_grid; // Beräknad rad i rutnätet.
	WorldToGrid(x, y, &x_grid, &y_grid, lvl);

	return x_grid >= 0 && y_grid >= 0 && x_grid < lvl->w && y_grid < lvl->h;
}

void camera::GridToWorld(float* x, float* y, const LevelData* lvl)
{
	*x *= TILE_SIZE_PX_SCALED; // Omvandlar cellens x-värde till pixlar.
	*x += TILE_SIZE_PX_SCALED;
	*x += SCREEN_WIDTH / 2.0;
	*x -= lvl->w * TILE_SIZE_PX_SCALED / 2.0;
	*y *= TILE_SIZE_PX_SCALED; // Omvandlar cellens y-värde till pixlar.
	*y += TILE_SIZE_PX_SCALED;
	*y += SCREEN_HEIGHT / 2.0;
	*y -= lvl->h * TILE_SIZE_PX_SCALED / 2.0;
}

void camera::WorldToGrid(float x_world, float y_world, int *x, int *y, const LevelData *lvl)
{
	*x = x_world;
	*y = y_world;
	*x += lvl->w * TILE_SIZE_PX_SCALED / 2.0;
	*x -= SCREEN_WIDTH / 2.0;
	*x /= TILE_SIZE_PX_SCALED;
	*y += lvl->h * TILE_SIZE_PX_SCALED / 2.0;
	*y -= SCREEN_HEIGHT / 2.0;
	*y /= TILE_SIZE_PX_SCALED;
}
