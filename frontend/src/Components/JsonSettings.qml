import QtQuick
import App

QtObject {
    property string path
    property list<string> categories
    signal loaded
    signal saved
    function properties() {
        let keys = []
        const exceptions = ["objectName", "path", "categories"]
        //if (exceptions.indexOf(key) === -1 && !key.includes("Changed"))
        for (const key in this)
            if (exceptions.indexOf(key) === -1 && typeof this[key] !== "function")
                keys.push(key)
        saved()
        return keys
    }
    function save() {
        let json = App.readJSON(path) ?? {}
        let jsObject = json
        for (let category of categories) {
            if (!jsObject[category]) jsObject[category] = {}
            jsObject = jsObject[category]
        }
        for (let property of properties()) jsObject[property] = this[property]
        App.saveFile(JSON.stringify(json, null, 4), path)
    }
    function load() {
        const json = App.readJSON(path)
        if (!json) return
        let jsObject = json
        for (let category of categories) {
            if (!jsObject[category]) jsObject[category] = {}
            jsObject = jsObject[category]
        }
        for (let property of properties()) if (jsObject[property]) this[property] = jsObject[property]
        loaded()
    }
    Component.onCompleted: load()
    Component.onDestruction: save()
}
