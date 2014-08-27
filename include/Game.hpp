#ifndef GAME_HPP
#define GAME_HPP

#include <queue>

#include <SFML/System.hpp>

#include "Data.hpp"
#include "Utils.hpp"

class UserView;
class GameEvent;
class Data;
class Agent;

class Game
{
	public:
	
		Game(unsigned int, Player);
		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;
		
		void addEvent(GameEvent event);
		void launch();
		
		~Game();
	
	private:
	
		void processEvent();
	
		unsigned int size;
		Data data;
		Player currentPlayer;
		bool finish;
		UserView * view;
		Agent * agent;
		std::queue<GameEvent> events;
		sf::Mutex protectEvents;
};

#endif