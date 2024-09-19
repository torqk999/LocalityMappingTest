#include <stdio.h>
#include <stdlib.h>
#include "Windows.h"
#include "gl/GL.h"

#include "GridSpaceRender.h"

/*
	1- Update & ForwardMigrate
	2- ReverseMigrate
	3- Collisions
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
		myPalette.colors[BCKGRND],
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
