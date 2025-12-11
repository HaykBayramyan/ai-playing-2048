#include "boardwidget.h"
#include "game2048.h"
#include <QPainter>
#include <algorithm>

BoardWidget::BoardWidget(Game2048* game, QWidget* parent)
    : QWidget(parent), m_game(game) {
    setFocusPolicy(Qt::StrongFocus);
}

QColor BoardWidget::tileColor(int v) const {
    switch (v) {
    case 0:    return QColor("#cdc1b4");
    case 2:    return QColor("#eee4da");
    case 4:    return QColor("#ede0c8");
    case 8:    return QColor("#f2b179");
    case 16:   return QColor("#f59563");
    case 32:   return QColor("#f67c5f");
    case 64:   return QColor("#f65e3b");
    case 128:  return QColor("#edcf72");
    case 256:  return QColor("#edcc61");
    case 512:  return QColor("#edc850");
    case 1024: return QColor("#edc53f");
    case 2048: return QColor("#edc22e");
    default:   return QColor("#3c3a32");
    }
}
QColor BoardWidget::textColor(int v) const {
    return (v <= 4) ? QColor("#776e65") : QColor("#f9f6f2");
}

void BoardWidget::paintEvent(QPaintEvent*) {
    if (!m_game) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), QColor("#bbada0"));

    const int N = m_game->size();
    const int margin = 14;
    int cell = (std::min(width(), height()) - margin*(N+1)) / N;

    QFont base = font();
    base.setBold(true);

    for (int r=0; r<N; ++r) {
        for (int c=0; c<N; ++c) {
            int x = margin + c*(cell+margin);
            int y = margin + r*(cell+margin);
            QRect tileRect(x, y, cell, cell);

            int val = m_game->at(r,c);

            p.setPen(Qt::NoPen);
            p.setBrush(tileColor(val));
            p.drawRoundedRect(tileRect, 10, 10);

            if (val != 0) {
                p.setPen(textColor(val));

                int fs = (val < 100) ? cell/3 : (val < 1000 ? cell/4 : cell/5);
                QFont f = base;
                f.setPointSize(fs);
                p.setFont(f);

                p.drawText(tileRect, Qt::AlignCenter, QString::number(val));
            }
        }
    }
}
