#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "audioConfig.h"
#include "Module.h"
#include "util.h"
#include "window.h"

const SDL_Color Drawable::bgColor{ 0x20, 0x20, 0x20 };
const SDL_Color Drawable::borderColor{ 0x40, 0x40, 0x40 };
const SDL_Color Drawable::textColor{ 0xD0, 0xD0, 0xD0 };

Drawable::Drawable(int x, int y) {
	this->x = x;
	this->y = y;
	queueDelete = false;
}

void Drawable::addChild(Drawable* child) {
	children.push_back(child);
	child->parent = this;
}

int Drawable::getX() {
	return parent ? x + parent->getX() : x;
}

int Drawable::getY() {
	return parent ? y + parent->getY() : y;
}

void Drawable::draw(Renderer& renderer) {
	for (int i = children.size() - 1; i >= 0; i--)
		children[i]->draw(renderer);
}

void Drawable::remove() {
	for (int i = children.size() - 1; i >= 0; i--)
		children[i]->remove();
}

bool Drawable::onMouseDown(SDL_MouseButtonEvent* evt) {
	for (int i = 0; i < children.size(); i++)
		if (children[i]->onMouseDown(evt))
			return true;
	return false;
}

bool Drawable::onMouseUp(SDL_MouseButtonEvent* evt) {
	for (int i = 0; i < children.size(); i++)
		if (children[i]->onMouseUp(evt))
			return true;
	return false;
}

bool Drawable::onMouseMotion(SDL_MouseMotionEvent* evt) {
	for (int i = 0; i < children.size(); i++)
		if (children[i]->onMouseMotion(evt))
			return true;
	return false;
}

bool Drawable::onKeyDown(SDL_KeyboardEvent* evt) {
	for (int i = 0; i < children.size(); i++)
		if (children[i]->onKeyDown(evt))
			return true;
	return false;
}

bool Drawable::onTextInput(SDL_TextInputEvent* evt) {
	for (int i = 0; i < children.size(); i++)
		if (children[i]->onTextInput(evt))
			return true;
	return false;
}

Draggable::Draggable(int x, int y) : Drawable(x, y) {
	minX = maxX = minY = maxY = nullptr;
	top = bottom = left = right = nullptr;
}

bool Draggable::onMouseDown(SDL_MouseButtonEvent* evt) {
	if (Drawable::onMouseDown(evt)) return true;

	dragging = evt->button == SDL_BUTTON_LEFT && inDragArea(evt->x, evt->y);

	if (dragging) {
		dragX = evt->x - getX();
		dragY = evt->y - getY();
	}

	return dragging;
}

bool Draggable::onMouseUp(SDL_MouseButtonEvent* evt) {
	if (Drawable::onMouseUp(evt)) return true;

	bool tmp = dragging;
	dragging = false;
	return tmp;
}

void Draggable::constrain() {
	int l = left ? *left : 0;
	int mix = minX ? *minX : 0;
	if (x - l < mix) x = mix + l;

	int r = right ? *right : 0;
	int max = maxX ? *maxX : window.width;
	if (x + r > max) x = max - r;

	int t = top ? *top : 0;
	int miy = minY ? *minY : 0;
	if (y - t < miy) y = miy + t;

	int b = bottom ? *bottom : 0;
	int may = maxY ? *maxY : window.height;
	if (y + b > may) y = may - b;
}

bool Draggable::onMouseMotion(SDL_MouseMotionEvent* evt) {
	if (Drawable::onMouseMotion(evt)) return true;

	if (dragging) {
		x = evt->x - dragX;
		y = evt->y - dragY;
		constrain();
	}
	return dragging;
}

const int Socket::radius = 8;

std::vector<Socket*> Socket::sockets;

Socket::Socket(int x, int y) : Drawable(x, y) {
	sockets.push_back(this);
	connector = nullptr;
	output = nullptr;
}

void Socket::draw(Renderer& renderer) {
	renderer.fillCircle(getX(), getY(), radius, borderColor);
}

void Socket::remove() {
	for (int i = sockets.size() - 1; i >= 0; i--) {
		if (sockets[i] == this) {
			sockets.erase(sockets.begin() + i);
			break;
		}
	}
	Drawable::remove();
}

const int Knob::radius = 15;
const int Knob::notchSize = 3;

Knob::Knob(int x, int y, std::vector<float> notches) : Draggable(x, y) {
	this->left = this->right = this->top = this->bottom = new int(radius);

	this->notches = notches;
	value = 0;
}

void Knob::draw(Renderer& renderer) {
	int knobX = getX();
	int knobY = getY();

	renderer.fillCircle(knobX, knobY, radius, borderColor);

	float knobAngle = angle(value);
	float lineX = SDL_cosf(knobAngle) * radius;
	float lineY = SDL_sinf(knobAngle) * radius;
	renderer.line(knobX, knobY, knobX + lineX, knobY + lineY, textColor);

	for (float notch : notches) {
		float notchAngle = angle(notch);
		float nx = SDL_cosf(notchAngle);
		float ny = SDL_sinf(notchAngle);
		renderer.line(
			knobX + nx * radius,
			knobY + ny * radius,
			knobX + nx * (radius + notchSize),
			knobY + ny * (radius + notchSize),
			textColor
		);
	}
}

bool Knob::inDragArea(int x, int y) {
	return pointInCircle(x, y, getX(), getY(), radius);
}

bool Knob::onMouseMotion(SDL_MouseMotionEvent* evt) {
	if (Drawable::onMouseMotion(evt)) return true;

	if (dragging)
		value = SDL_clamp(value + evt->xrel / 200.0, -1, 1);

	return dragging;
}

float Knob::angle(float value) {
	return value * 3 * M_PI / 4 - M_PI / 2;
}

const int Button::radius = 10;
const int Button::border = 2;

Button::Button(int x, int y, bool toggle) : Drawable(x, y) {
	this->pressed = false;
	this->toggle = toggle;
}

void Button::draw(Renderer& renderer) {
	int x = getX();
	int y = getY();

	renderer.fillCircle(x, y, radius, borderColor);
	renderer.fillCircle(x, y, radius - border, pressed ? textColor : bgColor);
}

bool Button::onMouseDown(SDL_MouseButtonEvent* evt) {
	if (!pointInCircle(evt->x, evt->y, getX(), getY(), radius))
		return false;

	if (toggle)
		pressed = !pressed;
	else
		pressed = true;
	return true;
}

bool Button::onMouseUp(SDL_MouseButtonEvent* evt) {
	if (!toggle)
		pressed = false;
	return false;
}

int Input::textX() { return 2; };
int Input::textY() { return 25; };

Input::Input(const char* name, int x, int y, int width, int height, int socketX, int socketY) : Drawable(x, y) {
	this->name = name;

	this->width = width;
	this->height = height;

	socket = new Socket(socketX, socketY);
	addChild(socket);
}

Input::Input(const char* name, int x, int y, int width, int height) : Input(name, x, y, width, height, 15, 15) {}

void Input::draw(Renderer& renderer) {
	SDL_Rect rect{ getX(), getY(), width, height};
	renderer.strokeRect(&rect, 2, borderColor);

	renderer.renderText(getX() + textX(), getY() + textY(), name, textColor);

	Drawable::draw(renderer);
}

float Input::getValue() {
	if (socket != nullptr && socket->connector != nullptr && socket->connector->other->socket != nullptr) {
		Output* output = socket->connector->other->socket->output;
		if (output != nullptr)
			return output->value;
	}
	return 0;
}

void Input::setValue(float value) {

}


const int KnobInput::knobX = 20;
const int KnobInput::knobY = 20;

int KnobInput::textX() { return 2; };
int KnobInput::textY() { return 34; };

KnobInput::KnobInput(const char* name, int x, int y, std::vector<float> notches, float _defaultValue) : Input(name, x, y, 60, 50, 48, 12) {
	knob = new Knob(knobX, knobY, notches);
	addChild(knob);
}

float KnobInput::getValue() {
	if (socket != nullptr && socket->connector != nullptr && socket->connector->other->socket != nullptr) {
		Output* output = socket->connector->other->socket->output;
		if (output != nullptr)
			return knob->value * output->value;
	}
	return knob->value;
}

void KnobInput::setValue(float value) {
	knob->value = value;
}


const int ButtonInput::buttonX = 15;
const int ButtonInput::buttonY = 15;

int ButtonInput::textX() { return 2; };
int ButtonInput::textY() { return 30; };

ButtonInput::ButtonInput(const char* name, int x, int y, bool toggle) : Input(name, x, y, 50, 45, 38, 12) {
	button = new Button(buttonX, buttonY, toggle);
	addChild(button);
}

float ButtonInput::getValue() {
	if (socket != nullptr && socket->connector != nullptr && socket->connector->other->socket != nullptr) {
		Output* output = socket->connector->other->socket->output;
		if (output != nullptr)
			return output->value;
	}
	return button->pressed ? 1 : 0;
}

void ButtonInput::setValue(float value) {
	button->pressed = value > 0 ? 1 : 0;
}

const int Output::socketX = 10;
const int Output::socketY = 10;

Output::Output(const char* name, int x, int y, std::function<float()> nextValue) : Drawable(x, y) {
	this->name = name;
	this->nextValue = nextValue;

	socket = new Socket(socketX, socketY);
	socket->output = this;
	addChild(socket);
}

void Output::draw(Renderer& renderer) {
	renderer.renderText(getX() + 2, getY() + 20, name, textColor);
	Drawable::draw(renderer);
}

void Output::step() {
	value = nextValue();
}

const int Connector::radius = 6;
const int Connector::snapDistance = 20;

Connector::Connector(int x, int y) : Draggable(x, y) {
	this->left = this->right = this->top = this->bottom = new int(radius);

	socket = nullptr;
	other = nullptr;
}

int Connector::getDrawX() {
	return socket ? socket->getX() : getX();
}

int Connector::getDrawY() {
	return socket ? socket->getY() : getY();
}

bool Connector::inDragArea(int x, int y) {
	return pointInCircle(x, y, getDrawX(), getDrawY(), radius);
}

bool Connector::onMouseMotion(SDL_MouseMotionEvent* evt) {
	Draggable::onMouseMotion(evt);

	if (dragging) {
		for (Socket* s : Socket::sockets) {
			if (pointInCircle(s->getX(), s->getY(), getX(), getY(), snapDistance)) {
				socket = s;
				socket->connector = this;
				return true;
			}
		}
		if (socket != nullptr) {
			socket->connector = nullptr;
			socket = nullptr;
		}
	}

	return dragging;
}

void Connector::draw(Renderer& renderer) {
	if (!dragging && socket) {
		x = socket->getX();
		y = socket->getY();
	}
}


Cable::Cable(int x, int y, SDL_Color color) : Drawable(0, 0) {
	this->color = color;

	start = new Connector(x, y);
	end = new Connector(x + 15, y);

	start->other = end;
	end->other = start;

	children.push_back(start);
	children.push_back(end);
}

void Cable::draw(Renderer& renderer) {
	int sx = start->getDrawX();
	int sy = start->getDrawY();
	int ex = end->getDrawX();
	int ey = end->getDrawY();

	renderer.fillCircle(sx, sy, Connector::radius, color);
	renderer.fillCircle(ex, ey, Connector::radius, color);

	int off = abs(sx - ex) / 3;
	int points = sqrt(pow(sx - ex, 2) + pow(sy - ey, 2) / 2) / 2;
	renderer.bezier(
		sx, sy,
		sx * 0.7 + ex * 0.3, sy * 0.7 + ey * 0.3 + off,
		sx * 0.3 + ex * 0.7, sy * 0.3 + ey * 0.7 + off,
		ex, ey,
		points, 3, color
	);
	Drawable::draw(renderer);
}

bool Cable::onMouseDown(SDL_MouseButtonEvent* evt) {
	if (evt->button == SDL_BUTTON_RIGHT && (
		this->start->inDragArea(evt->x, evt->y) ||
		this->end->inDragArea(evt->x, evt->y)
	)) {
		this->queueDelete = true;
		return true;
	}

	return this->start->onMouseDown(evt) || this->end->onMouseDown(evt);
}

MenuOption::MenuOption(const char* label, std::function<void(int, int)> action) {
	this->label = label;
	this->action = action;
}

const int Menu::optionHeight = 16;
const SDL_Color Menu::optionColor{ 0x20, 0x20, 0x20 };
const SDL_Color Menu::hoverColor{ 0x30, 0x30, 0x30 };

Menu::Menu(int x, int y, int width, std::vector<MenuOption> options) : Drawable(x, y) {
	this->width = width;
	this->options = options;

	open = false;
	hovered = -1;
}

void Menu::draw(Renderer& renderer) {
	if (!open) return;

	for (int i = 0; i < options.size(); i++) {
		MenuOption& opt = options[i];
		int y = getY() + optionHeight * i;
		SDL_Rect rect(getX(), y, width, optionHeight);
		renderer.fillRect(&rect, !opt.action || i == hovered ? hoverColor : optionColor);
		if (!opt.action) renderer.strokeRect(&rect, 1, textColor);
		renderer.renderText(getX() + 2, y, opt.label, textColor);
	}
}

bool Menu::onMouseMotion(SDL_MouseMotionEvent* evt) {
	if (!open) return false;
	SDL_Rect rect{ getX(), getY(), width, options.size() * optionHeight };
	hovered = pointInRect(evt->x, evt->y, &rect)
		? (evt->y - getY()) / optionHeight
		: -1;
	return true;
}

bool Menu::onMouseDown(SDL_MouseButtonEvent* evt) {
	if (!open) return false;
	SDL_Rect rect{ getX(), getY(), width, options.size() * optionHeight };
	if (evt->button == SDL_BUTTON_LEFT && pointInRect(evt->x, evt->y, &rect)) {
		int i = (evt->y - getY()) / optionHeight;
		if (options[i].action != nullptr)
			options[i].action(getX(), getY());
	}
	// Reset
	hovered = -1;
	open = false;
	return true;
}

bool Menu::onMouseUp(SDL_MouseButtonEvent* evt) {
	return open;
}

const int EditText::padding = 2;
const int EditText::height = 20;
const int EditText::doubleClickDelay = 300;

EditText::EditText(int x, int y, int width, const char* text) : Drawable(x, y) {
	this->width = width;
	this->text = std::string(text);
	cursor = 0;
	editing = false;
	clickTime = 0;
}

void EditText::draw(Renderer& renderer) {
	SDL_Rect r{ getX(), getY(), width, height };
	if (editing)
		renderer.fillRect(&r, bgColor);

	renderer.renderText(getX() + padding, getY() + padding, text.c_str(), textColor);

	if (editing) {
		renderer.strokeRect(&r, 1, textColor);

		int cursorPos = renderer.measureText(text.substr(0, cursor).c_str());
		int x = getX() + padding + cursorPos;
		int y = getY() + padding;
		renderer.line(x, y, x, y + height - padding * 2, textColor);
	}
}

bool EditText::onMouseDown(SDL_MouseButtonEvent* evt) {
	SDL_Rect r{ getX(), getY(), width + padding, height };
	if (evt->button == SDL_BUTTON_LEFT && pointInRect(evt->x, evt->y, &r)) {
		long t = getMs();
		if (editing) {
			return true;
		} else if ((clickTime > 0 && clickTime + doubleClickDelay > t)) {
			editing = true;
			clickTime = 0;
			SDL_StartTextInput();
			cursor = text.size();
			return true;
		} else {
			clickTime = t;
			return false;
		}
	}
	SDL_StopTextInput();
	editing = false;
	clickTime = 0;
	return false;
}

bool EditText::onKeyDown(SDL_KeyboardEvent* evt) {
	if (editing) {
		int code = evt->keysym.sym;
		if (code == SDLK_LEFT)
			cursor = std::max(cursor - 1, 0);
		else if (code == SDLK_RIGHT)
			cursor = std::min(cursor + 1, (int)text.size());
		else if (code == SDLK_BACKSPACE && cursor > 0) {
			text.erase((size_t)cursor - 1, 1);
			cursor--;
		}
		else if (code == SDLK_DELETE)
			text.erase(cursor, 1);
		else if (code == SDLK_RETURN) {
			SDL_StopTextInput();
			editing = false;
		}
	}

	return editing;
}

bool EditText::onTextInput(SDL_TextInputEvent* evt) {
	if (editing) {
		text.insert(cursor, evt->text);
		cursor += strlen(evt->text);
	}

	return editing;
}