#ifndef DATA_HPP
#define DATA_HPP

#include <set>
#include <vector>

#include "Utils.hpp"

class Data
{
	public:
	
		Data(unsigned int);
		
		Player winner() const;
		
		Player operator()(unsigned int, unsigned int) const;
		Player operator()(sf::Vector2u) const;
		
		Player MonteCarlo(sf::Vector2u, Player);
		void makeMove(sf::Vector2u position, Player player);
		std::vector<sf::Vector2u> moves() const;
		bool isEmpty(sf::Vector2u) const;
		
	private:
	
		Player& operator()(unsigned int, unsigned int);
		Player& operator()(sf::Vector2u);
		void disconnect(sf::Vector2u, Player, sf::Vector2u&) const;
		bool diamondPattern(sf::Vector2i, sf::Vector2i, sf::Vector2i, Player) const;
		bool correct(sf::Vector2i) const;
	
		std::vector<sf::Vector2u> neighboursCoor(sf::Vector2u) const;
	
		unsigned int size;
		std::vector<Player> board;
		std::set<sf::Vector2u, CompareVector2<unsigned>> movesList;
};

#endif
