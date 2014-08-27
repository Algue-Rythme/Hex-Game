#include "Agent.hpp"
#include "Game.hpp"
#include "Utils.hpp"
#include "UserView.hpp"

using namespace sf;

Game::Game(unsigned int _size, Player beginner) : 
           size(_size), data(_size), currentPlayer(beginner), finish(false),
		   view(nullptr), agent(new Agent(this, &data, size)) 
{ }

void Game::addEvent(GameEvent event)
{
	protectEvents.lock();
	events.push(event);
	protectEvents.unlock();
}

void Game::launch()
{
	view = new UserView(this, size);
	
	while (view->isOpen())
	{
		if (currentPlayer == Player::Human || finish)
			view->readUserInput();
		else
			agent->UCT();
		
		processEvent();
		
		if (!finish && data.winner() != Player::Empty)
			finish = true;
	}
}

void Game::processEvent()
{
	protectEvents.lock();
	
	GameEvent event;
	while (!events.empty())
	{
		event = events.front();
		events.pop();
		
		if (event.type == GameEventType::Closed)
			view->close();
			
		if (event.type == GameEventType::Move 
		 && data.isEmpty(event.position)
		 && event.player == currentPlayer
		 && !finish)
		{
			data.makeMove(event.position, currentPlayer);
			view->setColor(event.position, currentPlayer);
			agent->pruning(event.position, currentPlayer);
			currentPlayer = nextPlayer(currentPlayer);
		}
	}
	
	protectEvents.unlock();
}

Game::~Game()
{
	delete view;
	delete agent;
}
