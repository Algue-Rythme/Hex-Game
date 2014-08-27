#include <algorithm>
#include <cmath>
#include <chrono>
#include <fstream>
#include <random>
#include <vector>

#include "Agent.hpp"
#include "Game.hpp"

using namespace std;
using namespace sf;

#define LOG_TXT

#ifdef LOG_TXT
ofstream out("log.txt");
#endif

static const unsigned int seed = chrono::system_clock::now().time_since_epoch().count();
static default_random_engine randGenerator(seed);
unsigned int nbDescents = 32*1000;
unsigned int Tree::count = 0;
unsigned int nbSimulations = 0;

CompareTree::CompareTree(double _cUCT) : cUCT(_cUCT) 
{}
 
bool CompareTree::operator()(Tree * left, Tree * right) const
{
	return left->UCT(cUCT) < right->UCT(cUCT);
}

Agent::Agent(Game * father, Data * _data, unsigned int _size) : game(father), size(_size), data(_data), tree(nullptr)
{
	#ifdef LOG_TXT
	out.precision(2);
	out.setf(ios::fixed, ios::floatfield);
	#endif
}

void Agent::pruning(Vector2u move, Player player)
{
	if (tree == nullptr)
		return ;  
		
	#ifdef LOG_TXT
	out.width(6);
	out << Tree::count << "\t";
	#endif
		  
	Tree * child = tree->findChild(move);
	tree->clear(child);
	
	#ifdef LOG_TXT
	out.width(6);
	out << Tree::count << "\t";
	out.width(6);
	if (child != nullptr)
	{
		double ratio = child->UCT(0);
		if (player != Player::AI)
			ratio = 1.0 - ratio;
		out << ratio*100.0 << " %\t";
	}
	else
		out << "?" << " %\t";
	out << "Player : " << (player == Player::AI ? "AI" : "Human") << "\n";
	#endif
	
	delete tree;
	tree = child;
}

void Agent::UCT()
{
	if (tree == nullptr)
		tree = new Tree(nullptr, *data, Vector2u(0, 0), Player::AI);
	tree->father = nullptr;
		
	Clock clock;
	while (clock.getElapsedTime() < seconds(2.5f))
	{
		Data board = *data;
		Tree * sheet = selection(tree, board, Player::AI);
		Player winner = board.MonteCarlo(sheet->move, nextPlayer(sheet->player));
		reachBack(sheet, winner);
	}
	
	auto child = *(max_element(tree->childs.begin(), tree->childs.end(), CompareTree(0)));
	Vector2u bestMove = child->move;
	game->addEvent(GameEvent(GameEventType::Move, bestMove, Player::AI));  
}

Agent::~Agent()
{
	#ifdef LOG_TXT
	unsigned int unites = nbSimulations % 1000;
	unsigned int milliers = (nbSimulations / 1000) % 1000;
	unsigned int millions = (nbSimulations / 1000000);
	out.fill('0');
	out << "\n\tNumber of simulations : ";
	out.width(3);	
	out << millions << " ";
	out.width(3);
	out << milliers << " ";
	out.width(3);
	out << unites << "\n";
	#endif
	
	tree->clear(nullptr);
	delete tree;
}

Tree * selection(Tree * tree, Data& data, Player current)
{
	if (tree->noMoves.empty() && tree->childs.empty())
		return tree;
		
	if (!tree->noMoves.empty())
	{
		Vector2u randomMove = tree->noMoves.back();
		tree->noMoves.pop_back();
		data.makeMove(randomMove, current);
		
		Tree * sheet = new Tree(tree, data, randomMove, current);
		tree->childs.push_back(sheet);
		
		return sheet;
	}

	tree = *(max_element(tree->childs.begin(), tree->childs.end(), CompareTree(0.5)));
	data.makeMove(tree->move, current);

	return selection(tree, data, nextPlayer(current));
}

void reachBack(Tree * tree, Player current)
{
	while (tree != nullptr)
	{
		tree->nbSimulations += 1;
		if (current == tree->player)
			tree->nbWins += 1;
		tree = tree->father;
	}
}

Tree::Tree(Tree * _father, const Data& data, Vector2u _move, Player current) :
father(_father), move(_move), player(current), nbWins(0), nbSimulations(0)
{ 
	noMoves = data.moves();
	shuffle(noMoves.begin(), noMoves.end(), randGenerator);
	++count;
}

double Tree::UCT(double cUCT) const
{
	return 
		double(nbWins) / double(nbSimulations) + 
		cUCT * sqrt(log(double(father->nbSimulations)) / double(nbSimulations));
}

Tree * Tree::findChild(Vector2u move) const
{
	for (auto child : childs)
		if (child->move == move)
			return child;
	return nullptr;
}

void Tree::clear(Tree * chosen)
{
	for (auto child : childs)
	{
		if (child != chosen)
		{
			child->clear(nullptr);
			delete child;
		}
	}
}

Tree::~Tree()
{ 
	--count;
}
