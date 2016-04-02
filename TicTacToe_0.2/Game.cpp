#include "Game.h"

Game::Game()
{
	int increm = 0;
	for(int x = 0; x < 3; x++)
		for(int y = 1; y < 4; y++)
			grid[x][y-1] = -1 - increm++;
	turn = true;
	score[0] = score[1] = score[2] = 0;
	valid = false;
	gridX = gridY = -1;
	move[0] = -1;
	Com = false;
	for(int x = 1; x < 5; x++)
		move[x] = 0;
	ult = false;
	Do = 0;
}

Game::~Game()
{

}

bool Game::AITurn()
{
	if(Com)
	{
		bool validSpot = false;
		int x = -1, y = -1;
		while(!validSpot)
		{
			x =rand()%3; y = rand()%3;
			if(grid[x][y] < 0)
			{
				grid[x][y] = turn;
				turn = !turn;
				validSpot = true;
				valid = false;
			}
		}
		return true;
	}
	return false;
}

short* Game::GetGrid() const
{
	short temp[9] = {0};
	short increm = 0;
	for(int x = 0; x < 3; x++)
		for(int y = 0; y < 3; y++)
			temp[increm++] = grid[x][y];
	return temp;
}

void Game::Cheater(char cheat)
{
	switch (cheat)
	{
	case 0:
		{
			for(int x = 0; x < 3; x++)
				for(int y = 0; y < 3; y++)
				{
					if(grid[x][y] >= 0)
						grid[x][y] = !grid[x][y];
				}
		}
		break;
	case 1:
		ult = !ult;
		break;
	case 2:
		{
			int increm = -1;
			for(int x = 0; x < 3; x++)
				for(int y = 0; y < 3; y++)
				{
					if(grid[x][y] != (short)turn && grid[x][y] >= 0)
						grid[x][y] = increm--;
				}
		}
		break;
	}
}

void Game::DrawMark()
{
	move[0] = turn;

	move[1] = 100.0f + gridX*200.0f;
	move[2] = 100.0f + gridY*200.0f;
	move[3] = 300.0f + gridX*200.0f;
	move[4] = 300.0f + gridY*200.0f;
	grid[gridX][gridY] = turn;
	turn = !turn;
	valid = false;

}

float* Game::GetMove()
{
	return move;
}

bool Game::CheckGrid(short X, short Y)
{

	if(X != -1)
	{
		if (grid[X][Y] < 0 || ult)
		{

			valid = true;
			ult = false;
			gridX = X;
			gridY = Y;
			return true; 
		}
	}

	valid = false;
	return false;
}

const short* Game::GetScore()

{
	return score;
}

char Game::WinnerCheck(char& WinLine, bool add)
{
	int player = -1;
	if((grid[0][0] == grid[0][1] && grid[0][2] == grid[0][1]))
		WinLine |= 1;
	if(grid[1][0] == grid[1][1] && grid[1][2] == grid[1][1])
		WinLine |= 2;
	if((grid[2][0] == grid[2][1] && grid[2][2] == grid[2][1]))
		WinLine |= 4;
	if(grid[0][0] == grid[1][0] && grid[1][0] == grid[2][0])
		WinLine |= 8;
	if(grid[0][1] == grid[1][1] && grid[1][1] == grid[2][1])
		WinLine |= 16;
	if(grid[0][2] == grid[1][2] && grid[1][2] == grid[2][2])
		WinLine |= 32;
	if(grid[0][0] == grid[1][1] && grid[1][1] == grid[2][2])
		WinLine |= 64;
	if(grid[2][0] == grid[1][1] && grid[1][1] == grid[0][2])
		WinLine |= 128;

	if(WinLine != 0)
	{

		(!turn)? player = 1 : player = 2;
		if (add)
		{
			score[player-1] += 1; 

			//oss << "Player " << player << " wins!!\nWould you like to play again?";
			if(player == 1)
				Do += 1;
			else
				Do -= 1;
		}
		return player;
	}
	else
	{
		bool inGame = true;
		for(int x = 0; x < 3; x++)
			for(int y = 0; y < 3; y++)
				if(grid[x][y] < 0)
					inGame = false;
		if(inGame)
		{
			if (add)
			{
				score[2] += 1;
				//oss << "It's a draw...\nWould you like to play again?";
				Do -= 1; 
			}
			return player;
		}
	}
	return -11;
}

void Game::Reset(bool fullReset)
{
	int increm = 0;
	for(int x = 0; x < 3; x++)
		for(int y = 1; y < 4; y++)
		{
			grid[x][y-1] = -1 - increm++;
			//Grid[x][y-1] = -1;
		}
		turn = true;
		valid = false;
		gridX = gridY = -1;
		if(fullReset)
			for(int x = 0; x < 3; x++)
				score[x] = 0;
}

void Game::AI(bool On)
{
	Com = On;
}

bool Game::OnFireCheck() const
{
	if(Do >= 3 && score[0] >= score[1]+3 && score[0] >= score[2]+3)
		return true;
	return false;
}
