#pragma once

#include <vector>
#include <functional>
#include "Component.h"

class Drawable {
public:
	static const SDL_Color bgColor, borderColor, textColor;

	int x, y;

	Drawable(int x, int y);

	void addChild(Drawable* child);

	int getX();
	int getY();

	virtual void draw(Renderer& renderer);

	virtual void remove();

	virtual bool onMouseDown(SDL_MouseButtonEvent* evt);
	virtual bool onMouseUp(SDL_MouseButtonEvent* evt);
	virtual bool onMouseMotion(SDL_MouseMotionEvent* evt);

	virtual bool onKeyDown(SDL_KeyboardEvent* evt);

	virtual bool onTextInput(SDL_TextInputEvent* evt);

protected:
	Drawable* parent = nullptr;
	std::vector<Drawable*> children;
};

class Draggable : public Drawable {
public:
	Draggable(int x, int y);

	int* minX, * minY, * maxX, * maxY;
	int* left, * right, * top, * bottom;

	virtual bool inDragArea(int x, int y) = 0;

	void constrain();

	virtual bool onMouseDown(SDL_MouseButtonEvent* evt);
	virtual bool onMouseUp(SDL_MouseButtonEvent* evt);
	virtual bool onMouseMotion(SDL_MouseMotionEvent* evt);

protected:
	bool dragging = false;

	int dragX, dragY;
};

class Connector;
class Output;

class Socket : public Drawable {
public:
	static const int radius;

	static std::vector<Socket*> sockets;

	Connector* connector;
	Output* output;

	Socket(int x, int y);

	virtual void draw(Renderer& renderer);

	virtual void remove();
};

class Knob : public Draggable {
public:
	static const int radius;
	static const int notchSize;

	float value;

	Knob(int x, int y, std::vector<float> notches = { -1, 0, 1 });

	virtual void draw(Renderer& renderer);

	virtual bool inDragArea(int x, int y);

	virtual bool onMouseMotion(SDL_MouseMotionEvent* evt);

private:
	std::vector<float> notches;

	float angle(float value);
};

class Button : public Drawable {
public:
	static const int radius;
	static const int border;

	bool pressed;
	bool toggle;

	Button(int x, int y, bool toggle = false);

	virtual void draw(Renderer& renderer);

	virtual bool onMouseDown(SDL_MouseButtonEvent* evt);
	virtual bool onMouseUp(SDL_MouseButtonEvent* evt);
};

class Input : public Drawable {
public:
	virtual int textX();
	virtual int textY();

	int width, height;

	Socket* socket;

	Input(const char* name, int x, int y, int width, int height);

	virtual void draw(Renderer& renderer);

	virtual float getValue();

protected:
	Input(const char* name, int x, int y, int width, int height, int socketX, int socketY);

private:
	const char* name;
};

class KnobInput : public Input {
public:
	static const int knobX, knobY;

	virtual int textX();
	virtual int textY();

	KnobInput(const char* name, int x, int y, std::vector<float> notches = { -1, 0, 1 });

	virtual float getValue();

private:
	Knob* knob;
};

class ButtonInput : public Input {
public:
	static const int buttonX, buttonY;

	virtual int textX();
	virtual int textY();

	ButtonInput(const char* name, int x, int y, bool toggle = false);

	virtual float getValue();

private:
	Button* button;
};

class Module;

class Output : public Drawable {
public:
	static const int socketX, socketY;

	std::function<float()> nextValue;

	Socket* socket;

	float value = 0;

	Output(const char* name, int x, int y, std::function<float()> nextValue);

	virtual void draw(Renderer& renderer);

	void step();

private:
	const char* name;
};

class Connector : public Draggable {
public:
	static const int radius, snapDistance;

	Socket* socket;

	Connector(int x, int y);

	int getDrawX();
	int getDrawY();

	virtual void draw(Renderer& renderer);

	virtual bool inDragArea(int x, int y);

	virtual bool onMouseMotion(SDL_MouseMotionEvent* evt);

	Connector* other;
};

class Cable : public Drawable {
public:
	Cable(int x, int y, SDL_Color color);

	Connector* start;
	Connector* end;

	void draw(Renderer& renderer);

private:
	SDL_Color color;
};

class MenuOption {
public:
	const char* label;
	std::function<void(int, int)> action;

	MenuOption(const char* label, std::function<void(int, int)> action = nullptr);
};

class Menu : public Drawable {
public:
	static const int optionHeight;
	static const SDL_Color optionColor, hoverColor;

	bool open;

	Menu(int x, int y, int width, std::vector<MenuOption> options);

	void draw(Renderer& renderer);

	virtual bool onMouseMotion(SDL_MouseMotionEvent* evt);
	virtual bool onMouseDown(SDL_MouseButtonEvent* evt);
	virtual bool onMouseUp(SDL_MouseButtonEvent* evt);

private:
	int width;
	int hovered;
	std::vector<MenuOption> options;
};

class EditText : public Drawable {
public:
	static const int padding;
	static const int height;
	static const int doubleClickDelay;

	std::string text;
	long clickTime;

	EditText(int x, int y, int width, const char* text = "");

	void draw(Renderer& renderer);

	virtual bool onMouseDown(SDL_MouseButtonEvent* evt);

	virtual bool onKeyDown(SDL_KeyboardEvent* evt);

	virtual bool onTextInput(SDL_TextInputEvent* evt);

private:
	int width;
	int cursor;
	bool editing;
};