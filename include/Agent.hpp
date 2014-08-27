#ifndef AGENT_HPP
#define AGENT_HPP

#include <vector>

#include "Data.hpp"
#include "Utils.hpp"

class Game;
class Tree;

class CompareTree
{
	public:

		CompareTree(double);
		bool operator()(Tree * left, Tree * right) const;
	
	private:
	
		double cUCT;
};

class Tree
{
	public: 
		
		Tree(Tree *, const Data&, sf::Vector2u, Player);
		double UCT(double) const;
		void clear(Tree *);
		Tree * findChild(sf::Vector2u) const;
		~Tree();
	
		Tree * father;
		sf::Vector2u move;
		Player player;
		std::vector<Tree*> childs;
		std::vector<sf::Vector2u> noMoves;
		unsigned int nbWins;
		unsigned int nbSimulations;
		
		static unsigned int count;
};

class Agent
{
	public:
	
		Agent(Game *, Data *, unsigned int);
		
		void UCT();
		void pruning(sf::Vector2u, Player);
		
		~Agent();
		
	private:
		
		Game * game;
		unsigned int size;
		Data * data;
		Tree * tree;
};

Tree * selection(Tree *, Data&, Player);
void reachBack(Tree *, Player);

#endif