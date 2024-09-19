#pragma once

#include "LMDefs.h"

// Data begins immediately after the header
inline size_t Entity_getDataLoc(Entity* entity) {
	return ((size_t)entity) + sizeof(Entity);
}

inline size_t Entity_getData(Entity* entity, size_t size, int index) {
	return Entity_getDataLoc(entity) + (index * size);
}

inline void Entity_setData(Entity* entity, size_t size, int index, void* src) {
	char* ptr = &((char*)Entity_getDataLoc(entity))[index * size];
	for (size_t i = 0; i < size; i++) {
		ptr[i] = ((char*)src)[i];
	}
}

inline int Entity_isActive(Entity* entity) {
	return entity->flags & (0x1 << ACTIVE);
}

inline void Entity_setActive(Entity* entity, int state) {
	entity->flags &= ~(0x1 << ACTIVE);
	entity->flags |= (state > 0 ? 0x1 : 0x0 << ACTIVE);
}

void Entity_ctor(Entity* entity, Info* info, int uuid, size_t initSize, void* init);