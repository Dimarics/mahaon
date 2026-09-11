#include "appglobal.h"
#include "websocketinterface.h"

AppGlobal::AppGlobal() : m_interface(new WebSocketInterface(this)) { }

void AppGlobal::setInterface(QObject *interface)
{
    if (interface == m_interface) return;
    if (m_interface) m_interface->deleteLater(); // QMetaObject::invokeMethod(m_device, "destroy");
    m_interface = interface;
    emit interfaceChanged();
}

void AppGlobal::saveFile(const QByteArray &data, const QString &path) const
{
    QFileInfo fileInfo(path);
    if (!fileInfo.dir().exists()) QDir().mkpath(fileInfo.path());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Не удалось сохранить файл";
        return;
    }
    //file.write(data.toUtf8());
    file.write(data);
}

void AppGlobal::saveFile(const QByteArray &data, const QUrl &url) const
{
    saveFile(data, url.toLocalFile());
}

QByteArray AppGlobal::readFile(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Невозможно открыть файл";
        return {};
    }
    return file.readAll();
}

QByteArray AppGlobal::readFile(const QUrl &url) const
{
    return readFile(url.toLocalFile());
}

QVariant AppGlobal::readJSON(const QString &path) const
{
    QJsonParseError error;
    QByteArray rawData = readFile(path);
    if (rawData.isEmpty()) {
        qWarning() << "Файл пуст или не существует"; return {};
    }
    QJsonDocument json = QJsonDocument::fromJson(rawData, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << error.errorString(); return {};
    }
    return json.toVariant();
}

QVariant AppGlobal::readJSON(const QUrl &url) const
{
    return readJSON(url.toLocalFile());
}
