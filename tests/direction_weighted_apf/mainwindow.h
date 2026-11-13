#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "vec.h"
#include <QWidget>
#include <QTimer>

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow();

private:
    const qreal _scale = 420;
    const qreal _bounding_radius = 0.15;
    const qreal _k_inflation = 5.0;
    // const qreal _k_inscribed = 0.5257;
    const qreal _k_inscribed = 1.1;
    const qreal _r_inflation = _bounding_radius * _k_inflation;
    const qreal _r_inscribed = _bounding_radius * _k_inscribed;
    const qreal _cost_scaling_factor = 8.0;

    std::vector<VecF> m_obstacles;
    VecF m_pos;
    VecF m_goal;
    QImage m_apf;
    QTimer *m_timer;

    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // MAINWINDOW_H
