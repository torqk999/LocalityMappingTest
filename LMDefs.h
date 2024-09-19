#pragma once
#include "Windows.h"

typedef struct {
	RECT rect;
	HBRUSH brush;
} SolidColorRect;

typedef struct {
	int startIndex;
	int count;
} Cell;

typedef struct {
	const char* name;
	HBRUSH backGround;

	size_t memory;
	size_t entitySize;

	// TODO: abstract these pointers with inline functions....
	Cell* meta;
	int* dimCellCounts;
	int* dimCellWeights;
	//int* dimCellAccumulators;

	int maxEntityCount;
	int entityCount;
	int dimCount;
	int totalCellCount;
	float cellSize;

	
} GridSpace;

typedef enum {
	BASIC
} EntityType;

typedef enum {
	TEAM_A,
	TEAM_B
} Association;

typedef struct {
	float force;
	float* point;
} GravityWell;

typedef struct {
	char*		name;
	GridSpace*	hostGrid;
	//int*		nextId;

	HBRUSH		color;
	Association association;

	int			collisionMask,
				checkRange;
				

	float		collisionDmg,
				maxHealth,
				maxSpeed,
				size;

} Info;

typedef struct {
	int nextUUID;
	int infoCount;
	Info* infoLib;
	int spawnRate;
	int spawnTimer;
} GridPortfolio;

typedef struct {
	float left, top, right, bottom;
}RECTf;

typedef enum {
	ACTIVE,
} EntityFlags;

// Header of an entity memory chunk.
typedef struct {
	int cellIndex;
	int flags;
	int uuid;
	Info* info;
} Entity;

typedef enum {
	POS_X,
	POS_Y,
	DELTA_X,
	DELTA_Y,
	SIZE_X,
	SIZE_Y,
	HEALTH
} BasicDataType;

typedef enum {
	NONE = 0,
	SCENE = 1,
	BONOM = 2,
	PROJECTILE = 4
} COLLISION_MASK;

typedef struct {
	float px, py, dx, dy, sx, sy, health, _pad;
	//HBRUSH brush;
	//SolidColorRect rect;
} BasicEntityData;

typedef struct {
	HWND hwnd;
	HDC hdc;
	HDC swap;
	HBITMAP swapMap;
	HGLRC hglrc;
	GridSpace* grid;
}GridRenderContainer;

