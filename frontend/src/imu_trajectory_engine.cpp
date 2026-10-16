#include "imu_trajectory_engine.h"
#include <QtMath>
#include <QDebug>

const float TrajectoryEngine::ZUPT_THRESHOLD = 0.15f;

TrajectoryEngine::TrajectoryEngine()
{
    initButterworth();
    
    // Инициализация буферов ZUPT
    m_accelBufferX.resize(ZUPT_WINDOW);
    m_accelBufferY.resize(ZUPT_WINDOW);
    m_accelBufferZ.resize(ZUPT_WINDOW);
    m_accelBufferX.fill(0);
    m_accelBufferY.fill(0);
    m_accelBufferZ.fill(0);
}

void TrajectoryEngine::initButterworth()
{
    // Коэффициенты для fc = 5 Гц, fs = 100 Гц
    // Вычисляются один раз при старте
    float fc = 5.0f;
    float fs = SAMPLE_RATE_HZ;
    float w = tan(M_PI * fc / fs);
    float w2 = w * w;
    float denom = 1.0f + w * 1.41421356f + w2;
    
    // Инициализация для X
    m_filtX.a0 = w2 / denom;
    m_filtX.a1 = 2.0f * m_filtX.a0;
    m_filtX.a2 = m_filtX.a0;
    m_filtX.b1 = 2.0f * (w2 - 1.0f) / denom;
    m_filtX.b2 = (1.0f - w * 1.41421356f + w2) / denom;
    m_filtX.x1 = m_filtX.x2 = m_filtX.y1 = m_filtX.y2 = 0;
    
    // Инициализация для Y (аналогично)
    m_filtY = m_filtX;
}

float TrajectoryEngine::Butterworth::update(float x)
{
    float y = a0 * x + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2;
    x2 = x1; x1 = x;
    y2 = y1; y1 = y;
    return y;
}

void TrajectoryEngine::updateData(const QQuaternion &orientation, const QVector3D &linearAccel)
{
    m_orientation = orientation;
    m_linearAccelLocal = linearAccel;

    // 1. Поворот вектора ускорения в глобальную систему
    // QQuaternion::rotatedVector делает всю математику за нас
    m_linearAccelGlobal = m_orientation.rotatedVector(m_linearAccelLocal);

    // 2. Фильтрация (применяем к X и Y, Z пока игнорируем для плоскости)
    float axf = m_filtX.update(m_linearAccelGlobal.x());
    float ayf = m_filtY.update(m_linearAccelGlobal.y());

    // 3. ZUPT
    checkZUPT(QVector3D(axf, ayf, m_linearAccelGlobal.z()));
    if (m_isStationary) {
        m_velocity *= 0.95f; // Плавное гашение скорости
    }

    // 4. Интегрирование (Метод трапеций)
    QVector3D accel(axf, ayf, 0.0f); // Ограничиваем движение плоскостью XY

    QVector3D v_new = m_velocity + (m_prevAccel + accel) * 0.5f * DT;

    // Обновляем позицию (используя старую скорость для точности)
    m_position += m_velocity * DT + 0.5f * accel * DT * DT;

    m_velocity = v_new;
    m_prevAccel = accel;
}

void TrajectoryEngine::checkZUPT(const QVector3D &accel)
{
    // Сдвигаем буфер (простой FIFO)
    m_accelBufferX.removeFirst(); m_accelBufferX.append(accel.x());
    m_accelBufferY.removeFirst(); m_accelBufferY.append(accel.y());
    m_accelBufferZ.removeFirst(); m_accelBufferZ.append(accel.z());

    // Считаем дисперсию
    auto calcVariance = [](const QVector<float> &buf) {
        float mean = 0;
        for (float v : buf) mean += v;
        mean /= buf.size();
        
        float var = 0;
        for (float v : buf) var += (v - mean) * (v - mean);
        return var / buf.size();
    };

    float varX = calcVariance(m_accelBufferX);
    float varY = calcVariance(m_accelBufferY);
    float varZ = calcVariance(m_accelBufferZ);

    m_isStationary = (varX < ZUPT_THRESHOLD && varY < ZUPT_THRESHOLD && varZ < ZUPT_THRESHOLD);
}
