#pragma once

#include "GridSpace.h"
#include "Render.h"
#include "process.h"

void GridSpaceRender_renderGrid(GridRenderContainer* grc);

void GridSpaceRender_renderElements(GridRenderContainer* grc);

LRESULT CALLBACK GridSpaceRender_messageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void GridSpaceRender_init(GridSpace* gridSpace);