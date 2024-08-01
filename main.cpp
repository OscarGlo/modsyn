#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include "audioConfig.h"
#include "Component.h"
#include "Module.h"

constexpr int WIDTH = 640;
constexpr int HEIGHT = 480;

SDL_Color red{ 0xDD, 0x22, 0x22 };

std::vector<Drawable*> objects;

static SDL_Color randomColor() {
	float angle = 2 * M_PI * std::rand() / RAND_MAX;
	float light = 0.6 + 0.4 * std::rand() / RAND_MAX;
	return SDL_Color{
		(unsigned char)(255 * (0.6 + 0.4 * light * cos(angle))),
		(unsigned char)(255 * (0.6 + 0.4 * light * cos(angle + (2 * M_PI / 3)))),
		(unsigned char)(255 * (0.6 + 0.4 * light * cos(angle - (2 * M_PI / 3)))),
	};
}

static Draggable* windowBounds(Draggable* obj) {
	obj->minX = new int(0);
	obj->minY = new int(0);
	obj->maxX = new int(WIDTH);
	obj->maxY = new int(HEIGHT);
	return obj;
}

Player player = *(Player*) windowBounds(new Player(20, 20));

Menu moduleMenu(0, 0, 120, std::vector<MenuOption>{
	MenuOption("Add cable/module"),
	MenuOption("Cable", [](int x, int y) {
		Cable* cable = new Cable(x, y, randomColor());
		windowBounds(cable->start);
		windowBounds(cable->end);
		objects.insert(objects.begin() + 1, cable);
	}),
	MenuOption("VCO", [](int x, int y) {
		objects.push_back(windowBounds(new WaveGenerator(x, y)));
	}),
	MenuOption("ADSR", [](int x, int y) {
		objects.push_back(windowBounds(new ADSR(x, y)));
	}),
	MenuOption("Scope", [](int x, int y) {
		objects.push_back(windowBounds(new Scope(x, y)));
	}),
	MenuOption("BitCrusher", [](int x, int y) {
		objects.push_back(windowBounds(new BitCrusher(x, y)));
	}),
});

SDL_AudioSpec audioSpec {
	.freq = SAMPLE_RATE,
	.format = AUDIO_F32,
	.channels = 1,
	.samples = BUFFER_SIZE,
	.callback = [](void* userdata, uint8_t * stream, int len) {
		auto buffer = reinterpret_cast<float*>(stream);
		int samples = len / sizeof(float);

		for (int i = 0; i < samples; i++) {
			for (Drawable* obj : objects) {
				Module* m = dynamic_cast<Module*>(obj);
				if (m != nullptr)
					m->step();
			}
			buffer[i] = player.input->getValue();
		}
	},
};

int main(int argc, char* args[]) {
	std::srand(std::time(nullptr));

	new SDL(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	new TTF();

	Window window("modsynth", WIDTH, HEIGHT);
	Renderer renderer(window);

	AudioDevice audio(&audioSpec);

	objects = { &moduleMenu, &player };

	bool running = true;
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				running = false;

			if (e.type == SDL_MOUSEBUTTONDOWN) {
				bool handled = false;
				for (Drawable* o : objects) {
					if (o->onMouseDown(&e.button)) {
						handled = true;
						break;
					}
				}
				if (!handled && e.button.button == SDL_BUTTON_RIGHT) {
					moduleMenu.x = e.button.x;
					moduleMenu.y = e.button.y;
					moduleMenu.open = true;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
				for (Drawable* o : objects) {
					if (o->onMouseUp(&e.button))
						break;
				}
			else if (e.type == SDL_MOUSEMOTION)
				for (Drawable* o : objects) {
					if (o->onMouseMotion(&e.motion))
						break;
				}
			else if (e.type == SDL_KEYDOWN) {
				bool handled = false;
				for (Drawable* o : objects) {
					if (o->onKeyDown(&e.key)) {
						handled = true;
						break;
					}
				}
				if (!handled && e.key.keysym.scancode == SDLK_ESCAPE)
					running = false;
			}
			else if (e.type == SDL_TEXTINPUT)
				for (Drawable* o : objects) {
					if (o->onTextInput(&e.text))
						break;
				}
		}

		renderer.fillRect(new SDL_Rect{ 0, 0, WIDTH, HEIGHT }, SDL_Color{ 0, 0, 0 });

		for (int i = objects.size() - 1; i >= 0; i--) {
			Drawable* obj = objects[i];
			Module* mod = dynamic_cast<Module*>(obj);
			if (mod != nullptr && mod->queueDelete) {
				objects.erase(objects.begin() + i);
				i--;
			} else {
				obj->draw(renderer);
			}
		}

		SDL_RenderPresent(renderer.sdl);
	}

	return 0;
}