#include "mapper.h"
#include "mrpt/io/CFileInputStream.h"
#include "mrpt/io/CFileOutputStream.h"
#include "qdebug.h"
#include "qtimer.h"
#include <mrpt/serialization/CArchive.h>
#include <QIODevice>
// #include <mrpt/io/CMemoryStream.h>
// #include <mrpt/graphslam/NRD/CICPCriteriaNRD.h>
// #include <mrpt/graphslam/ERD/CICPCriteriaERD.h>
// #include <mrpt/graphslam/GSO/CLevMarqGSO.h>

Mapper::Mapper(QObject *parent)
    : QObject(parent)
    , m_mclInit(false)
    , m_buildingMap(false)
    , m_currentObs(nullptr)
    , m_gridMap(nullptr)
{
    // mrpt::containers::yaml cfg;
    // cfg.loadFromFile("config.yaml");
    // m_mapper.initialize(cfg);

    // Загрузка карты
    try {
        mrpt::io::CFileInputStream ifs("map.gridmap");
        auto arch_in = mrpt::serialization::archiveFrom(ifs);
        arch_in >> m_gridMap;
        if (m_gridMap) {
            QMetaObject::invokeMethod(this, [this] { emit mapChanged(m_gridMap); }, Qt::QueuedConnection);
            // m_mclInit = true;
            // m_MCL = mrpt::slam::CMonteCarloLocalization2D(100);
            // m_MCL.options.metricMap = m_gridMap;
            // m_MCL.resetUniformFreeSpace(m_gridMap.get());
            std::cout << "Карта успешно загружена" << std::endl;
        } else {
            qDebug() << "map is not valid";
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка при загрузке карты: " << e.what() << std::endl;
    }

    //---------------------------- ICP --------------------------------

    // m_gridMap.setSize(-50, -50, 50, 50, 0.05f);
    mrpt::maps::TSetOfMetricMapInitializers mapInit;

    mrpt::maps::COccupancyGridMap2D::TMapDefinition gridMapDefinition;
    gridMapDefinition.resolution = 0.05f;
    mapInit.push_back(gridMapDefinition);

    mrpt::maps::CSimplePointsMap::TMapDefinition simplePointMapDefinition;
    mapInit.push_back(simplePointMapDefinition);

    //-------------- ICP_options --------------
    m_mapBuilderICP.ICP_options.mapInitializers = mapInit;
    // m_mapBuilderICP.ICP_options.mapInitializers.push_back(mapDefinition);

    // true — сопоставление сканов выполняется с COccupancyGridMap2D картой, что быстрее, но менее точно.
    // false — сопоставление с картой точек CSimplePointsMap
    m_mapBuilderICP.ICP_options.matchAgainstTheGrid = false;

    // Минимальное перемещение для выполнения ICP-коррекции позы.
    // Если перемещение меньше этих значений, коррекция пропускается, и положение обновляется только по одометрии.
    // Значения по умолчанию: 0.20 метра и 30 градусов.
    m_mapBuilderICP.ICP_options.localizationLinDistance = 0.02;
    m_mapBuilderICP.ICP_options.localizationAngDistance = mrpt::DEG2RAD(2.0);

    // Минимальное линейное (в метрах) и угловое (в радианах) перемещение
    // робота для вставки нового наблюдения в карту
    m_mapBuilderICP.ICP_options.insertionLinDistance = 0.2;
    m_mapBuilderICP.ICP_options.insertionAngDistance = mrpt::DEG2RAD(30.0);

    // Минимальное качество ICP-решения (от 0 до 1) для его принятия.
    // Значения по умолчанию: 0.40
    m_mapBuilderICP.ICP_options.minICPgoodnessToAccept = 0.60;

    //-------------- ICP_params --------------

    // Максимальное число итераций ICP
    m_mapBuilderICP.ICP_params.maxIterations = 40;
    // Пороги расстояния и угла для поиска соответствий между точками
    m_mapBuilderICP.ICP_params.thresholdDist = 0.75f;
    m_mapBuilderICP.ICP_params.thresholdAng = 0.75f;
    // Коэффициент уменьшения порогов при сходимости
    m_mapBuilderICP.ICP_params.ALFA = 0.5;


    m_mapBuilderICP.initialize(mrpt::maps::CSimpleMap());
    m_mapBuilderICP.setMinLoggingLevel(mrpt::system::LVL_ERROR);

    //---------------------------- RBPF -------------------------------
    /*
    // [RBPF_SLAM]
    // PF_algorithm = 2                     # Включает pfOptimalProposal
    // pfOptimalProposal_mapSelection = 0   # 0 = используем сетку (gridmap)
    // ICPGlobalAlign_MinQuality = 0.7      # Если совмещение хуже этого порога, RBPF откатится к сырой одометрии
    // # Параметры для управления частотой обновления
    // insertionLinDistance = 0.10      # Обновлять фильтр каждые 10 см
    // insertionAngDistance = mrpt::utils::DEG2RAD(15) # ... или каждые 15 градусов

    // [ICP]                                # Параметры самого ICP
    // maxIterations = 50
    // thresholdAng = 0.1
    // thresholdLin = 0.05
    // # ... остальные настройки ICP ...

    // Конфигурируем параметры фильтра частиц
    // m_mapBuilderRBPF.STAM_OPTIONS.proposalDistribution = mrpt::slam::CMetricMapBuilderRBPF::pfOptimalProposal;
    // m_mapBuilderRBPF.STAM_OPTIONS.particlesCount = 30; // Оптимально для баланса скорость/точность

    // Настраиваем карту: создаем сетку занятости (Occupancy Grid Map)

    // Создаем структуру с опциями для конструктора
    mrpt::slam::CMetricMapBuilderRBPF::TConstructionOptions rbpfOpts;

    rbpfOpts.predictionOptions.pfOptimalProposal_mapSelection = 0;
    rbpfOpts.predictionOptions.ICPGlobalAlign_MinQuality = 0.2; // Минимальное качество ICP для принятия

    // Настройка фильтра частиц (правильные имена полей)
    rbpfOpts.PF_options.PF_algorithm = mrpt::bayes::CParticleFilter::pfOptimalProposal;
    rbpfOpts.PF_options.sampleSize = 200;                    // количество частиц
    rbpfOpts.PF_options.resamplingMethod = mrpt::bayes::CParticleFilter::prSystematic;
    rbpfOpts.PF_options.adaptiveSampleSize = false;          // отключаем адаптивный размер
    rbpfOpts.PF_options.BETA = 0.5;                          // порог ресемплизации

    // Настройка параметров вставки и локализации
    rbpfOpts.insertionLinDistance = 0; // 0.10;                    // обновление каждые 10 см
    rbpfOpts.insertionAngDistance = 0; // mrpt::DEG2RAD(5.0);     // или 15 градусов
    rbpfOpts.localizeLinDistance = 0; // 0.05;                     // обновление позы каждые 5 см
    rbpfOpts.localizeAngDistance = 0; // mrpt::DEG2RAD(2.0);       // или 2 градуса

    // Настройка карты
    mrpt::maps::COccupancyGridMap2D::TMapDefinition gridDef;
    gridDef.resolution = 0.05f;
    gridDef.min_x = -30; gridDef.max_x = 30;
    gridDef.min_y = -30; gridDef.max_y = 30;
    rbpfOpts.mapsInitializers.push_back(gridDef);

    // Создаем объект построителя карты
    m_mapBuilderRBPF = mrpt::slam::CMetricMapBuilderRBPF(rbpfOpts);

    // Инициализируем внутреннее состояние строителя карт
    mrpt::poses::CPose2D initialPose(0, 0, 0);
    auto initialPosePDF = mrpt::poses::CPosePDFGaussian::Create();
    initialPosePDF->mean = mrpt::poses::CPose2D(0, 0, 0);
    initialPosePDF->cov = mrpt::math::CMatrixDouble33::Identity() * 0.01;
    m_mapBuilderRBPF.initialize(mrpt::maps::CSimpleMap(), initialPosePDF.get());
*/
    //---------------------------- Graph SLAM -------------------------

    // mrpt::graphslam::deciders::CICPCriteriaNRD<mrpt::graphs::CNetworkOfPoses2DInf> node_reg;
    // mrpt::graphslam::deciders::CICPCriteriaERD<mrpt::graphs::CNetworkOfPoses2DInf> edge_reg;
    // mrpt::graphslam::optimizers::CLevMarqGSO<mrpt::graphs::CNetworkOfPoses2DInf> optimizer;

    // // 2. Создание движка Graph-SLAM (без визуализации)
    // std::string config_file = "graph_slam.ini";
    // std::string rawlog_fname = "";
    // std::string fname_GT = "";

    // m_graphSlamEngine = new mrpt::graphslam::CGraphSlamEngine<mrpt::graphs::CNetworkOfPoses2DInf>(
    //             config_file,
    //             nullptr,
    //             fname_GT,
    //             nullptr,  // Нет окна визуализации
    //             &node_reg, &edge_reg, &optimizer
    //             );

    // ----------------------------------------------------------------
    // double robotX = 2.0;
    // double robotY = 1.0;

    // // Получить индексы ячейки, в которую попадает точка
    // uint32_t idxX, idxY;
    // gridMap->x2idx(robotX, idxX);
    // gridMap->y2idx(robotY, idxY);

    // // Получить координаты ЦЕНТРА ячейки по индексам
    // double cellCenterX, cellCenterY;
    // gridMap->idx2x(idxX);
    // gridMap->idx2y(idxY);

    QTimer *saveTimer = new QTimer(this);
    saveTimer->setInterval(1000);
    saveTimer->callOnTimeout(this, [this] {
        // if (!m_buildingMap) return;
        // m_mapBuilderICP.saveCurrentEstimationToImage("map_snapshot");
        // const auto& metricMap = m_mapBuilderRBPF.getCurrentlyBuiltMetricMap();
        // auto gridMap = metricMap.mapByClass<mrpt::maps::COccupancyGridMap2D>();
        // if (gridMap != nullptr) {
        //     gridMap->saveAsBitmapFile("occupancy_grid.png");
        //     qDebug() << "map saved";
        // }
        if (m_gridMap) {
            // verticalFlip = false - избегаем переворота, если он не нужен
            // forceRGB = false - получаем изображение в оттенках серого (8 бит на пиксель)
            // tricolor = false - получаем плавный градиент, а не строго 3 цвета
            // mrpt::img::CImage img;
            // gridMap->getAsImage(img, true, true, false);
            // mrpt::io::CMemoryStream memBuffer;
            // if (img.getWidth() > 0 && img.getHeight() > 0) {
            //     img.saveToStreamAsJPEG(memBuffer, 90);
            //     size_t size = memBuffer.getTotalBytesCount();
            //     if (size) {
            //         emit mapUpdated(QByteArray(reinterpret_cast<const char*>(memBuffer.getRawBufferData()), size));
            //     }
            // }
            QByteArray data;
            QDataStream stream(&data, QIODevice::WriteOnly);
            stream << m_gridMap->getSizeX() << m_gridMap->getSizeY();
            for (auto &cell : m_gridMap->getRawMap()) {
                stream << m_gridMap->l2p_255(cell);
            }
            emit mapUpdated(data);
        }
    });
    saveTimer->start();
}

Mapper::~Mapper() { }

void Mapper::startBuildingMap() {
    m_buildingMap = true;
    m_mapBuilderICP.clear();
    const auto& metricMap = m_mapBuilderICP.getCurrentlyBuiltMetricMap();
    m_gridMap = metricMap.mapByClass<mrpt::maps::COccupancyGridMap2D>();
    emit mapChanged(m_gridMap);
}

void Mapper::finishBuildingMap() {
    m_buildingMap = false;
    if (!m_gridMap) {
        mrpt::io::CFileOutputStream ofs("map.gridmap");
        // Создаем архив для записи из этого потока
        auto arch_out = mrpt::serialization::archiveFrom(ofs);
        // Сериализуем карту в архив
        arch_out << m_gridMap;
    }

    //------------------- Monte Carlo Localization --------------------
    // if (m_gridMap) {
    //     m_mclInit = true;
    //     m_MCL = mrpt::slam::CMonteCarloLocalization2D(5000);
    //     // m_MCL.options.metricMaps.clear();
    //     m_MCL.options.metricMap = m_gridMap;
    //     m_MCL.resetUniformFreeSpace(m_gridMap.get());
    //     qDebug() << "MCL start";
    // } else {
    //     qDebug() << "map is not valid";
    // }
}

void Mapper::processScan(const mrpt::obs::CObservation2DRangeScan::Ptr &obs) {
    // void Mapper::processScan(const mrpt::obs::CObservationPointCloud::Ptr &obs) {
    if (m_buildingMap) {
        //----------------------------- MOLA -------------------------------

        // Получение данных с лидара
        // int valid_count = 0;
        // for (size_t i = 0; i < observation->getScanSize(); ++i) {
        //     if (observation->getScanRangeValidity(i)) ++valid_count;
        // }
        // qDebug() << "valid:" << valid_count << "\tsize:" << observation->getScanSize()
        //          << "\tratio:" << float(valid_count) / float(observation->getScanSize());

        // Передача данных в SLAM-систему
        // m_mapper.onNewObservation(obs);

        // Получение текущей оценки положения
        // auto pose = m_mapper.get_current_pose();
        // qDebug() << "x:" << pose.x()
        //          << "\ty:" << pose.y()
        //          << "\tz:" << pose.z()
        //          << "\tyaw:" << pose.yaw()
        //          << "\tpitch:" << pose.pitch()
        //          << "\troll:" << pose.roll();

        // qDebug() << "\n===================================\n";
        // qDebug() << "initialize:" << m_mapper.isActive();
        // qDebug() << "busy:" << m_mapper.isBusy();

        // auto& state = m_mapper.get_state();
        // qDebug() << "Keyframes count:" << state.time_to_kf_id.size() << "\t" << map.size();
        // qDebug() << "Связей в графе:" << state.get_kf_connectivity().edgeCount();
        // qDebug() << "GTSAM факторов:" << state.gtsam_data.graph_factors.size();
        // qDebug() << "GTSAM переменных:" << state.gtsam_data.graph_values.size();
        // qDebug() << "Generators count:" << state.pointcloud_generators.size();
        // qDebug() << "Filters count:" << state.pointcloud_filters.size();
        // qDebug() << "Last localization time:" << (state.last_localization_time.has_value() ? "yes" : "no");
        // qDebug() << "Last pose: x=" << state.last_localization.x() << " y=" << state.last_localization.y();
        // qDebug() << "Last localization:" << state.last_localization_time.has_value();
        // qDebug() << "obs name:" << obs->sensorLabel;

        //----------------------------- ICP -------------------------------

        // auto icp_obs = obs->duplicateGetSmartPtr();
        m_mapBuilderICP.processObservation(obs);

        // const auto& metricMap = m_mapBuilderICP.getCurrentlyBuiltMetricMap();
        // m_gridMap = metricMap.mapByClass<mrpt::maps::COccupancyGridMap2D>();

        // // Получение текущей оценки положения робота
        mrpt::poses::CPose3DPDF::Ptr posePDF = m_mapBuilderICP.getCurrentPoseEstimation();
        if (posePDF) {
            mrpt::poses::CPose3D currentPose;
            posePDF->getMean(currentPose);
            // std::cout << "Текущее положение: " << currentPose << std::endl;
            // qDebug() << "Текущее положение ICP: \nx =" << currentPose.x() << "\ty =" << currentPose.y() << "\tyaw" << mrpt::RAD2DEG(currentPose.yaw());
            emit positionChanged(currentPose.x(), currentPose.y(), currentPose.yaw());

            if (m_gridMap) {
                QPoint cellPos(m_gridMap->x2idx(currentPose.x()), m_gridMap->y2idx(currentPose.y()));
                if (m_currentCellPos != cellPos) {
                    m_currentCellPos = cellPos;
                    emit cellPositionChanged(cellPos);
                }
            }
        }

        //---------------------------- RBPF -------------------------------
        /*
    // Создаем действие перемещения с высокой ковариацией
    // mrpt::obs::CActionRobotMovement2D::TMotionModelOptions opts;
    // opts.modelSelection = mrpt::obs::CActionRobotMovement2D::mmGaussian;
    // opts.gaussianModel.a1 = 0.001;        // неопределенность от поступательного движения
    // opts.gaussianModel.a2 = 0.001;        // неопределенность от поворота
    // opts.gaussianModel.a3 = 0.001;        // неопределенность от поступательного движения (для угла)
    // opts.gaussianModel.a4 = 0.001;        // неопределенность от поворота (для угла)
    // opts.gaussianModel.minStdXY = 0.02;  // минимальная неопределенность позиции (метры)
    // opts.gaussianModel.minStdPHI = mrpt::DEG2RAD(0.5); // минимальная неопределенность угла (радианы)

    // mrpt::obs::CActionRobotMovement2D odomAction;
    // odomAction.estimationMethod = mrpt::obs::CActionRobotMovement2D::emOdometry;
    // odomAction.computeFromOdometry(mrpt::poses::CPose2D(0, 0, 0), opts);

    //---------------------------------------------------------------------

    // 1. ВЫЧИСЛЯЕМ СМЕЩЕНИЕ ЧЕРЕЗ ICP (вместо нулевого действия)
    // static mrpt::obs::CObservation2DRangeScan::Ptr lastScan = mrpt::obs::CObservation2DRangeScan::Create();
    // static mrpt::poses::CPose2D lastPose(0, 0, 0);

    // mrpt::poses::CPose2D deltaPose(0, 0, 0); // смещение между сканами

    // // 2. Создаем действие с РЕАЛЬНЫМ смещением (полученным от ICP)
    // mrpt::obs::CActionRobotMovement2D::TMotionModelOptions opts;
    // opts.modelSelection = mrpt::obs::CActionRobotMovement2D::mmGaussian;
    // opts.gaussianModel.a1 = 0.001;
    // opts.gaussianModel.a2 = 0.001;
    // opts.gaussianModel.a3 = 0.001;
    // opts.gaussianModel.a4 = 0.001;
    // opts.gaussianModel.minStdXY = 0.10;
    // opts.gaussianModel.minStdPHI = mrpt::DEG2RAD(2.0);

    // mrpt::obs::CActionRobotMovement2D odomAction;
    // odomAction.estimationMethod = mrpt::obs::CActionRobotMovement2D::emOdometry;

    // // if (lastScan) {
    // // 1. Преобразуем сканы в точечные карты (если нужно)
    // mrpt::maps::CSimplePointsMap map1, map2;
    // map1.insertObservationPtr(obs);
    // map2.insertObservationPtr(lastScan);

    // // 2. Создаем и настраиваем ICP
    // mrpt::slam::CICP icp;
    // icp.options.maxIterations = 50;
    // icp.options.thresholdDist = 0.75f;
    // icp.options.thresholdAng = 0.75f;

    // // 3. Выполняем совмещение с начальной оценкой (0, 0, 0)
    // // Создаем PDF с начальной оценкой
    // mrpt::poses::CPose2D initialGuess(0, 0, 0);
    // mrpt::poses::CPosePDFGaussian initialEstimate(initialGuess);

    // // Вызываем AlignPDF с PDF-оценкой
    // mrpt::poses::CPosePDF::Ptr pdf = icp.AlignPDF(&map1, &map2, initialEstimate);

    // // 4. Извлекаем среднее значение (смещение)
    // if (pdf) {
    //     deltaPose = pdf->getMeanVal();
    //     // qDebug() << "ICP смещение: dx=" << deltaPose.x()
    //     //          << " dy=" << deltaPose.y()
    //     //          << " dphi=" << mrpt::RAD2DEG(deltaPose.phi());
    //     // Извлекаем стандартное отклонение из ковариационной матрицы
    //     auto [cov, meanPose] = pdf->getCovarianceAndMean();
    //     deltaPose = meanPose; // или pdf->getMeanVal()

    //     double stdX = std::sqrt(cov(0,0));
    //     double stdY = std::sqrt(cov(1,1));
    //     double stdPhi = std::sqrt(cov(2,2));

    //     // Используем неопределенность из ICP
    //     opts.gaussianModel.minStdXY = std::max(0.02, std::max(stdX, stdY));
    //     opts.gaussianModel.minStdPHI = std::max(mrpt::DEG2RAD(0.5), stdPhi);
    // }
    // // }

    // odomAction.computeFromOdometry(deltaPose, opts);

    // // Сохраняем текущий скан для следующего шага
    // *lastScan = *obs;

    //---------------------------------------------------------------------

    // mrpt::obs::CActionCollection actionCollection;
    // actionCollection.insert(odomAction);

    // // sensory frame
    // mrpt::obs::CSensoryFrame sensoryFrame;
    // sensoryFrame.push_back(obs);
    // m_mapBuilderRBPF.processActionObservation(actionCollection, sensoryFrame);

    // // получение позиции
    // mrpt::poses::CPose3DPDF::Ptr poseRBPF = m_mapBuilderRBPF.getCurrentPoseEstimation();
    // if (poseRBPF) {
    //     mrpt::poses::CPose3D currentPose;
    //     poseRBPF->getMean(currentPose);
    //     qDebug() << "Текущее положение RBPF: \nx =" << currentPose.x() << "\ty =" << currentPose.y() << "\tyaw" << mrpt::RAD2DEG(currentPose.yaw());
    // }
*/
        //---------------------------- Graph SLAM -------------------------

        // static size_t measurement_count = 0;
        // auto obs_ptr = std::dynamic_pointer_cast<mrpt::obs::CObservation>(obs);
        // m_graphSlamEngine->execGraphSlamStep(obs_ptr, measurement_count);
        // measurement_count++;
    } else if (m_currentObs && m_gridMap) {
        mrpt::slam::CICP icp;
        icp.options.maxIterations = m_mapBuilderICP.ICP_params.maxIterations;
        icp.options.thresholdDist = m_mapBuilderICP.ICP_params.thresholdDist;
        icp.options.thresholdAng = m_mapBuilderICP.ICP_params.thresholdAng;
        icp.options.ICP_algorithm = mrpt::slam::icpClassic;
        icp.options.ALFA = m_mapBuilderICP.ICP_params.ALFA;

        mrpt::maps::CSimplePointsMap map1, map2;
        map1.insertObservationPtr(obs);
        map2.insertObservationPtr(m_currentObs);
        mrpt::math::TPose2D mclPose = m_MCL.getMostLikelyParticle();
        mrpt::poses::CPose2D initialPose(mclPose.x, mclPose.y, mclPose.phi);
        mrpt::poses::CPosePDF::Ptr pdf = icp.AlignPDF(
                    &map1, &map2,
                    mrpt::poses::CPosePDFGaussian(initialPose)
                    );
        mrpt::poses::CPose2D deltaPose(0, 0, 0);
        if (pdf) {
            deltaPose = pdf->getMeanVal();
            // qDebug() << "ICP delta: dx=" << deltaPose.x()
            //          << " dy=" << deltaPose.y()
            //          << " dphi=" << mrpt::RAD2DEG(deltaPose.phi());
        }
        /*
        mrpt::obs::CActionRobotMovement2D odomAction;

        mrpt::obs::CActionRobotMovement2D::TMotionModelOptions opts;
        opts.modelSelection = mrpt::obs::CActionRobotMovement2D::mmGaussian;
        opts.gaussianModel.a1 = 0.05;
        opts.gaussianModel.a2 = 0.05;
        opts.gaussianModel.a3 = 0.01;
        opts.gaussianModel.a4 = 0.01;
        opts.gaussianModel.minStdXY = 0.02;
        opts.gaussianModel.minStdPHI = mrpt::DEG2RAD(2.0);
        odomAction.computeFromOdometry(deltaPose, opts);
        odomAction.timestamp = obs->timestamp;
        // opts.modelSelection = mrpt::obs::CActionRobotMovement2D::mmGaussian;
        // opts.gaussianModel.a1 = 0.5;
        // opts.gaussianModel.a2 = 0.5;
        // opts.gaussianModel.a3 = 0.1;
        // opts.gaussianModel.a4 = 0.1;
        // opts.gaussianModel.minStdXY = 0.1;
        // opts.gaussianModel.minStdPHI = mrpt::DEG2RAD(5.0);
        // odomAction.computeFromOdometry(mrpt::poses::CPose2D(0,0,0), opts);

        mrpt::obs::CActionCollection actionCollection;
        actionCollection.insert(odomAction);

        mrpt::obs::CSensoryFrame sensory_frame;
        sensory_frame.insert(obs);

        // НАСТРОЙКА ПАРАМЕТРОВ ФИЛЬТРАЦИИ
        mrpt::bayes::CParticleFilter::TParticleFilterOptions pfOptions;
        pfOptions.resamplingMethod = mrpt::bayes::CParticleFilter::prSystematic;
        pfOptions.sampleSize = 5000;
        pfOptions.adaptiveSampleSize = true;  // Включаем адаптивный размер
        pfOptions.BETA = 0.3;                 // Порог ресемплинга (0-1)
        pfOptions.PF_algorithm = mrpt::bayes::CParticleFilter::pfStandardProposal;

        // KLD-параметры для адаптации количества частиц
        mrpt::slam::TKLDParams kldParams;
        kldParams.KLD_binSize_XY = 0.2;      // Размер бина для XY (метры)
        kldParams.KLD_binSize_PHI = mrpt::DEG2RAD(5.0); // Размер бина для угла
        kldParams.KLD_delta = 0.01;          // Доверительный интервал
        kldParams.KLD_epsilon = 0.01;        // Ошибка аппроксимации
        kldParams.KLD_maxSampleSize = 5000;  // Максимум частиц
        kldParams.KLD_minSampleSize = 500;   // Минимум частиц

        try {
            m_MCL.prediction_and_update_pfStandardProposal(
                        &actionCollection,
                        &sensory_frame,
                        mrpt::bayes::CParticleFilter::TParticleFilterOptions()
                        );
            mrpt::math::TPose2D currentPose = m_MCL.getMostLikelyParticle();
            emit positionChanged(currentPose.x, currentPose.y, currentPose.phi);

            QPoint cellPos(m_gridMap->x2idx(currentPose.x), m_gridMap->y2idx(currentPose.y));
            if (m_currentCellPos != cellPos) {
                m_currentCellPos = cellPos;
                emit cellPositionChanged(cellPos);
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
*/
    }
    m_currentObs = obs;
}
