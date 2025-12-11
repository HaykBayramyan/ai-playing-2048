#include "game2048.h"
#include <algorithm>

Game2048::Game2048(int size)
    : m_n(size), m_score(0), m_board(size, std::vector<int>(size, 0)) {
    std::random_device rd;
    m_rng.seed(rd());
    reset();
}

void Game2048::reset() {
    m_score = 0;
    for (auto& row : m_board)
        std::fill(row.begin(), row.end(), 0);

    spawnRandomTile();
    spawnRandomTile();
}

bool Game2048::slideAndMergeRowLeft(std::vector<int>& row) {
    bool changed = false;
    std::vector<int> original = row;
    std::vector<int> tmp;
    tmp.reserve(m_n);
    for (int v : row) if (v != 0) tmp.push_back(v);
    std::vector<int> merged;
    merged.reserve(m_n);
    for (int i = 0; i < (int)tmp.size(); ++i) {
        if (i + 1 < (int)tmp.size() && tmp[i] == tmp[i + 1]) {
            int nv = tmp[i] * 2;
            merged.push_back(nv);
            m_score += nv;
            ++i;
        } else {
            merged.push_back(tmp[i]);
        }
    }
    row.assign(m_n, 0);
    for (int i = 0; i < (int)merged.size(); ++i)
        row[i] = merged[i];

    if (row != original) changed = true;
    return changed;
}

bool Game2048::moveLeft() {
    bool changed = false;
    for (auto& row : m_board)
        changed |= slideAndMergeRowLeft(row);

    if (changed) spawnRandomTile();
    return changed;
}

bool Game2048::moveRight() {
    bool changed = false;
    for (auto& row : m_board) {
        std::reverse(row.begin(), row.end());
        changed |= slideAndMergeRowLeft(row);
        std::reverse(row.begin(), row.end());
    }
    if (changed) spawnRandomTile();
    return changed;
}

bool Game2048::moveUp() {
    bool changed = false;
    for (int c = 0; c < m_n; ++c) {
        std::vector<int> col(m_n);
        for (int r = 0; r < m_n; ++r) col[r] = m_board[r][c];

        bool colChanged = slideAndMergeRowLeft(col);
        changed |= colChanged;

        for (int r = 0; r < m_n; ++r) m_board[r][c] = col[r];
    }
    if (changed) spawnRandomTile();
    return changed;
}

bool Game2048::moveDown() {
    bool changed = false;
    for (int c = 0; c < m_n; ++c) {
        std::vector<int> col(m_n);
        for (int r = 0; r < m_n; ++r) col[r] = m_board[r][c];
        std::reverse(col.begin(), col.end());

        bool colChanged = slideAndMergeRowLeft(col);
        changed |= colChanged;

        std::reverse(col.begin(), col.end());
        for (int r = 0; r < m_n; ++r) m_board[r][c] = col[r];
    }
    if (changed) spawnRandomTile();
    return changed;
}

void Game2048::spawnRandomTile() {
    std::vector<std::pair<int,int>> empties;
    empties.reserve(m_n*m_n);
    for (int r=0;r<m_n;++r)
        for (int c=0;c<m_n;++c)
            if (m_board[r][c]==0) empties.push_back({r,c});

    if (empties.empty()) return;

    std::uniform_int_distribution<int> posDist(0, (int)empties.size()-1);
    auto [r,c] = empties[posDist(m_rng)];

    std::uniform_int_distribution<int> valDist(1, 10);
    m_board[r][c] = (valDist(m_rng)==10) ? 4 : 2;
}

bool Game2048::isWin() const {
    for (auto& row : m_board)
        for (int v : row)
            if (v == 2048) return true;
    return false;
}

bool Game2048::canMergeOrMove() const {
    for (int r=0;r<m_n;++r){
        for (int c=0;c<m_n;++c){
            if (m_board[r][c]==0) return true;
            if (c+1<m_n && m_board[r][c]==m_board[r][c+1]) return true;
            if (r+1<m_n && m_board[r][c]==m_board[r+1][c]) return true;
        }
    }
    return false;
}

bool Game2048::isGameOver() const {
    return !canMergeOrMove();
}
