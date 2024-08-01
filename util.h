#pragma once

#include <time.h>
#include <SDL2/SDL_rect.h>

static bool pointInRect(int x, int y, const SDL_Rect* rect) {
	return x > rect->x && x < rect->x + rect->w
		&& y > rect->y && y < rect->y + rect->h;
}

static bool pointInCircle(int x, int y, int cx, int cy, int r) {
	int rx = x - cx;
	int ry = y - cy;
	return rx * rx + ry * ry < r * r;
}

static long getMs() {
	return (long) (1000 * (double)clock() / CLOCKS_PER_SEC);
}