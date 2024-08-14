#pragma once

#include "Drawable.h"

class Module : public Draggable {
public:
	static const int borderWidth;
	static const int headerHeight;

	bool deletable;

	EditText* title;

	int width, height;

	Module(const char* name, int w, int h, int x, int y, bool deletable = true);

	virtual void step();

	virtual void draw(Renderer& renderer);

	virtual void remove();

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
	float pressValue;
	float releaseTime;
	float releaseValue;

	Input* attack;
	Input* decay;
	Input* sustain;
	Input* release;

	Input* trigger;

	Output* output;
};


class Delay : public Module {
public:
	Delay(int x, int y);


	virtual void step();
	virtual int getSampleOffset();

private:
	Input* input;
	Input* amount;
	Input* dryness;

	static const float delayMax;
	int maxSampleStored;

	int storedDelayOffset;
	float* buffer;
	float* transitionBuffer;
	int writeIndex, readIndex;

	bool odd;

	Output* output;
};

class Record : public Module {
public:
	Record(int x, int y);

	virtual void step();

private:
	Input* input;
	Input* recordButton;
	Input* readAt;
	Input* stopAt;

	static const float recordMax;
	int maxSampleStored;

	float* buffer;
	int currentIndex;

	bool isRecording;

	Output* output;
};

class Mixer : public Module {
public:
	Mixer(int x, int y);

private:
	Input* input;
	Input* volume;

	Output* output;
};