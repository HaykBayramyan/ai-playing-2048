#ifndef AI2048_H
#define AI2048_H

#include <vector>
#include <string>
#include "game2048.h"

enum class Direction {
    Left,
    Right,
    Up,
    Down
};

struct Weights {
    double wEmpty     = 200.0;
    double wMonotonic = 50.0;
    double wSmooth    = -3.0;
    double wCornerMax = 10000.0;
    double wMerge     = 100.0;
};

double evaluateBoard(const Game2048& game, const Weights& w);

Direction chooseMove(const Game2048& game, const Weights& w);

double playOneGame(const Weights& w,
                   int maxMoves = 1000,
                   int* outMoves = nullptr);

double evaluateFitness(const Weights& w,
                       int games,
                       int maxMoves,
                       double& outBestScore,
                       int& outBestMoves,
                       int threadCount = 0);

struct Individual {
    Weights w;
    double fitness   = 0.0;
    double bestScore = 0.0;
    int    bestMoves = 0;
};

using Population = std::vector<Individual>;

Weights randomWeights();
void mutateWeights(Weights& w, double rate = 0.1);
Weights crossover(const Weights& a, const Weights& b);

Population createInitialPopulation(int size);
void evaluatePopulation(Population& pop,
                        int games = 10,
                        int maxMoves = 1000,
                        int threadCount = 0);

bool savePopulation(const Population& pop,
                    int generation,
                    const std::string& filePath);

Population loadPopulation(const std::string& filePath,
                          int expectedSize,
                          int& outGeneration);

Population evolve(const Population& pop,
                  double eliteRate = 0.1,
                  double mutationRate = 0.1);

#endif // AI2048_H
