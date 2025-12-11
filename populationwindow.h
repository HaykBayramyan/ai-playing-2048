#pragma once

#include <QWidget>
#include <vector>
#include <memory>

#include "game2048.h"
#include "boardwidget.h"
#include "ai2048.h"

class QTimer;
class QLabel;

class PopulationWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PopulationWindow(QWidget* parent = nullptr);
    ~PopulationWindow() override = default;

private slots:
    void stepAll();
    void nextGeneration();

private:
    struct Agent {
        std::unique_ptr<Game2048> game;
        Weights      weights;
        BoardWidget* boardWidget = nullptr;
        QLabel*      scoreLabel  = nullptr;

        int          steps       = 0;
        int          bestScore   = 0;
        int          bestMoves   = 0;
        bool         finished    = false;
    };

    void setPopulation(const Population& pop, int generation);

    static constexpr int Count = 40;
    static constexpr int Rows  = 5;
    static constexpr int Cols  = 8;

    static constexpr const char* SaveFileName = "population_state.txt";

    std::vector<Agent> m_agents;

    QTimer*            m_stepTimer      = nullptr;
    QTimer*            m_gaTimer        = nullptr;
    Population         m_population;
    int                m_generation     = 0;
    QLabel*            m_generationLabel = nullptr;

    bool               m_waitingNextGen = false;
};
