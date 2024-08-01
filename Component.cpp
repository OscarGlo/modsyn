#include <format>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Component.h"

#define COMPONENT_EXCEPTION(f) ComponentException(std::format(f, SDL_GetError()).c_str())

constexpr int CIRCLE_POINTS = 16;

ComponentException::ComponentException(const char* msg) {
	message = msg;
}

const char* ComponentException::what() const {
	return message;
}

SDL::SDL(int flags) {
	int code = SDL_Init(flags);
	if (code < 0)
		throw COMPONENT_EXCEPTION("Couldn't initialize SDL: %s");
}

SDL::~SDL() {
	SDL_Quit();
}

AudioDevice::AudioDevice(SDL_AudioSpec* audioSpec) {
	id = SDL_OpenAudioDevice(nullptr, 0, audioSpec, audioSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if (id == 0)
		throw COMPONENT_EXCEPTION("Couldn't open audio device: %s");

	SDL_PauseAudioDevice(id, SDL_FALSE);
}

AudioDevice::~AudioDevice() {
	SDL_CloseAudioDevice(id);
}

TTF::TTF() {
	int code = TTF_Init();
	if (code < 0)
		throw COMPONENT_EXCEPTION("Couldn't initialize TTF: %s");
}

TTF::~TTF() {
	TTF_Quit();
}

Font::Font(const char* src, int size) {
	ttf = TTF_OpenFont(src, size);
	if (ttf == NULL)
		throw COMPONENT_EXCEPTION("Couldn't load font: %s");
}

Font::~Font() {
	TTF_CloseFont(ttf);
}

int Font::height() {
	return TTF_FontHeight(ttf);
}

Window::Window(const char* name, int width, int height) {
	this->width = width;
	this->height = height;
	sdl = SDL_CreateWindow(
		name,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_SHOWN
	);
	if (sdl == NULL)
		throw COMPONENT_EXCEPTION("Couldn't create window: %s");

}

Window::~Window() {
	SDL_DestroyWindow(sdl);
}

Renderer::Renderer(const Window& window) {
	sdl = SDL_CreateRenderer(window.sdl, -1, 0);
	if (sdl == NULL)
		throw COMPONENT_EXCEPTION("Couldn't create renderer: %s");

	font = new Font("JetBrainsMono-Regular.ttf", 12);
}

Renderer::~Renderer() {
	SDL_DestroyRenderer(sdl);
}

void Renderer::fillRect(const SDL_Rect* rect, const SDL_Color color) {
	SDL_SetRenderDrawColor(sdl, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(sdl, rect);
}

void Renderer::line(int x1, int y1, int x2, int y2, const SDL_Color color) {
	SDL_SetRenderDrawColor(sdl, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(sdl, x1, y1, x2, y2);
}

void Renderer::lines(const SDL_Point* points, int count, const SDL_Color color) {
	SDL_SetRenderDrawColor(sdl, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLines(sdl, points, count);
}

void Renderer::strokeRect(const SDL_Rect* rect, int weight, const SDL_Color color) {
	SDL_SetRenderDrawColor(sdl, color.r, color.g, color.b, color.a);

	if (weight == 1) {
		SDL_RenderDrawRect(sdl, rect);
		return;
	}

	int w = weight / 2;
	int W = (weight + 1) / 2;

	SDL_Rect top{ rect->x - W, rect->y - W, W + rect->w + W, weight };
	SDL_RenderFillRect(sdl, &top);

	SDL_Rect bottom{ rect->x - W, rect->y + rect->h - w, W + rect->w + W, weight };
	SDL_RenderFillRect(sdl, &bottom);

	SDL_Rect left{ rect->x - W, rect->y - W, weight, W + rect->h + W };
	SDL_RenderFillRect(sdl, &left);

	SDL_Rect right{ rect->x + rect->w - w, rect->y - W, weight, W + rect->h + W };
	SDL_RenderFillRect(sdl, &right);
}

void Renderer::fillCircle(int x, int y, int r, const SDL_Color color) {
	SDL_SetRenderDrawColor(sdl, color.r, color.g, color.b, color.a);

	SDL_Vertex vertices[CIRCLE_POINTS + 1]{};
	int indices[CIRCLE_POINTS * 3]{};

	vertices[0] = { { (float)x, (float)y }, color, { 0, 0 } };

	float off = 2 * M_PI / (CIRCLE_POINTS * 2);

	for (int i = 0; i < CIRCLE_POINTS; i++) {
		float a = 2 * M_PI * i / CIRCLE_POINTS + off;
		vertices[i + 1] = { { (float)x + SDL_cosf(a) * r, (float)y + SDL_sinf(a) * r }, color, { 0, 0 } };

		indices[3 * i] = 0;
		indices[3 * i + 1] = i + 1;
		indices[3 * i + 2] = i < CIRCLE_POINTS - 1 ? i + 2 : 1;
	}

	SDL_RenderGeometry(sdl, nullptr, vertices, CIRCLE_POINTS + 1, indices, CIRCLE_POINTS * 3);
}

int Renderer::measureText(const char* text) {
	int width = 0;
	TTF_SizeText(font->ttf, text, &width, nullptr);
	return width;
}

void Renderer::renderText(int x, int y, const char* text, const SDL_Color color) {
	SDL_Surface* textSurface = TTF_RenderText_Blended(font->ttf, text, color);
	
	if (textSurface) {
		SDL_Texture* textTexture = SDL_CreateTextureFromSurface(sdl, textSurface);
		SDL_RenderCopy(sdl, textTexture, NULL, new SDL_Rect{ x, y, textSurface->w, textSurface->h });

		SDL_FreeSurface(textSurface);
		SDL_DestroyTexture(textTexture);
	}
}

void Renderer::scaledPoints(const SDL_Point* points, int count, int scale, const SDL_Color color) {
	SDL_FPoint* scaled = (SDL_FPoint*) malloc(sizeof(SDL_FPoint) * count);
	for (int i = 0; i < count; i++)
		scaled[i] = SDL_FPoint(((float) points[i].x) / scale, ((float) points[i].y) / scale);

	SDL_RenderSetScale(sdl, scale, scale);
	SDL_SetRenderDrawColor(sdl, color.r, color.g, color.b, color.a);
	SDL_RenderDrawPointsF(sdl, scaled, count);
	SDL_RenderSetScale(sdl, 1, 1);
}

void Renderer::bezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int count, int weight, const SDL_Color color) {
	SDL_Point* points = (SDL_Point*) malloc(sizeof(SDL_Point) * (count + 1));
	for (int i = 0; i <= count; i++) {
		float t = (float)i / count;
		float t1 = 1 - t;
		points[i] = SDL_Point(
			t1 * t1 * t1 * x1 + 3 * t1 * t1 * t * x2 + 3 * t1 * t * t * x3 + t * t * t * x4,
			t1 * t1 * t1 * y1 + 3 * t1 * t1 * t * y2 + 3 * t1 * t * t * y3 + t * t * t * y4
		);
	}
	scaledPoints(points, count, weight, color);
}