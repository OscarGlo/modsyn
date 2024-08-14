#pragma once

#include <format>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <exception>

class ComponentException : std::exception {
public:
	const char* message;

	ComponentException(const char* msg);

	const char* what() const;
};

class Component {};

class SDL : Component {
public:
	SDL(int flags);
	~SDL();
};

class AudioDevice : Component {
public:
	SDL_AudioDeviceID id;

	AudioDevice(SDL_AudioSpec* audioSpec);
	~AudioDevice();
};

class TTF : Component {
public:
	TTF();
	~TTF();
};

class Font : Component {
public:
	TTF_Font* ttf;

	Font(const char* src, int size);
	~Font();

	int height();
};

class Window : Component {
public:
	SDL_Window* sdl;

	int width, height;

	Window(const char* name, int width, int height, bool resizable = true);
	~Window();
};

class Renderer : Component {
public:
	SDL_Renderer* sdl;

	Font* font;

	Renderer(const Window& window);
	~Renderer();

	void fillRect(const SDL_Rect* rect, const SDL_Color color);

	void line(int x1, int y1, int x2, int y2, const SDL_Color color);

	void lines(const SDL_Point* points, int count, const SDL_Color color);

	void strokeRect(const SDL_Rect* rect, int weight, const SDL_Color color);

	void fillCircle(int x, int y, int r, const SDL_Color color);

	int measureText(const char* text);

	void renderText(int x, int y, const char* text, const SDL_Color color);

	void scaledPoints(const SDL_Point* points, int count, int scale, const SDL_Color color);

	void bezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int count, int weight, const SDL_Color color);
};