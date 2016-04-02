#ifndef _MEMORY_FUNCTIONS_H_
#define _MEMORY_FUNCTIONS_H_

#include <sstream>
using std::wostringstream;

class Game
{
	short grid[3][3];
	bool turn;
	float move[5];
	short score[3];
	char Do;
	bool valid;
	bool Com;
	bool ult;
	short gridX, gridY;


public:
	Game();
	~Game();
	bool CheckGrid(short X, short Y);
	char WinnerCheck(char& WinLine, bool add = true);
	void DrawMark();
	const short* GetScore();
	void Reset(bool fullRest = false);
	float* GetMove();
	void AI(bool On);
	bool AITurn();
	short* GetGrid() const;
	void Cheater(char cheat);
	bool WhoseTurn() const { return turn; }
	bool OnFireCheck() const;


};
#endif