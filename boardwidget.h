#pragma once
#include <QWidget>

class Game2048;

class BoardWidget : public QWidget {
    Q_OBJECT
public:
    explicit BoardWidget(Game2048* game, QWidget* parent=nullptr);

    QSize minimumSizeHint() const override { return {420, 420}; }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    Game2048* m_game;

    QColor tileColor(int value) const;
    QColor textColor(int value) const;
};
