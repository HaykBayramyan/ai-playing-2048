#pragma once
#include <vector>
#include <random>

class Game2048 {
public:
    Game2048(int size = 4);

    void reset();
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();

    bool isWin() const;
    bool isGameOver() const;
    int  score() const { return m_score; }
    int  size()  const { return m_n; }

    int  at(int r, int c) const { return m_board[r][c]; }
    const std::vector<std::vector<int>>& board() const { return m_board; }

private:
    int m_n;
    int m_score;
    std::vector<std::vector<int>> m_board;

    std::mt19937 m_rng;

    void spawnRandomTile();
    bool canMergeOrMove() const;
    bool slideAndMergeRowLeft(std::vector<int>& row);
};
