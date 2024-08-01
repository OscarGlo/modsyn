#pragma once

#include "Drawable.h"

class Module : public Draggable {
public:
	static const int borderWidth;
	static const int headerHeight;

	bool queueDelete;
	bool deletable;

	EditText* title;

	int width, height;

	Module(const char* name, int w, int h, int x, int y, bool deletable = true);

	virtual void step();

	virtual void draw(Renderer& renderer);

	bool inDragArea(int x, int y);

	bool onMouseDown(SDL_MouseButtonEvent* evt);
};

class WaveGenerator : public Module {
public:
	WaveGenerator(int x, int y);

	virtual void step();

private:
	float phase;

	Input* freq;
	Input* type;

	Output* output;
};

class Player : public Module {
public:
	static const float delta;

	Input* input;

	Player(int x, int y);
};

class Scope : public Module {
public:
	static const int bufferLength;

	Scope(int x, int y);

	virtual void step();

	virtual void draw(Renderer& renderer);

private:
	int n;
	std::vector<float> buffer;

	Input* input;
	Input* rate;
};

class BitCrusher : public Module {
public:
	BitCrusher(int x, int y);

private:
	Input* input;
	Input* depth;

	Output* output;
};

class ADSR : public Module {
public:
	ADSR(int x, int y);

	virtual void step();

	virtual void draw(Renderer& renderer);

private:
	bool pressed;

	float pressTime;
	float releaseTime;

	Input* attack;
	Input* decay;
	Input* sustain;
	Input* release;

	Input* trigger;

	Output* output;
};