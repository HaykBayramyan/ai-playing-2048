#include "populationwindow.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QLabel>
#include <algorithm>
#include <iostream>

PopulationWindow::PopulationWindow(QWidget* parent)
    : QWidget(parent)
    , m_agents(Count)
    , m_stepTimer(new QTimer(this))
    , m_gaTimer(new QTimer(this))
    , m_population(createInitialPopulation(Count))
    , m_generation(0)
{
    int loadedGeneration = 0;
    m_population = loadPopulation(SaveFileName, Count, loadedGeneration);
    m_generation = loadedGeneration;

    setWindowTitle("GA Population Visualization");
    m_generationLabel = new QLabel(this);
    m_generationLabel->setAlignment(Qt::AlignCenter);
    m_generationLabel->setText(QString("Generation: %1").arg(m_generation));

    // Сетка агентов
    auto* grid = new QGridLayout();
    grid->setSpacing(4);
    grid->setContentsMargins(0, 0, 0, 0);

    int index = 0;
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            Agent& a = m_agents[index];

            a.game    = std::make_unique<Game2048>();
            a.weights = m_population[index].w;
            a.steps   = 0;
            a.bestScore = 0;
            a.bestMoves = 0;
            a.finished  = false;

            a.boardWidget = new BoardWidget(a.game.get(), this);
            a.boardWidget->setMinimumSize(80, 80);
            a.boardWidget->setSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding);

            a.scoreLabel = new QLabel(this);
            a.scoreLabel->setAlignment(Qt::AlignCenter);
            a.scoreLabel->setText("Score: 0 | Best: 0 (0 steps)");

            auto* cellLayout = new QVBoxLayout();
            cellLayout->addWidget(a.scoreLabel);
            cellLayout->addWidget(a.boardWidget);

            grid->addLayout(cellLayout, r, c);

            ++index;
        }
    }

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_generationLabel);
    mainLayout->addLayout(grid);
    setLayout(mainLayout);

    connect(m_stepTimer, &QTimer::timeout,
            this, &PopulationWindow::stepAll);
    m_stepTimer->start(50);   // каждые 50 мс — один шаг для всех агентов

    m_gaTimer->setSingleShot(true);
    connect(m_gaTimer, &QTimer::timeout,
            this, &PopulationWindow::nextGeneration);
    setPopulation(m_population, m_generation);
}

void PopulationWindow::setPopulation(const Population& pop, int generation)
{
    m_generation = generation;

    if (m_generationLabel) {
        m_generationLabel->setText(
            QString("Generation: %1").arg(m_generation));
    }

    int n = std::min<int>(Count, pop.size());

    for (int i = 0; i < n; ++i) {
        Agent& a = m_agents[i];
        const Individual& ind = pop[i];

        a.weights   = ind.w;
        a.steps     = 0;
        a.bestScore = 0;
        a.bestMoves = 0;
        a.finished  = false;

        if (a.game) {
            a.game->reset();
        }

        if (a.scoreLabel) {
            a.scoreLabel->setText(
                QString("Score: 0 | Best: %1 (%2 steps)")
                    .arg(a.bestScore)
                    .arg(a.bestMoves));
        }

        if (a.boardWidget) {
            a.boardWidget->update();
        }
    }
}
void PopulationWindow::stepAll()
{
    bool allFinished = true;

    for (auto& a : m_agents) {
        if (!a.game) continue;

        if (a.finished) {
            continue;
        }
        allFinished = false;

        if (a.game->isGameOver()) {
            a.finished = true;

            int finalScore = a.game->score();
            if (finalScore > a.bestScore) {
                a.bestScore = finalScore;
                a.bestMoves = a.steps;
            }

            if (a.scoreLabel) {
                a.scoreLabel->setText(
                    QString("Score: %1 | Best: %2 (%3 steps)")
                        .arg(finalScore)
                        .arg(a.bestScore)
                        .arg(a.bestMoves));
            }

            continue;
        }

        Direction d = chooseMove(*a.game, a.weights);

        bool moved = false;
        switch (d) {
        case Direction::Left:  moved = a.game->moveLeft();  break;
        case Direction::Right: moved = a.game->moveRight(); break;
        case Direction::Up:    moved = a.game->moveUp();    break;
        case Direction::Down:  moved = a.game->moveDown();  break;
        }

        if (moved) {
            ++a.steps;

            int curScore = a.game->score();
            if (curScore > a.bestScore) {
                a.bestScore = curScore;
                a.bestMoves = a.steps;
            }

            if (a.boardWidget) {
                a.boardWidget->update();
            }
            if (a.scoreLabel) {
                a.scoreLabel->setText(
                    QString("Score: %1 | Best: %2 (%3 steps)")
                        .arg(curScore)
                        .arg(a.bestScore)
                        .arg(a.bestMoves));
            }
        }
    }

    if (allFinished && !m_waitingNextGen) {
        m_waitingNextGen = true;
        if (m_gaTimer) {
            m_gaTimer->start(5000);
        }
    }
}

void PopulationWindow::nextGeneration()
{
    m_waitingNextGen = false;

    int n = std::min<int>(Count, m_population.size());
    for (int i = 0; i < n; ++i) {
        Individual& ind = m_population[i];
        const Agent& a  = m_agents[i];

        ind.fitness   = a.bestScore;
        ind.bestScore = a.bestScore;
        ind.bestMoves = a.bestMoves;
    }

    auto bestIt = std::max_element(
        m_population.begin(), m_population.end(),
        [](const Individual& x, const Individual& y){
            return x.fitness < y.fitness;
        });

    if (bestIt != m_population.end()) {
        std::cout << "Generation " << m_generation
                  << " best fitness = " << bestIt->fitness
                  << " (score=" << bestIt->bestScore
                  << ", steps=" << bestIt->bestMoves << ")"
                  << std::endl;
    }

    m_population = evolve(m_population, 0.1, 0.1);
    ++m_generation;
    setPopulation(m_population, m_generation);

    savePopulation(m_population, m_generation, SaveFileName);
}
