#pragma once

struct Entity;
struct LevelData;

// Returnerar sant om fienden hittar en väg mot spelare och fyller i nextX och nextY
bool FindNextStepAStar(Entity* enemy, Entity* target, LevelData* level, int* nextX, int* nextY);