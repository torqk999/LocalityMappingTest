#include "Entity.h"

void Entity_ctor(Entity* entity, Info* info, int uuid, size_t entitySize, void* init){
	entity->info = info;
	entity->flags = 0;
	entity->cellIndex = -1;
	entity->uuid = uuid;
	Entity_setData(entity, entitySize - sizeof(Entity), 0, init);
	Entity_setActive(entity, 1);
}
