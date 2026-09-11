#ifndef APPGLOBAL_H
#define APPGLOBAL_H

#pragma once

#include <QtQmlIntegration>
#include <QQmlEngine>

class AppGlobal : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString tempLocation READ tempLocation CONSTANT)
    Q_PROPERTY(QString appDataLocation READ appDataLocation CONSTANT)
    Q_PROPERTY(QObject *interface READ interface WRITE setInterface NOTIFY interfaceChanged)

public:
    AppGlobal();
    static inline QString tempLocation() { return QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/Hobots/"; }
    static inline QString appDataLocation() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/"; }
    static inline AppGlobal* instance() { static AppGlobal *instance = new AppGlobal; return instance; };
    static inline AppGlobal* qmlInstance(QQmlEngine*, QJSEngine*) { return instance(); }
    void setInterface(QObject *interface);
    inline QObject *interface() const { return m_interface; }
    Q_INVOKABLE void saveFile(const QByteArray &data, const QString &path) const;
    Q_INVOKABLE void saveFile(const QByteArray &data, const QUrl &url) const;
    Q_INVOKABLE QByteArray readFile(const QString &path) const;
    Q_INVOKABLE QByteArray readFile(const QUrl &url) const;
    Q_INVOKABLE QVariant readJSON(const QString &path) const;
    Q_INVOKABLE QVariant readJSON(const QUrl &url) const;

private:
    QObject *m_interface;

signals:
    void interfaceChanged();
};

#endif // APPBACKEND_H
