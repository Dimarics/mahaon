#ifndef FIRMWARELOADER_H
#define FIRMWARELOADER_H

#include <QtQmlIntegration>

#if QT_CONFIG(process)

#include <QProcess>

class FirmwareLoader : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
public:
    enum State { Inactive, Compile, Upload };
    Q_ENUM(State)
    FirmwareLoader();
    inline State state() const { return m_state; }
    Q_INVOKABLE void loadFirmware(const QByteArray &code, const QString &portName,
                                  const QString &fqbn, const QString &board_options);

protected:
    void setState(State state);

private:
    static const QString arduino_cli_path;
    static const QString sketch_path;
    State m_state;
    QString m_portName;
    QProcess *m_process;
    QString m_fqbn;
    QString m_boardOptions;

signals:
    void stateChanged();
};

#else

class FirmwareLoader : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
public:
    enum State { Inactive, Compile, Upload };
    Q_ENUM(State)
    FirmwareLoader();
    inline State state() const { return m_state; }
    Q_INVOKABLE void loadFirmware(const QByteArray &code, const QString &portName,
                                  const QString &fqbn, const QString &board_options);

protected:
    void setState(State state);

private:
    State m_state;
    QString m_portName;
    QString m_fqbn;
    QString m_boardOptions;

signals:
    void stateChanged();
};

#endif

#endif // FIRMWARELOADER_H
