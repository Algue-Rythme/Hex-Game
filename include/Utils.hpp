#ifndef UTILS_HPP
#define UTILS_HPP

#include <complex>

#include <SFML/System.hpp>

enum class Player
{
	Empty,
	AI,
	Human
};

enum class GameEventType
{
	Move,
	Closed
};

struct Edge
{
	Edge()
	{}
	Edge(Player _player, sf::Vector2u coor = sf::Vector2u(0, 0)) :
						player(_player), position(coor)
	{}
	
	Player player;
	sf::Vector2u position;
};

struct GameEvent
{
	GameEvent() 
	{}
	GameEvent(GameEventType eventType, sf::Vector2u coor = sf::Vector2u(0, 0), Player _player = Player::Empty) : 
	                   type(eventType), position(coor), player(_player)
	{}
	
	GameEventType type;
	sf::Vector2u position;
	Player player;
};

inline Player nextPlayer(Player player)
{
	if (player == Player::Human)
		return Player::AI;
	if (player == Player::AI)
		return Player::Human;
	return Player::Empty;
}

template<typename T>
inline sf::Vector2<T> makeVector2(std::complex<T> value)
{
	return sf::Vector2<T>(value.real(), value.imag());
}

template<typename T>
struct CompareVector2
{
	bool operator()(const sf::Vector2<T>& left, const sf::Vector2<T>& right) const
	{
		if (left.x != right.x)
			return (left.x < right.x);
		return (left.y < right.y);
	}
};

#endif