#include "ai2048.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <future>
#include <mutex>
#include <fstream>
#include <sstream>
#include <thread>

static int countEmpty(const Game2048& g) {
    int n = g.size();
    int cnt = 0;
    for (int r = 0; r < n; ++r)
        for (int c = 0; c < n; ++c)
            if (g.at(r, c) == 0) ++cnt;
    return cnt;
}

static bool maxInCorner(const Game2048& g) {
    int n = g.size();
    int maxVal = 0;
    for (int r = 0; r < n; ++r)
        for (int c = 0; c < n; ++c)
            maxVal = std::max(maxVal, g.at(r, c));

    return
        g.at(0, 0)       == maxVal ||
        g.at(0, n-1)     == maxVal ||
        g.at(n-1, 0)     == maxVal ||
        g.at(n-1, n-1)   == maxVal;
}

static double monotonicityScore(const Game2048& g) {
    int n = g.size();
    double score = 0.0;

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c + 1 < n; ++c) {
            int a = g.at(r, c);
            int b = g.at(r, c + 1);
            if (a != 0 && b != 0) {
                if (a >= b) score += 1.0;
                else        score -= 1.0;
            }
        }
    }

    for (int c = 0; c < n; ++c) {
        for (int r = 0; r + 1 < n; ++r) {
            int a = g.at(r, c);
            int b = g.at(r + 1, c);
            if (a != 0 && b != 0) {
                if (a >= b) score += 1.0;
                else        score -= 1.0;
            }
        }
    }

    return score;
}

static double smoothnessScore(const Game2048& g) {
    int n = g.size();
    double penalty = 0.0;

    auto logVal = [](int v) {
        return (v > 0) ? std::log2((double)v) : 0.0;
    };

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            int v = g.at(r, c);
            if (v == 0) continue;

            if (c + 1 < n) {
                int v2 = g.at(r, c + 1);
                if (v2 != 0) penalty += std::abs(logVal(v) - logVal(v2));
            }
            if (r + 1 < n) {
                int v2 = g.at(r + 1, c);
                if (v2 != 0) penalty += std::abs(logVal(v) - logVal(v2));
            }
        }
    }

    return -penalty;
}

static int mergePotential(const Game2048& g) {
    int n = g.size();
    int cnt = 0;

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c + 1 < n; ++c) {
            if (g.at(r,c) != 0 && g.at(r,c) == g.at(r,c+1)) ++cnt;
        }
    }
    for (int c = 0; c < n; ++c) {
        for (int r = 0; r + 1 < n; ++r) {
            if (g.at(r,c) != 0 && g.at(r,c) == g.at(r+1,c)) ++cnt;
        }
    }
    return cnt;
}

double evaluateBoard(const Game2048& game, const Weights& w)
{
    int empty = countEmpty(game);
    bool corner = maxInCorner(game);
    double mono = monotonicityScore(game);
    double smooth = smoothnessScore(game);
    int merges = mergePotential(game);

    double score = 0.0;
    score += w.wEmpty     * empty;
    score += w.wMonotonic * mono;
    score += w.wSmooth    * smooth;
    score += w.wMerge     * merges;
    score += w.wCornerMax * (corner ? 1.0 : -1.0);

    return score;
}

static bool tryMove(Game2048& g, Direction dir) {
    switch (dir) {
    case Direction::Left:  return g.moveLeft();
    case Direction::Right: return g.moveRight();
    case Direction::Up:    return g.moveUp();
    case Direction::Down:  return g.moveDown();
    }
    return false;
}

Direction chooseMove(const Game2048& game, const Weights& w)
{
    Direction dirs[] = {
        Direction::Left,
        Direction::Right,
        Direction::Up,
        Direction::Down
    };

    double bestScore = -1e100;
    Direction bestDir = Direction::Left;

    for (Direction d : dirs) {
        Game2048 tmp = game;
        if (!tryMove(tmp, d)) {
            continue;
        }

        double s = evaluateBoard(tmp, w);
        if (s > bestScore) {
            bestScore = s;
            bestDir = d;
        }
    }

    return bestDir;
}

double playOneGame(const Weights& w, int maxMoves, int* outMoves)
{
    Game2048 g;
    int moves = 0;

    while (!g.isGameOver() && moves < maxMoves) {
        Direction d = chooseMove(g, w);

        bool moved = false;
        switch (d) {
        case Direction::Left:  moved = g.moveLeft();  break;
        case Direction::Right: moved = g.moveRight(); break;
        case Direction::Up:    moved = g.moveUp();    break;
        case Direction::Down:  moved = g.moveDown();  break;
        }

        if (!moved) {
            break;
        }

        ++moves;
    }

    if (outMoves) {
        *outMoves = moves;
    }

    return static_cast<double>(g.score());
}

double evaluateFitness(const Weights& w, int games, int maxMoves,
                       double& outBestScore, int& outBestMoves,
                       int threadCount)
{
    double total = 0.0;
    outBestScore = 0.0;
    outBestMoves = 0;

    const int workers = (threadCount > 0)
        ? threadCount
        : std::max(1u, std::thread::hardware_concurrency());

    const int tasks = std::max(1, std::min(games, workers));
    const int baseCount = games / tasks;
    const int remainder = games % tasks;

    std::mutex aggMutex;
    std::vector<std::future<void>> futures;
    futures.reserve(tasks);

    int start = 0;
    for (int i = 0; i < tasks; ++i) {
        int count = baseCount + (i < remainder ? 1 : 0);
        int taskStart = start;
        int taskEnd   = start + count;
        start = taskEnd;

        futures.emplace_back(std::async(std::launch::async,
            [&, taskStart, taskEnd]() {
                double localTotal = 0.0;
                double localBestScore = 0.0;
                int    localBestMoves = 0;

                for (int idx = taskStart; idx < taskEnd; ++idx) {
                    (void)idx; // suppress unused warning
                    int moves = 0;
                    double score = playOneGame(w, maxMoves, &moves);

                    localTotal += score;
                    if (score > localBestScore) {
                        localBestScore = score;
                        localBestMoves = moves;
                    }
                }

                std::scoped_lock lock(aggMutex);
                total += localTotal;
                if (localBestScore > outBestScore) {
                    outBestScore = localBestScore;
                    outBestMoves = localBestMoves;
                }
            }
        ));
    }

    for (auto& f : futures) {
        f.get();
    }

    return (games > 0) ? (total / games) : 0.0;
}

static double rnd(double a, double b) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(a, b);
    return dist(rng);
}

Weights randomWeights() {
    Weights w;
    w.wEmpty     = rnd(50, 300);
    w.wMonotonic = rnd(10, 100);
    w.wSmooth    = rnd(-10, -1);
    w.wCornerMax = rnd(5000, 20000);
    w.wMerge     = rnd(10, 200);
    return w;
}

void mutateWeights(Weights& w, double rate)
{
    if (rnd(0,1) < rate) w.wEmpty     += rnd(-20, 20);
    if (rnd(0,1) < rate) w.wMonotonic += rnd(-10, 10);
    if (rnd(0,1) < rate) w.wSmooth    += rnd(-3, 3);
    if (rnd(0,1) < rate) w.wCornerMax += rnd(-2000, 2000);
    if (rnd(0,1) < rate) w.wMerge     += rnd(-20, 20);
}

Weights crossover(const Weights& a, const Weights& b)
{
    Weights c;
    c.wEmpty     = (rnd(0,1) < 0.5 ? a.wEmpty     : b.wEmpty);
    c.wMonotonic = (rnd(0,1) < 0.5 ? a.wMonotonic : b.wMonotonic);
    c.wSmooth    = (rnd(0,1) < 0.5 ? a.wSmooth    : b.wSmooth);
    c.wCornerMax = (rnd(0,1) < 0.5 ? a.wCornerMax : b.wCornerMax);
    c.wMerge     = (rnd(0,1) < 0.5 ? a.wMerge     : b.wMerge);
    return c;
}

Population createInitialPopulation(int size)
{
    Population pop(size);
    for (auto& ind : pop) {
        ind.w = randomWeights();
        ind.fitness = 0.0;
        ind.bestScore = 0.0;
        ind.bestMoves = 0;
    }
    return pop;
}

Population evolve(const Population& pop, double eliteRate, double mutationRate)
{
    int size = (int)pop.size();
    int eliteCount = std::max(1, int(size * eliteRate));

    Population newPop;
    newPop.reserve(size);

    Population sorted = pop;
    std::sort(sorted.begin(), sorted.end(),
              [](auto& a, auto& b){ return a.fitness > b.fitness; });

    for (int i = 0; i < eliteCount; ++i)
        newPop.push_back(sorted[i]);

    while ((int)newPop.size() < size)
    {
        const Individual& p1 = sorted[rnd(0,1) < 0.5 ? 0 : (int)rnd(1, sorted.size()-1)];
        const Individual& p2 = sorted[rnd(0,1) < 0.5 ? 0 : (int)rnd(1, sorted.size()-1)];

        Individual child;
        child.w = crossover(p1.w, p2.w);
        mutateWeights(child.w, mutationRate);
        child.fitness   = 0.0;
        child.bestScore = 0.0;
        child.bestMoves = 0;

        newPop.push_back(child);
    }

    return newPop;
}

void evaluatePopulation(Population& pop, int games, int maxMoves, int threadCount)
{
    for (auto& ind : pop) {
        double bestScore = 0.0;
        int    bestMoves = 0;

        ind.fitness = evaluateFitness(ind.w,
                                      games,
                                      maxMoves,
                                      bestScore,
                                      bestMoves,
                                      threadCount);

        ind.bestScore = bestScore;
        ind.bestMoves = bestMoves;
    }
}

static void writeIndividual(std::ostream& os, const Individual& ind)
{
    os << ind.w.wEmpty << ' '
       << ind.w.wMonotonic << ' '
       << ind.w.wSmooth << ' '
       << ind.w.wCornerMax << ' '
       << ind.w.wMerge << ' '
       << ind.fitness << ' '
       << ind.bestScore << ' '
       << ind.bestMoves << '\n';
}

bool savePopulation(const Population& pop, int generation, const std::string& filePath)
{
    std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) return false;

    ofs << generation << '\n';
    ofs << pop.size() << '\n';
    for (const auto& ind : pop) {
        writeIndividual(ofs, ind);
    }
    return true;
}

static bool readIndividual(std::istream& is, Individual& ind)
{
    return static_cast<bool>(is >> ind.w.wEmpty
                               >> ind.w.wMonotonic
                               >> ind.w.wSmooth
                               >> ind.w.wCornerMax
                               >> ind.w.wMerge
                               >> ind.fitness
                               >> ind.bestScore
                               >> ind.bestMoves);
}

Population loadPopulation(const std::string& filePath, int expectedSize, int& outGeneration)
{
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        outGeneration = 0;
        return createInitialPopulation(expectedSize);
    }

    int generation = 0;
    int size = 0;
    if (!(ifs >> generation)) {
        outGeneration = 0;
        return createInitialPopulation(expectedSize);
    }
    if (!(ifs >> size) || size <= 0) {
        outGeneration = generation;
        return createInitialPopulation(expectedSize);
    }

    Population pop;
    pop.reserve(size);
    for (int i = 0; i < size; ++i) {
        Individual ind;
        if (!readIndividual(ifs, ind)) break;
        pop.push_back(ind);
    }

    if ((int)pop.size() != size) {
        outGeneration = generation;
        return createInitialPopulation(expectedSize);
    }

    outGeneration = generation;
    return pop;
}
