#include "lidar.h"
#include "qdebug.h"
#include <QTimer>

Lidar::Lidar(QObject *parent) : QObject(parent), m_scan(nullptr)
{
    return;
    //ydlidar::os_init();

    // инициализация SDK и LiDAR

    // Цикл получения данных
    m_thread = std::thread([this]() {
        CYdLidar lidar;
        ////////////////////// string property ///////////////////
        // порт лидара (укажите ваш COM-порт)
#if defined(__arm__) || defined(__aarch64__)
        std::string port = "/dev/ttyAMA2";
#else
        std::string port = "COM3";
#endif
        lidar.setlidaropt(LidarPropSerialPort, port.c_str(), port.size());

        // массив игнорирования
        std::string ignore_array;
        ignore_array.clear();
        lidar.setlidaropt(LidarPropIgnoreArray, ignore_array.c_str(),
                          ignore_array.size());

        ////////////////////// int property ///////////////////
        // скорость порта - для X3 Pro ОБЯЗАТЕЛЬНО 115200
        int optval = 115200;
        lidar.setlidaropt(LidarPropSerialBaudrate, &optval, sizeof(int));

        // тип лидара - триангуляционный
        optval = TYPE_TRIANGLE;  // <-- ПРАВИЛЬНО
        lidar.setlidaropt(LidarPropLidarType, &optval, sizeof(int));

        // тип устройства - последовательный порт
        optval = YDLIDAR_TYPE_SERIAL;
        lidar.setlidaropt(LidarPropDeviceType, &optval, sizeof(int));

        // частота дискретизации - для X3 Pro стандартная
        optval = 5;  // можно оставить 9 или 5
        lidar.setlidaropt(LidarPropSampleRate, &optval, sizeof(int));

        // количество проверок аномалий
        optval = 4;
        lidar.setlidaropt(LidarPropAbnormalCheckCount, &optval, sizeof(int));

        ////////////////////// bool property ///////////////////
        // фиксированное разрешение по углу
        bool b_optvalue = false;
        lidar.setlidaropt(LidarPropFixedResolution, &b_optvalue, sizeof(bool));

        // поворот на 180 градусов
        lidar.setlidaropt(LidarPropReversion, &b_optvalue, sizeof(bool));

        // направление вращения (против часовой)
        lidar.setlidaropt(LidarPropInverted, &b_optvalue, sizeof(bool));

        // автоматическое переподключение
        b_optvalue = true;
        lidar.setlidaropt(LidarPropAutoReconnect, &b_optvalue, sizeof(bool));

        // однонаправленная связь
        b_optvalue = true;
        lidar.setlidaropt(LidarPropSingleChannel, &b_optvalue, sizeof(bool));

        // интенсивность (для X3 Pro false)
        b_optvalue = false;
        lidar.setlidaropt(LidarPropIntenstiy, &b_optvalue, sizeof(bool));

        // управление мотором через DTR
        b_optvalue = false;
        lidar.setlidaropt(LidarPropSupportMotorDtrCtrl, &b_optvalue, sizeof(bool));

        ////////////////////// float property ///////////////////
        // минимальный угол (градусы)
        float f_optvalue = -180.f;
        lidar.setlidaropt(LidarPropMinAngle, &f_optvalue, sizeof(float));

        // максимальный угол (градусы)
        f_optvalue = 180.f;
        lidar.setlidaropt(LidarPropMaxAngle, &f_optvalue, sizeof(float));

        // минимальная дальность (метры)
        lidar.setlidaropt(LidarPropMinRange, &m_minRange, sizeof(float));

        // максимальная дальность (метры) - для X3 Pro до 8м
        lidar.setlidaropt(LidarPropMaxRange, &m_maxRange, sizeof(float));

        // частота сканирования (Гц) - для X3 Pro 5-10 Гц
        f_optvalue = 10.0f;  // можно 5.0f
        lidar.setlidaropt(LidarPropScanFrequency, &f_optvalue, sizeof(float));
        bool ret = lidar.initialize();
        if (ret) {
            // Запустить процедуру сканирования
            ret = lidar.turnOn();
        } else {
            qDebug() << "Ошибка инициализации:" << lidar.DescribeError();
            return;
        }
        while (m_running.load() && ydlidar::os_isOk()) {
            // QElapsedTimer tm;
            // tm.start();
            auto scan = std::make_shared<LaserScan>();
            if (lidar.doProcessSimple(*scan)) {
                m_scan.store(scan);
                m_updated.store(true);
            } else {
                qDebug() << "Не удалось получить данные лидара";
            }
        }
        lidar.turnOff();
        lidar.disconnecting();
    });

    QTimer *timer = new QTimer(this);
    timer->setInterval(1);
    timer->callOnTimeout(this, &Lidar::loadScan);
    timer->start();
}

Lidar::~Lidar() {
    m_running.store(false);
    m_thread.join();
}

void Lidar::loadScan() {
    if (!m_updated.load()) return;
    m_updated.store(false);
    auto scan = m_scan.load();
    if (!scan) return;

    //======================================================================

    // qDebug() << "======================================";
    // qDebug() << QString("Получено сканирование[%1]: %2 точек, частота [%3] Гц")
    //             .arg(scan->stamp / 1e6).arg((unsigned int)scan->size).arg(1.0 / scan->config.scan_time);
    // qDebug() << "min angle:" << scan->config.min_angle
    //          << "\nmax angle:" << scan->config.max_angle
    //          << "\nmin range:" << scan->config.min_range
    //          << "\nmax range:" << scan->config.max_range
    //          << "\nangle increment:" << scan->config.angle_increment
    //          << "\ntime increment:" << scan->config.time_increment;

    //======================================================================

    // mrpt::poses::CPose2D sensorPose;    // Поза сенсора относительно робота
    // std::vector<float> scan;            // Дальности сканирования
    // std::vector<float> validRange;      // Валидность измерений
    // double aperture;                    // Угол обзора (радианы)
    // double maxRange;                    // Максимальная дальность
    // double timestamp;                   // Временная метка
    // int32_t scans_count;                // Количество лучей

    auto obs = mrpt::obs::CObservation2DRangeScan::Create();
    //auto obs = std::make_shared<mrpt::obs::CObservation2DRangeScan>();

    obs->resizeScan(scan->size);
    obs->setScanHasIntensity(false);
    obs->rightToLeft = false;
    obs->maxRange = m_maxRange;
    obs->aperture = 2.0 * M_PI;
    // Положение сенсора на роботе
    obs->setSensorPose(mrpt::poses::CPose3D(0, 0, 0, 0, 0, 0));
    // obs->setSensorPose(mrpt::poses::CPose3D::Identity());

    // Временная метка
    // obs->timestamp = mrpt::Clock::fromDouble(static_cast<double>(scan->stamp) / 1e9);
    obs->timestamp = mrpt::Clock::now();
    // static double start_time = static_cast<double>(scan->stamp) / 1e9;
    // qDebug() << static_cast<double>(scan->stamp) / 1e9 - start_time;
    // static long long start_time = mrpt::Clock::now().time_since_epoch().count();
    // qDebug() << mrpt::Clock::now().time_since_epoch().count() - start_time;

    obs->sensorLabel = "lidar";

    // сортировка
    std::sort(scan->points.begin(), scan->points.end(), [](const LaserPoint &a, const LaserPoint &b) {
        return a.angle < b.angle;
    });

    // интерполяция
    for (int i = 0; i < scan->size; ++i) {
        // qDebug() << obs->getScanAngle(i) << scan->points[i].angle;
        auto [range, valid] = interpolateValue(scan->points, -obs->getScanAngle(i));
        obs->setScanRange(i, range);
        obs->setScanRangeValidity(i, valid);
    }

    // for (int i = 0; i < scan->size; ++i) {
    //     float &range = scan->points[i].range;
    //     obs->setScanRange(i, range);
    //     obs->setScanRangeValidity(i, rangeIsValid(range));
    // }

    emit scanChanged(obs);

    //=========================================================

    auto points = mrpt::maps::CSimplePointsMap::Create();
    for (const auto &point : scan->points) {
        if (!rangeIsValid(point.range)) continue;
        float x = point.range * cos(point.angle);
        float y = point.range * sin(point.angle);
        float z = 0.0f;
        points->insertPoint(x, y, z);
    }

    auto pointCloud = mrpt::obs::CObservationPointCloud::Create();
    pointCloud->pointcloud = points;
    pointCloud->timestamp = mrpt::Clock::now();
    pointCloud->sensorLabel = "lidar";
    pointCloud->setSensorPose(mrpt::poses::CPose3D(0, 0, 0, 0, 0, 0));

    emit pointCloudChanged(pointCloud);
}

std::pair<float, bool> Lidar::interpolateValue(const std::vector<LaserPoint> &data, float targetAngle) {
    if (data.empty()) return {0.0f, false};

    if (data.size() == 1) return {data[0].range, rangeIsValid(data[0].range)};

    // Проверяем, не нужно ли интерполировать через границу
    float angleSpan = data.back().angle - data.front().angle;
    bool crossBoundary = (angleSpan > M_PI); // Скан покрывает > 180°

    // Если угол вне диапазона и скан не покрывает полный круг
    if (targetAngle < data.front().angle || targetAngle > data.back().angle) {
        if (!crossBoundary)
        {
            // Экстраполяция: берем ближайшую точку
            if (targetAngle < data.front().angle)
                return {data.front().range, rangeIsValid(data.front().range)};
            else
                return {data.back().range, rangeIsValid(data.back().range)};
        }
    }

    // Находим две соседние точки
    auto it = std::lower_bound(data.begin(), data.end(), targetAngle, [](const LaserPoint& p, float angle) {
        return p.angle < angle;
    });

    // Обработка граничных случаев для полного круга
    if (it == data.begin() || it == data.end()) {
        if (crossBoundary) {
            // Интерполяция через границу
            const auto& first = data.front();
            const auto& last = data.back();

            float t;
            if (targetAngle < 0)
            {
                // Угол около -π, интерполируем между last (-π+ε) и first (π-ε)
                t = (targetAngle + M_PI) / (first.angle + M_PI);
            }
            else
            {
                // Угол около +π
                t = (targetAngle - last.angle) / (M_PI - last.angle);
            }

            t = std::max(0.0f, std::min(1.0f, t));
            float range = last.range + t * (first.range - last.range);
            bool valid = rangeIsValid(last.range) && rangeIsValid(first.range);

            return {range, valid};
        } else {
            // Берем ближайшую точку
            if (it == data.begin())
                return {data.front().range, rangeIsValid(data.front().range)};
            else
                return {data.back().range, rangeIsValid(data.back().range)};
        }
    }

    // Нормальная интерполяция
    auto prev = it - 1;
    auto next = it;

    float t = (targetAngle - prev->angle) / (next->angle - prev->angle);
    t = std::max(0.0f, std::min(1.0f, t));

    float range = prev->range + t * (next->range - prev->range);
    bool valid = rangeIsValid(prev->range) && rangeIsValid(next->range);

    return {range, valid};
}
