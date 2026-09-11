#include "firmwareloader.h"
#include "appglobal.h"

#if QT_CONFIG(process)

const QString FirmwareLoader::sketch_path = AppGlobal::tempLocation() + "firmware/firmware.ino";
const QString FirmwareLoader::arduino_cli_path = "arduino-cli";

FirmwareLoader::FirmwareLoader() : m_state(Inactive), m_process(new QProcess(this))
{
    connect(m_process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        //qDebug() << exitCode << exitStatus;
        const QString location = AppGlobal::tempLocation() + "firmware/";
        if (exitCode == 0 && exitStatus == QProcess::NormalExit && m_state == Compile) {
            //qDebug() << "\nupload\n";
            /*QStringList args {
                "upload", "-p", m_portName,
                "--fqbn", "STMicroelectronics:stm32:GenF1",
                "--board-options", "pnum=GENERIC_F103RETX,xserial=generic,usb=CDCgen,xusb=FS,opt=osstd,dbg=none,rtlib=nano,upload_method=hidMethod",
                location + "firmware.ino", "--build-path", location + "build"//, "-v"
            };*/
            /*QStringList args {
                "upload", "-p", m_portName,
                "--fqbn", "STMicroelectronics:stm32:GenF4",
                "--board-options", "pnum=GENERIC_F413ZGTX,xserial=generic,usb=CDCgen,xusb=FS,opt=osstd,dbg=none,rtlib=nano,upload_method=hidMethod",
                location + "firmware.ino", "--build-path", location + "build"//, "-v"
            };*/
            QStringList args {
                "upload", "-p", m_portName,
                "--fqbn", m_fqbn,
                "--board-options", m_boardOptions,
                location + "firmware.ino", "--build-path", location + "build"//, "-v"
            };
            m_state = Upload;
            m_process->start("arduino-cli", args);
        } else {
            setState(Inactive);
        }
    });
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        qDebug() << QString(m_process->readAllStandardOutput());
    });
    connect(m_process, &QProcess::readyReadStandardError, this, [this] {
        qDebug() << QString(m_process->readAllStandardError());
    });
    connect(m_process, &QProcess::errorOccurred, this, [](QProcess::ProcessError error) {
        qDebug() << error;
    });
}

void FirmwareLoader::loadFirmware(const QByteArray &code, const QString &portName,
                                  const QString &fqbn, const QString &board_options)
{
    const QString location = AppGlobal::tempLocation() + "firmware/";
    if (!QDir(location).exists()) QDir().mkpath(location);
    QFile sketch(location + "firmware.ino");
    if (!sketch.open(QIODevice::WriteOnly)) {
        qDebug() << "Не удалось записать скетч";
        return;
    }
    sketch.write(code);
    sketch.close();
    m_state = Compile;
    m_portName = portName;
    m_fqbn = fqbn;
    m_boardOptions = board_options;
    QStringList args {
        "compile",
        "--fqbn", fqbn,
        "--board-options", board_options,
        sketch.fileName(), "--build-path", location + "build"//, "-v"
    };
    m_process->start("arduino-cli", args);
}

#else

FirmwareLoader::FirmwareLoader() : m_state(Inactive)
{
}

void FirmwareLoader::loadFirmware(const QByteArray &code, const QString &portName,
                                  const QString &fqbn, const QString &board_options)
{
}

#endif

void FirmwareLoader::setState(State state)
{
    if (state == m_state) return;
    m_state = state;
    emit stateChanged();
}
