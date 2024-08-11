#include "audioConfig.h"
#include "Module.h"
#include "util.h"

const int Module::borderWidth = 3;
const int Module::headerHeight = 20;

Module::Module(const char* name, int w, int h, int x, int y, bool deletable) : Draggable(x, y) {
	this->width = w;
	this->height = h;
	this->right = &this->width;
	this->bottom = &this->height;

	this->deletable = deletable;

	title = new EditText(0, 0, std::min(100, deletable ? w - 20 : w), name);
	addChild(title);

	queueDelete = false;
}

void Module::step() {
	for (Drawable* d : children) {
		Output* o = dynamic_cast<Output*>(d);
		if (o != nullptr)
			o->step();
	}
};

void Module::draw(Renderer& renderer) {
	renderer.fillRect(new SDL_Rect{ getX(), getY(), width, height }, borderColor);

	renderer.fillRect(new SDL_Rect{
		getX() + borderWidth,
		getY() + headerHeight,
		width - borderWidth * 2,
		height - borderWidth - headerHeight,
	}, bgColor);

	if (deletable) {
		renderer.line(
			getX() + width - headerHeight + 5, getY() + 5,
			getX() + width - 5, getY() + headerHeight - 5,
			textColor
		);
		renderer.line(
			getX() + width - headerHeight + 5, getY() + headerHeight - 5,
			getX() + width - 5, getY() + 5,
			textColor
		);
	}

	Draggable::draw(renderer);
}

bool Module::inDragArea(int x, int y) {
	return pointInRect(x, y, new SDL_Rect{ getX(), getY(), width, headerHeight });
}

void Module::remove() {
	queueDelete = true;
	Drawable::remove();
}

bool Module::onMouseDown(SDL_MouseButtonEvent* evt) {
	if (deletable && evt->button == SDL_BUTTON_LEFT) {
		SDL_Rect xRect{ getX() + width - headerHeight, getY(), headerHeight, headerHeight };
		if (pointInRect(evt->x, evt->y, &xRect)) {
			remove();
			return true;
		}
	}

	if (Draggable::onMouseDown(evt)) return true;

	return pointInRect(evt->x, evt->y, new SDL_Rect{ getX(), getY(), width, height });
}

WaveGenerator::WaveGenerator(int x, int y) : Module("VCO", 150, 130, x, y) {
	freq = new KnobInput("  freq", 10, headerHeight + 10, std::vector<float>{ -1, -0.67, -0.33, 0, 0.33, 0.67, 1 });
	addChild(freq);

	type = new KnobInput("  type", 80, headerHeight + 10, std::vector<float>{ -1, 0, 1 });
	addChild(type);

	output = new Output("out", 50, headerHeight + freq->height + 15, [this]() {
		float t = type->getValue();
		if (t < 0)
			return (float) (phase > 0.5 ? -1 : 1);
		return SDL_sinf(2 * M_PI * phase);
	});
	addChild(output);

	phase = 0;
}

void WaveGenerator::step() {
	Module::step();

	phase += 440 * SDL_powf(2, freq->getValue() * 3) / SAMPLE_RATE;
	if (phase > 1) phase -= 1;
}

Player::Player(int x, int y) : Module("Player", 80, 90, x, y, false) {
	input = new KnobInput(" input", 10, headerHeight + 10);
	addChild(input);
}

const int Scope::bufferLength = 65;

Scope::Scope(int x, int y) : Module("Scope", 150, 150, x, y) {
	input = new Input("input", 15, headerHeight + 10, 40, 40);
	addChild(input);

	rate = new KnobInput("  rate", 75, headerHeight + 10);
	addChild(rate);

	n = 0;
}

void Scope::step() {
	int r = pow(10, rate->getValue() + 1);
	n++;
	if (n > r) {
		buffer.push_back(input->getValue());
		if (buffer.size() > bufferLength)
			buffer.erase(buffer.begin());
		n = 0;
	}
};

void Scope::draw(Renderer& renderer) {
	Module::draw(renderer);

	int viewX = getX() + 10;
	int viewY = getY() + headerHeight + 70;

	renderer.fillRect(new SDL_Rect{ viewX, viewY, 130, 50 }, SDL_Color(0, 0, 0));

	SDL_Point points[bufferLength]{};
	for (int i = 0; i < bufferLength; i++)
		points[i] = SDL_Point(
			viewX + 2 * i,
			viewY + 25 - (i < buffer.size() ? buffer[i] * 25 : 0)
		);
	renderer.lines(points, bufferLength, SDL_Color(0xF4, 0xF1, 0x86));
};

BitCrusher::BitCrusher(int x, int y) : Module("BitCrusher", 130, 130, x, y) {
	input = new Input("input", 10, headerHeight + 10, 40, 40);
	addChild(input);

	depth = new KnobInput(" depth", 60, headerHeight + 10, std::vector<float>{ -1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1 });
	addChild(depth);

	output = new Output("out", 40, headerHeight + depth->height + 15, [this]() {
		float bits = pow(2, (depth->getValue() + 1) * 4);
		return std::min(std::max(round(input->getValue() * bits) / bits, -1.f), 1.f);
	});
	addChild(output);
}

ADSR::ADSR(int x, int y) : Module("ADSR", 290, 190, x, y) {
	attack = new KnobInput(" attack", 10, headerHeight + 60);
	addChild(attack);

	decay = new KnobInput(" decay", 80, headerHeight + 60);
	addChild(decay);

	sustain = new KnobInput("sustain", 150, headerHeight + 60);
	addChild(sustain);

	release = new KnobInput("release", 220, headerHeight + 60);
	addChild(release);

	pressed = false;
	pressTime = 0;
	pressValue = 0;
	releaseTime = SAMPLE_RATE * 2;
	releaseValue = 0;

	trigger = new ButtonInput("trigger", 80, headerHeight + attack->height + 70);
	addChild(trigger);

	output = new Output("out", 170, headerHeight + attack->height + 75, [this]() {
		float atk = (attack->getValue() + 1) / 2;
		float dec = (decay->getValue() + 1) / 2;
		float rel = (release->getValue() + 1) / 2;
		float sus = (sustain->getValue() + 1) / 2;

		if (pressed) {
			pressValue = pressTime / SAMPLE_RATE < atk ? (1 - releaseValue) * pressTime / (SAMPLE_RATE * atk) + releaseValue :
				pressTime / SAMPLE_RATE < atk + dec ? sus + (1 - sus) * (atk + dec - pressTime / SAMPLE_RATE) / dec :
				sus;
			return pressValue;
		} else {
			releaseValue = releaseTime / SAMPLE_RATE < rel ? pressValue * (rel - releaseTime / SAMPLE_RATE) / rel : 0;
			return releaseValue;
		}
	});
	addChild(output);
}

const float Delay::delayMax = 1.0;

Delay::Delay(int x, int y) : Module("Delay", 130, 130, x, y) {
	input = new Input("input", 10, headerHeight + 10, 40, 40);
	addChild(input);

	amount = new KnobInput("amount", 60, headerHeight + 10, std::vector<float>{ -1, 1 });

	addChild(amount);

	maxSampleStored = SAMPLE_RATE * delayMax;
	buffer = new float[maxSampleStored];
	transitionBuffer = new float[maxSampleStored];
	writeIndex = 0;
	storedDelayOffset = 0;

	odd = false;

	readIndex = amount->getValue();

	output = new Output("out", 40, headerHeight + amount->height + 15, [this]() {
		int newDelay = getSampleOffset();
		int delayDiff = newDelay - storedDelayOffset;
		storedDelayOffset = newDelay;


		int finalIndex = (writeIndex - storedDelayOffset + maxSampleStored) % maxSampleStored;
		return buffer[finalIndex];
		});
	addChild(output);
}

int Delay::getSampleOffset() {

	return ((amount->getValue() + 1) / 2) * (maxSampleStored-1);
}

void Delay::step() {
	Module::step();

	writeIndex = (writeIndex + 1) % maxSampleStored;
	buffer[writeIndex] = input->getValue();
}

void ADSR::step() {
	Module::step();

	bool p = trigger->getValue() > 0;

	if (!pressed && p) {
		pressTime = 0;
		releaseTime = 0;
	}

	pressed = p;

	if (pressed)
		pressTime++;
	else
		releaseTime++;
}

void ADSR::draw(Renderer& renderer) {
	Module::draw(renderer);

	int x = getX() + 10;
	int y = getY() + headerHeight + 10;
	int w = 270;
	int h = 40;
	
	float atk = (attack->getValue() + 1) / 2;
	float dec = (decay->getValue() + 1) / 2;
	float rel = (release->getValue() + 1) / 2;
	float sus = (sustain->getValue() + 1) / 2;
	float total = atk + dec + 1 + rel;

	SDL_Point points[5]{
		SDL_Point(x, y + h),
		SDL_Point(x + w * atk / 4, y),
		SDL_Point(x + w * (atk + dec) / 4, y + (1 - sus) * h),
		SDL_Point(x + w * (total - rel) / 4, y + (1 - sus) * h),
		SDL_Point(x + w * total / 4, y + h)
	};
	renderer.lines(points, 5, textColor);
}

Mixer::Mixer(int x, int y) : Module("Mixer", 130, 130, x, y) {
	input = new Input("input", 10, headerHeight + 10, 40, 40);
	addChild(input);

	volume = new KnobInput("volume", 60, headerHeight + 10);
	addChild(volume);

	output = new Output("out", 40, headerHeight + volume->height + 15, [this]() {
		return input->getValue() * volume->getValue();
	});
	addChild(output);
}