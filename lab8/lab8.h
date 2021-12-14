#pragma once

#include "resource.h"
#include <vector>
struct Tile
{
	Tile()
	{
		wasShooted = false;
	}
	bool wasShooted;
};
struct ShipTile : Tile
{
	ShipTile(int x, int y) : Tile(), x(x), y(y)
	{}
	int x;
	int y;
};
struct Ship
{
	Ship(POINT start, POINT end, int size) 
		: size(size), isDefeated(false)
	{
		int offX = (end.x - start.x + 1) / size;
		int offY = (end.y - start.y + 1) / size;
		for (int i = 0; i < size; i++)
		{
			int x = start.x + offX * i;
			int y = start.y + offY * i;
			shipInfo.push_back(ShipTile(x, y));
		}
	}
	std::vector<ShipTile> shipInfo;
	int size;
	bool isDefeated;
};
class GameField
{
public:
	GameField(int xSize, int ySize)
	{
		for (int i = 0; i < xSize * ySize; i++)
		{
			tilesArray.push_back(Tile());
		}
		this->xSize = xSize;
		this->ySize = ySize;
	}
	void SetShip(Ship newShip)
	{
		shipsArray.push_back(newShip);
	}
	Tile& getTile(int x, int y)
	{
		return tilesArray.at(x + y * xSize);
	}
	void shoot(int x, int y)
	{
		tilesArray.at(x + y * xSize).wasShooted = true;
	}
	int xSize;
	int ySize;
	std::vector<Tile> tilesArray;
	std::vector<Ship> shipsArray;
};
