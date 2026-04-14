import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: page

    property alias cfg_updateInterval: updateIntervalField.value
    property alias cfg_showSensors: showSensorsCheck.checked

    Item {
        Kirigami.FormData.isSection: false
        implicitHeight: Kirigami.Units.gridUnit
    }

    // Setting for the update interval
    SpinBox {
        id: updateIntervalField
        Kirigami.FormData.label: "Update interval (ms):"
        from: 1000
        to: 60000
        stepSize: 500
    }

    // Setting for sensor visibility
    CheckBox {
        id: showSensorsCheck
        Kirigami.FormData.label: "Sensors:"
        text: "Display temperature and RPM"
    }
}
