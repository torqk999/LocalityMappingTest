#include "GridSpace.h"

size_t clz = 0;

GridSpace GridSpace_ctor(
	const char* name,
	int dimCount,
	int* dimCellCounts,
	int maxEntityCount,
	int cellSize,
	size_t dataSize
) {
	GridSpace newGrid = { 0 };

	newGrid.name = name;
	newGrid.dimCount = dimCount;
	newGrid.cellSize = cellSize;
	newGrid.maxEntityCount = maxEntityCount;
	newGrid.entityCount = 0;

	// bound entities to a cache line
	size_t criticalEntitySize = sizeof(Entity) + dataSize;
	newGrid.entitySize = 0;
	while (newGrid.entitySize < criticalEntitySize)
		newGrid.entitySize += clz;
	
	newGrid.totalCellCount = 1;
	for (int i = dimCount - 1; i >= 0; i--)
		newGrid.totalCellCount *= dimCellCounts[i];


	size_t memory = newGrid.entitySize * maxEntityCount;
	size_t meta = sizeof(Cell) * newGrid.totalCellCount;
	size_t dimCounters = sizeof(int) * dimCount;

	size_t block = _aligned_malloc(memory + meta + (dimCounters * 2), clz);

	newGrid.memory = block;
	block += memory;
	newGrid.meta = block;
	block += meta;
	newGrid.dimCellCounts = block;
	newGrid.dimCellWeights = ((size_t)newGrid.dimCellCounts) + (sizeof(int) * newGrid.dimCount);

	int weight = 1;
	for (int i = dimCount - 1; i >= 0; i--) {
		newGrid.dimCellWeights[i] = weight;
		newGrid.dimCellCounts[i] = dimCellCounts[i];
		weight *= dimCellCounts[i];
	}

	//GridSpace_initializeMemory(&newGrid);

	for (int i = 0; i < newGrid.totalCellCount; i++)
		newGrid.meta[i] = (Cell){ 0,0 };

	newGrid._collisionCheckCount = 0;
	newGrid._collisionCheckCycleMax = 0;

	return newGrid;
}

void GridSpace_init() {
	srand(time(NULL));

	SYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer[256];
	DWORD returnLength = sizeof(buffer);
	if (!GetLogicalProcessorInformation(buffer, &returnLength)) {
		perror("GetLogicalProcessorInformation");
		return 1;
	}

	for (int i = 0; i < returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); i++) {
		if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
			clz = buffer[i].Cache.LineSize;
			//printf("Cache line size: %d bytes\n", buffer[i].Cache.LineSize);
			break;
		}
	}
}

void GridSpace_initializeMemory(GridSpace* gridSpace) {
	for (int i = 0; i < gridSpace->maxEntityCount; i++)
		InitMemory(gridSpace->memory, gridSpace->entitySize);
}

void GridSpace_dtor(GridSpace* gridSpace) {
	free(gridSpace->memory);
	//DeleteObject(gridSpace->backGround);
	gridSpace->memory = NULL;
	gridSpace->meta = NULL;
	gridSpace->dimCellCounts = NULL;
	gridSpace->dimCellWeights = NULL;
	gridSpace->entityCount = 0;
	gridSpace->maxEntityCount = 0;
	gridSpace->totalCellCount = 0;
}

void GridSpace_swapEntities(GridSpace* grid, int indexA, int indexB) {

	if (indexA == indexB)
		return;

	void* locA = grid->memory + (indexA * grid->entitySize);
	void* locB = grid->memory + (indexB * grid->entitySize);

	SwapMemory(locA, locB, grid->entitySize);
}

BasicEntityData GridSpace_generateRandomEntityData(Info* info) {

	//int halfMax = RAND_MAX / 2;

	float wMax = (float)(info->hostGrid->dimCellCounts[0] * info->hostGrid->cellSize);
	float hMax = (float)(info->hostGrid->dimCellCounts[1] * info->hostGrid->cellSize);

	BasicEntityData newData = {
		randf() * wMax,
		randf() * hMax,
		((randf() * 2) - 1) * info->maxSpeed,
		((randf() * 2) - 1) * info->maxSpeed,
		info->size,
		info->size,
		info->maxHealth
	};

	return newData;
}

inline int GridPortfolio_nextUUID(GridPortfolio* portfolio) {
	return portfolio->nextUUID++;
}

void GridSpace_addEntity(Info* info, int uuid, BasicEntityData* init) {

	if (info->hostGrid->entityCount >= info->hostGrid->maxEntityCount)	// No more room for entities
		return;

	// Get the chunk of memory that is the size of this grids entity model.
	Entity* next = GridSpace_getEntityByIndex(info->hostGrid, info->hostGrid->entityCount);

	Entity_ctor(
		next,
		info,
		uuid,
		info->hostGrid->entitySize,
		init
	);

	GridSpace_updateCellIndexOfEntity(next);							// Set the target cell index for the entity after it gets it pos data

	info->hostGrid->meta[info->hostGrid->totalCellCount - 1].count++;	// Add to the last cell and let it resolve via the memory rolling
	info->hostGrid->entityCount++;										// Add to the grid

	//printf("Entity %d added\n", uuid);
}

void GridSpace_randomPopulate(GridPortfolio* portfolio, int entityCount) {

	for (int i = 0; i < entityCount && i < portfolio->infoLib->hostGrid->maxEntityCount; i++) {

		Info* nextInfo = &(portfolio->infoLib)[rand() % portfolio->infoCount];
		BasicEntityData bed = GridSpace_generateRandomEntityData(nextInfo);
		GridSpace_addEntity(nextInfo, GridPortfolio_nextUUID(portfolio), &bed);
	}
}



void GridSpace_randomSpawn(GridPortfolio* portfolio) {

	if (portfolio->infoLib->hostGrid->entityCount >= portfolio->infoLib->hostGrid->maxEntityCount)	// No more room for entities
		return;

	Info* nextInfo = &(portfolio->infoLib)[rand() % portfolio->infoCount];
	BasicEntityData bed = GridSpace_generateRandomEntityData(nextInfo);
	GridSpace_addEntity(nextInfo, GridPortfolio_nextUUID(portfolio), &bed);
}

void GridSpace_EntityReport(Entity* entity) {
	printf(
		"Entity target cellIndex: %d\n"
		"Entity position (y/x): %d/%d\n"
		"Entity delta (y/x): %d/%d\n\n",
		entity->cellIndex,
		*(int*)Entity_getData(entity, sizeof(int), POS_X), *(int*)Entity_getData(entity, sizeof(int), POS_Y),
		*(int*)Entity_getData(entity, sizeof(int), DELTA_X), *(int*)Entity_getData(entity, sizeof(int), DELTA_Y));
}

void GridSpace_updateEntity(Entity* entity, GravityWell* gravityWell) {

	float* data = Entity_getDataLoc(entity);
	BasicEntityData* _data = data;

	if (!Entity_isActive(entity))
		return;

	if (data[HEALTH] <= 0) {
		Entity_setActive(entity, 0);
		entity->info->hostGrid->entityCount--;
		//printf("%s died...\n", entity->info->name);
		return;
	}

	float fastDistance = 0;
	float fastCurrentSpeed = 0;
	int deltas = entity->info->hostGrid->dimCount;

	for (int i = 0; i < deltas; i++) {

		data[i] += data[i + deltas];	// position in first chunk, delta in second chunk

		// fast distance from gravity well
		if (gravityWell) {
			float dG = (data[i] - gravityWell->point[i]);
			float v = data[i + deltas];
			fastDistance += (dG * dG);
			fastCurrentSpeed += (v * v);
		}


		// wall bounce
		float dimMax = entity->info->hostGrid->dimCellCounts[i] * entity->info->hostGrid->cellSize;
		if (data[i] < 0 || data[i] >= dimMax) {
			if (data[i] < 0)
				data[i] = 0;
			else
				data[i] = dimMax - 1;
			data[i + deltas] *= -1;
		}
	}

	// gravity & friction
	if (gravityWell) {
		float fastMaxSpeed = entity->info->maxSpeed;
		fastMaxSpeed *= fastMaxSpeed;
		fastMaxSpeed *= deltas;

		for (int i = 0; i < deltas; i++) {
			float dG = (data[i] - gravityWell->point[i]);
			data[i + deltas] -= fastDistance > 0 ? (gravityWell->force * dG) / (fastDistance) : gravityWell->force;
			data[i + deltas] *= fastCurrentSpeed > fastMaxSpeed ? .9f : 1;
		}
	}

	GridSpace_updateCellIndexOfEntity(entity);
}

Entity* GridSpace_getEntityByIndex(GridSpace* gridSpace, int index) {
	return gridSpace->memory + (index * gridSpace->entitySize);
}

// May be out of bounds, always check return value;
inline int GridSpace_getDimIndexOfEntityPosition(Entity* entity, int dim) {
	float* entityData = Entity_getData(entity, sizeof(float), dim);
	return (int)(*entityData / entity->info->hostGrid->cellSize); // Make faster version with bitmasking, or dedicated bytes per dim
}

int GridSpace_getCellIndexByIndices(GridSpace* gridSpace, int* indices)
{
	int cellIndex = 0;
	int weight = 1;

	for (int d = gridSpace->dimCount - 1; d >= 0; d--) {
		int dimIndex = indices[d];

		// off the grid
		if (dimIndex < 0 || dimIndex >= gridSpace->dimCellCounts[d])
			return -1;

		cellIndex += dimIndex * weight;
		weight *= gridSpace->dimCellCounts[d];
	}

	return cellIndex;
}

Cell* GridSpace_getCellByIndex(GridSpace* gridSpace, int index) {
	return (index < 0 || index >= gridSpace->totalCellCount) ? NULL : &(gridSpace->meta[index]);
}

Cell* GridSpace_getCellByIndices(GridSpace* gridSpace, int* indices) {
	return GridSpace_getCellByIndex(gridSpace, GridSpace_getCellIndexByIndices(gridSpace, indices));
}

int GridSpace_getCellIndexOfEntity(Entity* entity) {

	int cellIndex = 0;
	int weight = 1;

	for (int d = entity->info->hostGrid->dimCount - 1; d >= 0; d--) {
		int dimIndex = GridSpace_getDimIndexOfEntityPosition(entity, d);

		// off the grid
		if (dimIndex < 0 || dimIndex >= entity->info->hostGrid->dimCellCounts[d])
			return -1;

		cellIndex += dimIndex * weight;
		weight *= entity->info->hostGrid->dimCellCounts[d];
	}

	return cellIndex;
}

void GridSpace_rollCellForward(GridSpace* gridSpace, int currentCellIndex) {

	//int rollCount = 0;

	Cell* currentCell = GridSpace_getCellByIndex(gridSpace, currentCellIndex);

	for (int i = (currentCell->startIndex + currentCell->count) - 1; i >= currentCell->startIndex; i--) {

		Entity* currentEntity = GridSpace_getEntityByIndex(gridSpace, i);
		int intendedCellIndex = currentEntity->cellIndex;

		if (!Entity_isActive(currentEntity) ||		// Inactive
			intendedCellIndex < 0 ||				// Out of bounds
			intendedCellIndex > currentCellIndex	// towards end
			) {

			GridSpace_swapEntities(gridSpace, i, currentCell->startIndex + currentCell->count - 1);
			currentCell->count--;

			Cell* nextCell = GridSpace_getCellByIndex(gridSpace, currentCellIndex + 1);
			if (nextCell) {
				nextCell->startIndex--;
				nextCell->count++;
			}
		}
	}

	//if (rollCount > 50)
	//	printf("rollForwardCount: %d\n", rollCount);
}

void GridSpace_rollCellBackward(GridSpace* gridSpace, int currentCellIndex) {

	//int rollCount = 0;

	Cell* currentCell = GridSpace_getCellByIndex(gridSpace, currentCellIndex);

	for (int i = currentCell->startIndex; i < (currentCell->startIndex + currentCell->count); i++) {

		Entity* currentEntity = GridSpace_getEntityByIndex(gridSpace, i);
		int intendedCellIndex = currentEntity->cellIndex;//GridSpace_getCellIndexOfEntity(currentEntity);

		if (intendedCellIndex < currentCellIndex) {

			GridSpace_swapEntities(gridSpace, i, currentCell->startIndex);
			currentCell->count--;
			currentCell->startIndex++;

			Cell* previousCell = GridSpace_getCellByIndex(gridSpace, currentCellIndex - 1);
			if (previousCell) {
				previousCell->count++;
			}

			//rollCount++;
		}
	}

	//if (rollCount > 50)
	//	printf("rollBackCount: %d\n\n", rollCount);
}

int GridSpace_checkEntityCollision(Entity* a, Entity* b) {

	// Self check
	if (a == b)
		return 0;

	// Mask check
	if (!(a->info->collisionMask & b->info->collisionMask))
		return 0;

	// Spacial Check
	RECT rectA, rectB;

	GridSpace_getRectFromEntity(a, &rectA);
	GridSpace_getRectFromEntity(b, &rectB);

	if (rectA.right < rectB.left ||
		rectB.right < rectA.left ||
		rectA.bottom < rectB.top ||
		rectB.bottom < rectA.top)
		return 0;

	return 1;
}

void GridSpace_procEntityCollision(Entity* a, Entity* b) {
	BasicEntityData* dataA = Entity_getDataLoc(a);
	BasicEntityData* dataB = Entity_getDataLoc(b);

	//RECT rectA, rectB;
	//
	//GridSpace_getRectFromEntity(a, &rectA);
	//GridSpace_getRectFromEntity(b, &rectB);
	//
	//if (rectA.right < rectB.left ||
	//	rectB.right < rectA.left ||
	//	rectA.bottom < rectB.top ||
	//	rectB.bottom < rectA.top)
	//	return;

	// Damage
	if (a->info->association != b->info->association) {
		dataA->health -= b->info->collisionDmg;
		dataB->health -= a->info->collisionDmg;
	}

	// Half adjust each entity for collision
	int halfDifX = (dataA->px - dataB->px) / 2;
	int halfDifY = (dataA->py - dataB->py) / 2;

	int absDifX = halfDifX > 0 ? halfDifX : -halfDifX;
	int absDifY = halfDifY > 0 ? halfDifY : -halfDifY;

	halfDifX = absDifX > absDifY ? halfDifX : 0;
	halfDifY = absDifY > absDifX ? halfDifY : 0;

	dataA->px += halfDifX;
	dataA->py += halfDifY;
	dataB->px -= halfDifX;
	dataB->px -= halfDifY;

	// Swap velocities
	SwapMemory(&(dataA->dx), &(dataB->dx), 2 * sizeof(int));

}

void GridSpace_updateInterGridCollisions(Entity* entity, int cellIndex) {
	// Skip your own cell, its already been checked
	if (entity->cellIndex == cellIndex)
		return;

	if (entity->info->hostGrid->totalCellCount <= cellIndex ||
		0 > cellIndex)
		return;

	GridSpace* gridSpace = entity->info->hostGrid;
	Cell* collidingCell = GridSpace_getCellByIndex(gridSpace, cellIndex);
	for (int i = collidingCell->startIndex; i < collidingCell->count + collidingCell->startIndex; i++) {
		Entity* other = GridSpace_getEntityByIndex(gridSpace, i);
		gridSpace->_collisionCheckCount++;
		if (GridSpace_checkEntityCollision(entity, other))
			GridSpace_procEntityCollision(entity, other);
	}

}

void GridSpace_dimRangeProc(GridSpace* gridSpace, void(*proc)(void*, int), void* args, int trgCell, int range, int depth) {
	depth -= 1;
	if (depth < 0) {

		proc(args, trgCell);
		return;
	}

	for (int i = -range; i <= range; i++)
		GridSpace_dimRangeProc(gridSpace, proc, args,
			trgCell + (i * gridSpace->dimCellWeights[depth]),
			range, depth);
}

void GridSpace_dimensionsRangeProc(GridSpace* gridSpace, void(*proc)(void*, int), void* args, int originCell, int range) {
	GridSpace_dimRangeProc(gridSpace, proc, args, originCell, range, gridSpace->dimCount);
}

void GridSpace_updateEntityCollisions(GridSpace* gridSpace) {
	for (int c = 0; c < gridSpace->totalCellCount; c++)
		for (int e = gridSpace->meta[c].startIndex; e < gridSpace->meta[c].count + gridSpace->meta[c].startIndex; e++) {
			Entity* a = GridSpace_getEntityByIndex(gridSpace, e);

			for (int o = e + 1; o < gridSpace->meta[c].count + gridSpace->meta[c].startIndex; o++) {
				gridSpace->_collisionCheckCount++;
				Entity* b = GridSpace_getEntityByIndex(gridSpace, o);
				if (GridSpace_checkEntityCollision(a, b))
					GridSpace_procEntityCollision(a, b);
			}
			GridSpace_dimensionsRangeProc(gridSpace, GridSpace_updateInterGridCollisions, a, a->cellIndex, a->info->checkRange);
		}
}

void GridSpace_rollForward(GridSpace* gridSpace) {
	for (int i = 0; i < gridSpace->totalCellCount; i++)
		GridSpace_rollCellForward(gridSpace, i);
}

void GridSpace_rollBackward(GridSpace* gridSpace) {
	for (int i = gridSpace->totalCellCount - 1; i >= 0; i--)
		GridSpace_rollCellBackward(gridSpace, i);
}

void GridSpace_updateEntities(GridSpace* gridSpace, GravityWell* gravityWell) {
	for (int i = 0; i < gridSpace->entityCount; i++)
		GridSpace_updateEntity(GridSpace_getEntityByIndex(gridSpace, i), gravityWell);
}

void GridSpace_printCellCounts(GridSpace* gridSpace) {
	for (int i = 0; i < gridSpace->totalCellCount; i++)
		printf("Cell %d: %d\n", i, gridSpace->meta[i].count);
}

void GridSpace_testUpdate(
	GridSpace* gridSpace,
	GridPortfolio* portfolio,
	GravityWell* gravityWell
) {
	if (!gridSpace)
		return;

	gridSpace->_collisionCheckCount = 0;
	portfolio->spawnTimer++;

	if (portfolio->spawnTimer >= portfolio->spawnRate) {
		GridSpace_randomSpawn(portfolio);
		portfolio->spawnTimer = 0;
	}
	
	// Always perform these operations in this order: updates, rolls, collisions
	GridSpace_updateEntities(gridSpace, gravityWell);
	GridSpace_rollForward(gridSpace);
	GridSpace_rollBackward(gridSpace);
	GridSpace_updateEntityCollisions(gridSpace);

	gridSpace->_collisionCheckCycleMax =
		gridSpace->_collisionCheckCount > gridSpace->_collisionCheckCycleMax ?
		gridSpace->_collisionCheckCount : gridSpace->_collisionCheckCycleMax;
}