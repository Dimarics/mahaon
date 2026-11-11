#include "appglobal.h"
#include "websocketinterface.h"
#include <QGuiApplication>
#include <QKeyEvent>

AppGlobal::AppGlobal() : m_interface(new WebSocketInterface(this)) {
    // new SerialPortInterface();
    qApp->installEventFilter(this);
}

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

bool AppGlobal::eventFilter(QObject *target, QEvent *event) {
    if (target == qApp->focusObject() &&
            (target->metaObject()->className() == QStringLiteral("QQuickRootItem") ||
             target->metaObject()->className() == QStringLiteral("QQuickWindowQmlImpl"))) {
        switch (event->type()) {
        case QEvent::KeyPress:
            if (static_cast<QKeyEvent*>(event)->isAutoRepeat()) return false;
            emit keyPressed(static_cast<QKeyEvent*>(event)->nativeScanCode());
            break;
        case QEvent::KeyRelease:
            if (static_cast<QKeyEvent*>(event)->isAutoRepeat()) return false;
            emit keyReleased(static_cast<QKeyEvent*>(event)->nativeScanCode());
            break;
        default:
            break;
        }
    }
    return false;
}
