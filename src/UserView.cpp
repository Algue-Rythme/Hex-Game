#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <random>

#include "Game.hpp"
#include "RenderingView.hpp"
#include "UserView.hpp"
#include "Utils.hpp"

using namespace sf;
using namespace std;

static const float sqrt3 = sqrt(3);
static const float pi = atan(1) * 4.0;
static const complex<float> twoHours = polar<float>(1.0, pi / 3.0);
static const Color AIColor(80, 80, 80, 255);

UserView::UserView(Game * father, unsigned int dataSize) : 
 window(nullptr), rendering(nullptr), game(father), size(dataSize)
{ 
	width = VideoMode::getDesktopMode().width;
	height = VideoMode::getDesktopMode().height;
	float widthSize = float(width) / float(2*size + size - 1);
	float heightSize = float(height) / float(sqrt3 * size);
	cellSize = min(widthSize, heightSize);
	hexagonSize = (14./15.) * cellSize;

	stoneTexture.loadFromFile("stone.png");
	stoneTexture.setSmooth(true);
	initBackground();
	initHexagons();
	initSide();

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	window = new RenderWindow(VideoMode::getDesktopMode(), "Jeu de Hex", Style::None, settings);
	window->setActive(false);
	
	rendering = new Thread(RenderingView(window, this));
	rendering->launch();
}

bool UserView::collision(unsigned int xMouse, unsigned int yMouse, Vector2u& position) const
{
	Lock lock(protectVertices);
	for (unsigned int y = 0; y < size; ++y)
	{
		for (unsigned int x = 0; x < size; ++x)
		{
			const Vertex * hexagon = &vertices[index(x, y)];
			float deltaX = hexagon[0].position.x - float(xMouse);
			float deltaY = hexagon[0].position.y - float(yMouse);
			if (sqrt(deltaX*deltaX + deltaY*deltaY) <= (sqrt3/2.)*hexagonSize)
			{
				position = Vector2u(x, y);
				return true;
			}
		}
	}
	
	return false;
}

void UserView::close()
{
	rendering->terminate();
	if (window != nullptr)
		window->close();
}

Sprite UserView::createStone(Vector2f position) const
{
	return createStone(position.x, position.y);
}

Sprite UserView::createStone(float x, float y) const
{
	Sprite stone;
	stone.setTexture(stoneTexture);
	stone.setOrigin(Vector2f(stoneTexture.getSize().x, stoneTexture.getSize().y) * 0.5f);
	stone.setScale(1.45 * (cellSize / float(stoneTexture.getSize().x)), 
	               1.45 * (cellSize / float(stoneTexture.getSize().y)));
	stone.setPosition(x, y);
	return stone;
}

void UserView::draw(RenderTarget& target, RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(background, states);
	states.texture = &cellTexture;
	protectVertices.lock();
	target.draw(vertices, states);
	for (auto& stone : stones)
		target.draw(stone, states);
	protectVertices.unlock();
}

unsigned int UserView::index(unsigned int x, unsigned int y) const
{
	return (y * size + x) * 6 * 3;
}

void UserView::initBackground()
{
	backgroundTexture.loadFromFile("wood1.jpg");
	background.setTexture(backgroundTexture);
	unsigned int xSize = backgroundTexture.getSize().x;
	unsigned int ySize = backgroundTexture.getSize().y;
	background.setScale(float(width) / float(xSize), float(height) / float(ySize));
}

void UserView::initHexagons()
{	
	cellTexture.loadFromFile("wood2.jpg");
	cellTexture.setSmooth(true);
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	minstd_rand0 generator(seed);
	
	vertices.resize(size * size * 6 * 3);
	vertices.setPrimitiveType(Triangles);
	
	float xBegin = (width - cellSize*(2*size + size - 1)) / 2.0 + cellSize;
	float yBegin = height / 2.0;
	complex<float> line(xBegin, yBegin);
	for (unsigned int y = 0; y < size; ++y)
	{
		complex<float> column = line;
		for (unsigned int x = 0; x < size; ++x)
		{
			complex<float> center = column;
			complex<float> needle = polar<float>(hexagonSize, 0);
			
			unsigned int xRand = generator()%(cellTexture.getSize().x - 2*unsigned(cellSize)) + cellSize;
			unsigned int yRand = generator()%(cellTexture.getSize().y - 2*unsigned(cellSize)) + cellSize;
			complex<float> centerTexture(xRand, yRand);
			
			Vertex * hexagon = &vertices[index(x, y)];
			for (unsigned int vertice = 0; vertice < 6; ++vertice)
			{
				Vertex * triangle = &hexagon[vertice * 3];
				triangle[0].position = makeVector2<float>(center);
				triangle[1].position = makeVector2<float>(center + needle);
				triangle[2].position = makeVector2<float>(center + needle * twoHours);
				
				triangle[0].texCoords = makeVector2<float>(centerTexture);
				triangle[1].texCoords = makeVector2<float>(centerTexture + needle);
				triangle[2].texCoords = makeVector2<float>(centerTexture + needle * twoHours);
				
				needle *= twoHours;
			}
			
			column += polar<float>(sqrt3 * cellSize, pi / 6.0);
		}
		line += polar<float>(sqrt3 * cellSize, -pi / 6.0);
	}
}

void UserView::initSide()
{
	float left = vertices[index(size/3, 0)].position.x;
	float right = vertices[index(size - size/3 - 1, size-1)].position.x;
	float top = vertices[index(0, size - size/4 - 1)].position.y;
	float bottom = vertices[index(size-1, size/4)].position.y;
	
	stones.push_back(createStone(left, top));
	stones.push_back(createStone(right, bottom));
	
	stones.push_back(createStone(left, bottom));
	stones.back().setColor(AIColor);
	stones.push_back(createStone(right, top));
	stones.back().setColor(AIColor);
}

bool UserView::isOpen() const
{
	return window->isOpen();
}

bool UserView::readUserInput()
{
	Event event;
	while (window->waitEvent(event))
	{
		if (event.type == Event::Closed
		|| (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape))
		{
			game->addEvent(GameEvent(GameEventType::Closed));
			return true;
		}
		
		if (event.type == Event::MouseButtonPressed
		&& (event.mouseButton.button == Mouse::Left))
		{
			Vector2u position(0, 0);
			if (collision(event.mouseButton.x, event.mouseButton.y, position))
			{
				game->addEvent(GameEvent(GameEventType::Move, position, Player::Human));
				return true;
			}
		}
	}
	
	return false;
}

void UserView::setColor(Vector2u position, Player player)
{
	setColor(position.x, position.y, player);
}

void UserView::setColor(unsigned int x, unsigned int y, Player player)
{
	protectVertices.lock();
	const Vertex * hexagon = &vertices[index(x, y)];
	Sprite stone = createStone(hexagon[0].position);
	stone.setPosition(hexagon[0].position);
	if (player == Player::AI)
		stone.setColor(AIColor);
	stones.push_back(stone);
	protectVertices.unlock();
}

UserView::~UserView()
{
	close();
	delete window;
	delete rendering;
}
