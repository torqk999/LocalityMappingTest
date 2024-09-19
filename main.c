#include <stdio.h>
#include <stdlib.h>
#include "Windows.h"

#include "GridSpaceRender.h"

/*

	The arena is set to the size of the maximum number of allowed entities.
	No additional vectors or dynamic allocation will take place, instead an
	'aggressive' two directional bubble sort takes place. We know that collision
	checks will be based on entities relative locality, so the approach relies
	on keeping elements in the memory close to one another based on their
	'virtual' proximity to the fixed cells. A set of fixed meta Cell objects
	are created as 'bookmarks' into the arena, as they contain an index and a
	count. When an entity updates its position, it also updates its 'intended'
	cell to be in. During the migrations, each cell checks to see whether or not
	its entities are in the appropriate cell, or if they need to migrate. A swap
	sort takes place with the leading edge of the cell, and the current and
	adjacent cells adjust their index and counts, since they share an edge in
	memory. Across the first dimension, entities will migrate only one cell, but
	across higher dimensions, it will unfortunately take them longer to travel.
	However, because their locations in memory are still in cache, the workload
	focuses the cpu on swaps rather than indirection. A single pass both forward
	backwards migrates all migrations. The cell's memory 'chunk' remains unsorted,
	but localized. When the collision pass occurs, it continues to benefit from
	entities being grouped both physically in memory, and virtually in their
	respective cells. Is implemented for 'n' number of dimensions.

	1- Update
	2- ForwardMigrate
	3- ReverseMigrate
	4- Collisions
*/


#define ENT_CNT 2000

typedef struct {
	int count;
	HBRUSH* colors;
} BrushPalette;

typedef enum {
	BCKGRND,
	RED,
	GREEN
} MY_COLORS;

void BrushPalette_dtor(BrushPalette* brushPalette) {
	for (int i = 0; i < brushPalette->count; i++)
		DeleteObject(brushPalette->colors[i]);
	brushPalette->count = 0;
}

int main() {

	srand(time(NULL));

	BrushPalette myPalette = { 3,
		(HBRUSH[]) {
			CreateSolidBrush(RGB(255, 255, 255)),
			CreateSolidBrush(RGB(255, 0, 0)),
			CreateSolidBrush(RGB(0, 255, 0))
		}
	};

	GridSpace myGrid = GridSpace_ctor(
		"My Test Grid",
		//myPalette.colors[BCKGRND],
		2,
		(int[]) { 20, 20 },
		ENT_CNT,
		50,
		sizeof(BasicEntityData)
	);

	GridPortfolio myPortfolio = {
		0,
		2,
		(Info[]) {
			{
				"Bonom",
				& myGrid,
				//& nextId,

				myPalette.colors[GREEN],
				TEAM_A,
				BONOM,
				1,
				1,
				100,
				3,
				10
			},
			{
				"Broheem",
				&myGrid,
				//&nextId,

				myPalette.colors[RED],
				TEAM_B,
				BONOM,
				1,
				2,
				50,
				6,
				8
			}
		},
		2,
		0
	};

	GravityWell myWell = {
		5,
		(float[]) {
			500,
			500
		}
	};

	GridSpaceRender_init(&myGrid);

	while(1){
		GridSpace_testUpdate(&myGrid, &myPortfolio, &myWell);
		Sleep(1);
	}

	GridSpace_dtor(&myGrid);

	BrushPalette_dtor(&myPalette);

}
