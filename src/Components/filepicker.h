#ifndef FILEPICKER_H
#define FILEPICKER_H

#include <QtQmlIntegration>
#include <QQuickItem>

#ifndef __EMSCRIPTEN__

#include <QQmlProperty>

class FilePicker : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int fileMode READ fileMode WRITE setFileMode NOTIFY fileModeChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString acceptLabel READ acceptLabel WRITE setAcceptLabel NOTIFY acceptLabelChanged)
    Q_PROPERTY(QString currentFolder READ currentFolder WRITE setCurrentFolder NOTIFY currentFolderChanged)
    Q_PROPERTY(QString defaultSuffix READ defaultSuffix WRITE setDefaultSuffix NOTIFY defaultSuffixChanged)
    Q_PROPERTY(QString selectedFile READ selectedFile WRITE setSelectedFile NOTIFY selectedFileChanged)
    Q_PROPERTY(QList<int> nameFilters READ nameFilters WRITE setNameFilters NOTIFY nameFiltersChanged)

public:
    enum FileMode: uint8_t {
        OpenFile, OpenFiles, SaveFile
    };
    enum AcceptableFiles: uint8_t {
        All, Text, JSON, CPP, Py, Images
    };
    Q_ENUM(FileMode)
    Q_ENUM(AcceptableFiles)
    FilePicker(QObject *parent = nullptr);
    inline ~FilePicker() { delete m_fileDialog; }

    void classBegin() override;
    void componentComplete() override;

    Q_INVOKABLE void open();
    Q_INVOKABLE void save(const QByteArray &data);
    Q_INVOKABLE void saveAs(const QByteArray &data);

    inline int fileMode() const { return QQmlProperty::read(m_fileDialog, "fileMode").toInt(); };
    inline QString title() const { return QQmlProperty::read(m_fileDialog, "title").toString(); };
    inline QString acceptLabel() const { return QQmlProperty::read(m_fileDialog, "acceptLabel").toString(); };
    inline QString currentFolder() const { return QQmlProperty::read(m_fileDialog, "currentFolder").toUrl().toLocalFile(); };
    inline QString defaultSuffix() const { return QQmlProperty::read(m_fileDialog, "defaultSuffix").toString(); };
    inline QString selectedFile() const { return QQmlProperty::read(m_fileDialog, "selectedFile").toUrl().toLocalFile(); };
    inline QList<int> nameFilters() const { return m_nameFilters; };

    inline void setFileMode(int fileMode) {
        QQmlProperty::write(m_fileDialog, "fileMode", fileMode);
    }
    inline void setTitle(const QString &title) {
        QQmlProperty::write(m_fileDialog, "title", title);
    }
    inline void setAcceptLabel(const QString &acceptLabel) {
        QQmlProperty::write(m_fileDialog, "acceptLabel", acceptLabel);
    }
    inline void setCurrentFolder(const QString &currentFolder) {
        QQmlProperty::write(m_fileDialog, "currentFolder", QUrl::fromLocalFile(currentFolder));
    }
    inline void setDefaultSuffix(const QString &defaultSuffix) {
        QQmlProperty::write(m_fileDialog, "defaultSuffix", defaultSuffix);
    }
    inline void setSelectedFile(const QString &selectedFile) {
        QQmlProperty::write(m_fileDialog, "selectedFile", QUrl::fromLocalFile(selectedFile));
    }
    void setNameFilters(const QList<int> &nameFilters);

private:
    QByteArray m_buffer;
    QList<int> m_nameFilters;
    QObject *m_fileDialog;

private slots:
    void fileHandler();

signals:
    void fileModeChanged();
    void titleChanged();
    void acceptLabelChanged();
    void currentFolderChanged();
    void defaultSuffixChanged();
    void selectedFileChanged();
    void nameFiltersChanged();
    void fileReaded(const QByteArray &data);
    void closed();
};

#else

class FilePicker : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int fileMode READ fileMode WRITE setFileMode NOTIFY fileModeChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString acceptLabel READ acceptLabel WRITE setAcceptLabel NOTIFY acceptLabelChanged)
    Q_PROPERTY(QString currentFolder READ currentFolder WRITE setCurrentFolder NOTIFY currentFolderChanged)
    Q_PROPERTY(QString defaultSuffix READ defaultSuffix WRITE setDefaultSuffix NOTIFY defaultSuffixChanged)
    Q_PROPERTY(QString selectedFile READ selectedFile WRITE setSelectedFile NOTIFY selectedFileChanged)
    Q_PROPERTY(QList<int> nameFilters READ nameFilters WRITE setNameFilters NOTIFY nameFiltersChanged)

public:
    enum FileMode: uint8_t {
        OpenFile, OpenFiles, SaveFile
    };
    enum AcceptableFiles: uint8_t {
        All, Text, JSON, CPP, Py, Images
    };
    Q_ENUM(FileMode)
    Q_ENUM(AcceptableFiles)
    FilePicker(QObject *parent = nullptr);
    Q_INVOKABLE void open();
    Q_INVOKABLE void save(const QByteArray &data);
    Q_INVOKABLE void saveAs(const QByteArray &data);

    inline int fileMode() const { return m_fileMode; };
    inline QString title() const { return m_title; };
    inline QString acceptLabel() const { return m_acceptLabel; };
    inline QString currentFolder() const { return m_currentFolder; };
    inline QString defaultSuffix() const { return m_defaultSuffix; };
    inline QString selectedFile() const { return m_selectedFile; };
    inline QList<int> nameFilters() const { return m_nameFilters; };

    void setFileMode(int fileMode);
    void setTitle(const QString &title);
    void setAcceptLabel(const QString &acceptLabel);
    void setCurrentFolder(const QString &currentFolder);
    void setDefaultSuffix(const QString &defaultSuffix);
    void setSelectedFile(const QString &selectedFile);
    void setNameFilters(const QList<int> &nameFilters);

    static void readFile(uintptr_t ptr, const std::string &data);
    static void clearBuffer(uintptr_t ptr);

private:
    int m_fileMode;
    QString m_title;
    QString m_acceptLabel;
    QString m_currentFolder;
    QString m_defaultSuffix;
    QString m_selectedFile;
    QByteArray m_types;
    QByteArray m_buffer;
    QList<int> m_nameFilters;

signals:
    void fileModeChanged();
    void titleChanged();
    void acceptLabelChanged();
    void currentFolderChanged();
    void defaultSuffixChanged();
    void selectedFileChanged();
    void nameFiltersChanged();
    void fileReaded(const QString &data);
    void closed();
};

#endif // __EMSCRIPTEN__

#endif // FILEPICKER_H
