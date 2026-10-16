#ifndef TRAJECTORYENGINE_H
#define TRAJECTORYENGINE_H

#include <QVector3D>
#include <QQuaternion>

class TrajectoryEngine
{
public:
    TrajectoryEngine();

    // Метод для получения данных от BNO085
    void updateData(const QQuaternion &orientation, const QVector3D &linearAccel);

    // Геттеры для вывода
    QVector3D position() const { return m_position; }
    QVector3D velocity() const { return m_velocity; }
    bool isStationary() const { return m_isStationary; }

private:
    // Состояние
    QQuaternion m_orientation;
    QVector3D m_linearAccelLocal;
    QVector3D m_linearAccelGlobal;
    QVector3D m_velocity;
    QVector3D m_position;
    
    // Для интегрирования
    QVector3D m_prevAccel;
    
    // Параметры фильтрации и ZUPT
    struct Butterworth {
        float a0, a1, a2, b1, b2;
        float x1, x2, y1, y2;
        void init(float fc, float fs);
        float update(float x);
    };
    Butterworth m_filtX, m_filtY;

    // ZUPT
    bool m_isStationary = false;
    QVector<float> m_accelBufferX;
    QVector<float> m_accelBufferY;
    QVector<float> m_accelBufferZ;
    static const int ZUPT_WINDOW = 30;
    static const float ZUPT_THRESHOLD;
    
    // Частота
    static const int SAMPLE_RATE_HZ = 100;
    const float DT = 1.0f / SAMPLE_RATE_HZ;
    
    void initButterworth();
    void checkZUPT(const QVector3D &accel);
};

#endif // TRAJECTORYENGINE_H
