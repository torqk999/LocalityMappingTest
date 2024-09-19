#pragma once

#include "time.h"
#include "stdio.h"

#include "Entity.h"
#include "Utils.h"

void GridSpace_initializeMemory(GridSpace* gridSpace);

GridSpace GridSpace_ctor(
	char* name,
	HBRUSH backGround,
	int dimCount,
	int* dimCellCounts,
	int maxEntityCount,
	int cellSize,
	size_t dataSize);

void GridSpace_dtor(GridSpace* gridSpace);

void GridSpace_swapEntities(GridSpace* gridSpace, int indexA, int indexB);

void GridSpace_randomPopulate(GridPortfolio* portfolio, int entityCount);

void GridSpace_randomSpawn(GridPortfolio* portfolio);

void GridSpace_updateEntity(Entity* entity, GravityWell* gravityWell);

Entity* GridSpace_getEntityByIndex(GridSpace* gridSpace, int index);

inline int GridSpace_getDimIndexOfEntityPosition(Entity* entity, int dim);

int GridSpace_getCellIndexByIndices(GridSpace* gridSpace, int* indices);

Cell* GridSpace_getCellByIndex(GridSpace* gridSpace, int index);

Cell* GridSpace_getCellByIndices(GridSpace* gridSpace, int* indices);

int GridSpace_getCellIndexOfEntity(Entity* entity);

void GridSpace_testUpdate(
	GridSpace* gridSpace,
	GridPortfolio* portfolio,
	GravityWell* gravityWell
);

inline void GridSpace_getRectFromEntity(Entity* entity, RECT* rect) {
	BasicEntityData* data = Entity_getDataLoc(entity);
	int hx			= data->sx / 2;
	int hy			= data->sy / 2;
	rect->left		= data->px - hx;
	rect->top		= data->py - hy;
	rect->right		= data->px + hx;
	rect->bottom	= data->py + hy;
}

inline void GridSpace_getSolidRectFromEntity(Entity* entity, SolidColorRect* rect) {
	GridSpace_getRectFromEntity(entity, &rect->rect);
	BasicEntityData* data = Entity_getDataLoc(entity);
	rect->brush = entity->info->color;
}

inline void GridSpace_getSolidRectFromCell(GridSpace* gridSpace, SolidColorRect* rect, int index) {
	rect->brush = CreateSolidBrush(RGB(0, 0, gridSpace->meta[index].count * 5));//gridSpace->backGround;
	int xDim = index / gridSpace->dimCellWeights[0];
	index %= gridSpace->dimCellWeights[0];
	int yDim = index;
	rect->rect.left = gridSpace->cellSize * xDim;
	rect->rect.top = gridSpace->cellSize * yDim;
	rect->rect.right = gridSpace->cellSize * (xDim + 1);
	rect->rect.bottom = gridSpace->cellSize * (yDim + 1);
}

inline void GridSpace_updateCellIndexOfEntity(Entity* entity) {
	entity->cellIndex = GridSpace_getCellIndexOfEntity(entity);
}