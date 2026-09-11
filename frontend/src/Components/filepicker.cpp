#include "filepicker.h"
#include <QNetworkAccessManager>
#include <QHostAddress>

#ifndef __EMSCRIPTEN__

#include <QFile>

FilePicker::FilePicker(QObject *parent) : QObject(parent) {}

void FilePicker::classBegin()
{
    QQmlEngine *engine = qmlEngine(this);
    if (!engine) return;
    QQmlComponent component(engine, this);
    QByteArray data = "import QtQuick; import QtQuick.Dialogs; FileDialog {}";
    component.setData(data, QUrl());
    m_fileDialog = component.create();
    m_fileDialog->setParent(this);

    connect(m_fileDialog, SIGNAL(fileModeChanged()), this, SIGNAL(fileModeChanged()));
    connect(m_fileDialog, SIGNAL(titleChanged()), this, SIGNAL(titleChanged()));
    connect(m_fileDialog, SIGNAL(acceptLabelChanged()), this, SIGNAL(acceptLabelChanged()));
    connect(m_fileDialog, SIGNAL(currentFolderChanged()), this, SIGNAL(currentFolderChanged()));
    connect(m_fileDialog, SIGNAL(defaultSuffixChanged()), this, SIGNAL(defaultSuffixChanged()));
    connect(m_fileDialog, SIGNAL(selectedFileChanged()), this, SIGNAL(selectedFileChanged()));

    connect(m_fileDialog, SIGNAL(accepted()), this, SLOT(fileHandler()));
    connect(m_fileDialog, SIGNAL(rejected()), this, SIGNAL(closed()));

    /*
    connect(this, &FilePicker::fileModeChanged,
            this, [this]{ QQmlProperty::write(m_fileDialog, "fileMode", m_fileMode); });
    connect(this, &FilePicker::titleChanged,
            this, [this]{ QQmlProperty::write(m_fileDialog, "title", m_title); });
    connect(this, &FilePicker::acceptLabelChanged,
            this, [this]{ QQmlProperty::write(m_fileDialog, "acceptLabel", m_acceptLabel); });
    connect(this, &FilePicker::currentFolderChanged,
            this, [this]{ QQmlProperty::write(m_fileDialog, "currentFolder", m_currentFolder); });
    connect(this, &FilePicker::defaultSuffixChanged,
            this, [this]{ QQmlProperty::write(m_fileDialog, "defaultSuffix", m_defaultSuffix); });
    connect(this, &FilePicker::nameFiltersChanged,
            this, [this]{ QQmlProperty::write(m_fileDialog, "nameFilters", m_nameFilters); });
    */
}

void FilePicker::componentComplete() {}

void FilePicker::open() {
    QQmlProperty::write(m_fileDialog, "fileMode", OpenFile);
    QMetaObject::invokeMethod(m_fileDialog, "open");
}

void FilePicker::save(const QByteArray &data) {
    m_buffer = data;
    fileHandler();
}

void FilePicker::saveAs(const QByteArray &data) {
    m_buffer = data;
    QQmlProperty::write(m_fileDialog, "fileMode", SaveFile);
    QMetaObject::invokeMethod(m_fileDialog, "open");
}

void FilePicker::setNameFilters(const QList<int> &nameFilters) {
    if (nameFilters == m_nameFilters) return;
    QStringList types;
    for (const int &acceptableFile : nameFilters) {
        switch (acceptableFile) {
        case All: types << "Все файлы (*.*)"; break;
        case Text: types << "Текстовые файлы (*.txt)"; break;
        case JSON: types << "Файлы JSON (*.json)"; break;
        default: continue;
        }
    }
    QQmlProperty::write(m_fileDialog, "nameFilters", types);
    emit nameFiltersChanged();
}

void FilePicker::fileHandler()
{
    switch (fileMode()) {
    case OpenFile: {
        QFile file(selectedFile());
        if (!file.exists()) {
            qDebug() << "файл не существует";
            return;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "недостаточно прав для открытия файла";
            return;
        }
        QByteArray data = file.readAll();
        file.close();
        if (data.isEmpty()) {
            qDebug() << "файл пуст";
            return;
        }
        emit fileReaded(data);
        emit closed();
    }
        break;
    case SaveFile: {
        QFile file(selectedFile());
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "недостаточно прав для открытия файла";
            return;
        }
        file.write(m_buffer);
        file.close();
        m_buffer.clear();
        emit closed();
    }
        break;
    default:
        break;
    }
}

#else

#include <emscripten.h>
#include <emscripten/bind.h>

FilePicker::FilePicker(QObject *parent) : QObject(parent) {
    connect(this, &FilePicker::fileReaded, this, [](const QString &data) {
        QNetworkAccessManager networkManager;
        networkManager.connectToHost(QHostAddress(QHostAddress::Any).toString(), 80);
        QNetworkRequest request(QUrl("device"));
        networkManager.post(request, data.toUtf8());
    });
}

void FilePicker::open() {
    QByteArray accept;
    for (const int &acceptableFile : m_nameFilters) {
        switch (acceptableFile) {
        case Text:
            accept += "text/plain,";
            break;
        case JSON:
            accept += "application/json,";
            break;
        case CPP:
            accept += "text/x-c,text/x-c++src";
            break;
        case Py:
            accept += "text/py,";
            break;
        default:
            continue;
        }
    }
    EM_ASM({
               const input = document.createElement('input');
               input.type = 'file';
               input.accept = UTF8ToString($0);
               input.multiple = false;
               input.onchange = function(e) {
                   if (e.target.files.length > 0) {
                       const reader = new FileReader();
                       reader.onload = function(e) {
                           Module.readFile($1, e.target.result);
                       };
                       reader.readAsArrayBuffer(e.target.files[0]);
                   };
               };
               input.click();
           }, accept.data(), reinterpret_cast<uintptr_t>(this));
    /*
    EM_ASM({
               const tempButton = document.createElement('button');
               tempButton.onclick = async() => {
                   const fileHandle = await window.showOpenFilePicker({
                       multiple: false,
                       excludeAcceptAllOption: $0,
                       types: JSON.parse(UTF8ToString($1))
                   });
                   const file = await fileHandle[0].getFile();
                   await Module.readFile($2, await file.arrayBuffer());
               };
               tempButton.click();
           }, !m_nameFilters.contains(All), m_types.data(), this);
    */
}

void FilePicker::save(const QByteArray &data)
{
    if (data.isEmpty()) return;
    m_buffer = data;
    EM_ASM({
               const blob = new Blob([UTF8ToString($0)]);
               const url = URL.createObjectURL(blob);
               const a = document.createElement('a');
               a.href = url;
               a.download = UTF8ToString($1);
               a.click();
               URL.revokeObjectURL(url);
           }, m_buffer.data(), m_selectedFile.toUtf8().data());
}

void FilePicker::saveAs(const QByteArray &data)
{
    if (data.isEmpty()) return;
    m_buffer = data;
    EM_ASM({
               const tempButton = document.createElement('button');
               tempButton.onclick = async() => {
                   const file = await window.showSaveFilePicker({
                       multiple: false,
                       excludeAcceptAllOption: $0,
                       types: JSON.parse(UTF8ToString($1))
                   });
                   const writable = await file.createWritable();
                   writable.write(UTF8ToString($2));
                   writable.close();
                   await Module.clearBuffer($3);
               };
               tempButton.click();
           }, !m_nameFilters.contains(All), m_types.data(), m_buffer.data(), &m_buffer);
}

void FilePicker::setFileMode(int fileMode) {
    if (fileMode == m_fileMode) return;
    m_fileMode = fileMode;
    emit fileModeChanged();
}

void FilePicker::setTitle(const QString &title) {
    if (title == m_title) return;
    m_title = title;
    emit titleChanged();
}

void FilePicker::setAcceptLabel(const QString &acceptLabel) {
    if (acceptLabel == m_acceptLabel) return;
    m_acceptLabel = acceptLabel;
    emit acceptLabelChanged();
}

void FilePicker::setCurrentFolder(const QString &currentFolder) {
    if (currentFolder == m_currentFolder) return;
    m_currentFolder = currentFolder;
    emit currentFolderChanged();
}

void FilePicker::setDefaultSuffix(const QString &defaultSuffix) {
    if (defaultSuffix == m_defaultSuffix) return;
    m_defaultSuffix = defaultSuffix;
    emit defaultSuffixChanged();
}

void FilePicker::setSelectedFile(const QString &selectedFile) {
    if (selectedFile == m_selectedFile) return;
    m_selectedFile = selectedFile;
    emit selectedFileChanged();
}

void FilePicker::setNameFilters(const QList<int> &nameFilters)
{
    if (nameFilters == m_nameFilters) return;
    m_nameFilters = nameFilters;
    m_types.clear();
    QJsonArray types;
    for (const int &acceptableFile : nameFilters) {
        QString description; QJsonObject accept;
        switch (acceptableFile) {
        case All:
            break;
        case Text:
            description = "Текстовые файлы";
            accept["text/plain"] = QJsonArray({".txt"});
            break;
        case JSON:
            description = "Файлы JSON";
            accept["text/json"] = QJsonArray({".json"});
            break;
        case CPP:
            description = "Файлы исходных текстов";
            accept["text/cpp"] = QJsonArray({".cpp"});
            break;
        default:
            continue;
        }
        QJsonObject type;
        type["accept"] = accept;
        type["description"] = description;
        types << type;
    }
    m_types = QJsonDocument(types).toJson(QJsonDocument::Compact);
    emit nameFiltersChanged();
}

EMSCRIPTEN_BINDINGS(filepicker) {
    emscripten::function("readFile", &FilePicker::readFile);
    emscripten::function("clearBuffer", &FilePicker::clearBuffer);
}

void FilePicker::readFile(uintptr_t ptr, const std::string &data) {
    emit reinterpret_cast<FilePicker*>(ptr)->fileReaded(QString::fromStdString(data));
}

void FilePicker::clearBuffer(uintptr_t ptr) {
    reinterpret_cast<QByteArray*>(ptr)->clear();
}

#endif //__EMSCRIPTEN__
