#include "pathfinding.h"
#include "entity.h"
#include "levels.h"
#include <vector>
#include <cmath>
#include <algorithm>

struct Node
{
    int x, y;
    int gCost; 
    int hCost; 
    int parentIndex; // Index till föräldern i closedList

    int fCost() const { return gCost + hCost; }
};

bool FindNextStepAStar(Entity *enemy, Entity *target, LevelData *level, int *nextX, int *nextY)
{
    int startX = enemy->x;
    int startY = enemy->y;
    int targetX = target->x;
    int targetY = target->y;

    if (startX == targetX && startY == targetY) return false;

    std::vector<Node> openList;
    std::vector<Node> closedList;

    // Startnoden har ingen förälder, så vi sätter -1
    openList.push_back({startX, startY, 0, std::abs(targetX - startX) + std::abs(targetY - startY), -1});

    while (!openList.empty()) 
    {
        auto currentIt = std::min_element(openList.begin(), openList.end(), [](const Node& a, const Node& b) {
            return a.fCost() < b.fCost();
        });

        Node current = *currentIt;
        openList.erase(currentIt);
        closedList.push_back(current);

        // Om vi nått målet
        if (current.x == targetX && current.y == targetY)
        {
            int currentIndex = (int)closedList.size() - 1;
            
            // Backa genom parentIndex tills vi hittar noden precis efter startpunkten
            while (closedList[currentIndex].parentIndex != -1)
            {
                int pIdx = closedList[currentIndex].parentIndex;
                if (closedList[pIdx].parentIndex == -1) // Är nästa steg startpunkten?
                {
                    *nextX = closedList[currentIndex].x;
                    *nextY = closedList[currentIndex].y;
                    return true;
                }
                currentIndex = pIdx;
            }
            return false;
        }

        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};

        for (int i = 0; i < 4; i++)
        {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            if (nx < 0 || ny < 0 || nx >= level->w || ny >= level->h) continue;

            bool walkable = IsWalkable(nx, ny, level) && 
                            (GetEntity(level, nx, ny) == nullptr || (nx == targetX && ny == targetY));

            if (!walkable) continue;

            if (std::any_of(closedList.begin(), closedList.end(), [&](const Node& n) {
                return n.x == nx && n.y == ny;
            })) continue;

            int newGCost = current.gCost + 1;
          
            Node newNode = {nx, ny, newGCost, std::abs(targetX - nx) + std::abs(targetY - ny), (int)closedList.size() - 1};

            auto openIt = std::find_if(openList.begin(), openList.end(), [&](const Node& n) {
                return n.x == nx && n.y == ny;
            });

            if (openIt == openList.end()) 
            {
                openList.push_back(newNode);
            } 
            else if (newGCost < openIt->gCost) 
            {
                openIt->gCost = newGCost;
                openIt->parentIndex = newNode.parentIndex;
            }
        }
    }
    return false;
}