#pragma once
#include <QMainWindow>

class QLabel;
class QPushButton;
class BoardWidget;
class Game2048;
class PopulationWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    void refreshUI();
    void showWinDialogIfNeeded();

    Game2048* m_game = nullptr;
    BoardWidget* m_board = nullptr;
    QLabel* m_scoreLabel = nullptr;
    QPushButton* m_resetBtn = nullptr;
    bool m_winShown = false;
    PopulationWindow* m_populationWindow = nullptr;
};
