/*
    SPDX-License-Identifier: MIT
    MSI CoreLiquid Mode Switcher for Plasma 6
*/

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.plasma5support as Plasma5Support

PlasmoidItem {
    id: root

    // Current active mode (default -1 until detected)
    property string activeMode: "-1"
    property int fanRadiatorSpeed: 0
    property int fanWaterBlockSpeed: 0
    property int pumpSpeed: 0
    property int waterTemp: 0

    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground

    preferredRepresentation: (plasmoid.location === PlasmaCore.Types.Floating)
                             ? fullComp
                             : compactComp

    compactRepresentation: compactComp
    fullRepresentation: fullComp

    function doUpdate() {
        executable.connectSource("systemctl list-units --state=active --no-legend 'my_msi_coreliquid_driver@*'")
        executable.connectSource("qdbus --system io.github.MSICoreliquid /io/github/MSICoreliquid org.freedesktop.DBus.Properties.GetAll io.github.MSICoreliquid")
    }

    function get_mode_name(mode_id) {
        switch (mode_id) {
            case "0": return "silent";
            case "1": return "balance";
            case "2": return "game";
            case "5": return "smart";
            default:  return "Unknown";
        }
    }

    onActiveModeChanged: {
        var mode_name = get_mode_name(root.activeMode);
        var icon_path = "../icons/fan-" + mode_name + ".png";

        plasmoid.icon = Qt.resolvedUrl(icon_path);
        root.toolTipSubText = "Current Mode: " + mode_name;
    }

    onExpandedChanged: {
        if (expanded) {
            console.log("MSI-DEBUG: Widget expanded, starting updates.");
            doUpdate();
        }
    }

    Plasma5Support.DataSource {
        id: executable
        engine: "executable"
        connectedSources: []

        onNewData: (sourceName, data) => {
            if (sourceName.indexOf("list-units") !== -1) {
                let match = data.stdout.match(/@(\d)/);
                root.activeMode = match ? match[1] : "-1";
                disconnectSource(sourceName);
            }
            else if (sourceName.indexOf("io.github.MSICoreliquid") !== -1) {
                let lines = data.stdout.split('\n');
                lines.forEach(line => {
                    let parts = line.split(':');
                    if (parts.length >= 2) {
                        let name = parts[0].trim();
                        let value = parseInt(parts[1].trim());

                        if (name === "FanRadiatorSpeed") root.fanRadiatorSpeed = value;
                        else if (name === "FanWaterBlockSpeed") root.fanWaterBlockSpeed = value;
                        else if (name === "PumpSpeed") root.pumpSpeed = value;
                        else if (name === "LiquidTemp") root.waterTemp = value;
                    }
                });

                disconnectSource(sourceName);
            }
        }
    }

    // Timer to refresh every 5 seconds
    Timer {
        interval: 5000; running: true; repeat: true; triggeredOnStart: true
        onTriggered: {
            if (root.expanded) {
                root.doUpdate();
            }
        }
    }

    function runCmd(mode) {
        let command = "systemctl stop my_msi_coreliquid_driver@* && systemctl start my_msi_coreliquid_driver@" + mode;
        executable.connectSource(command);
        // Immediate update for better UX
        root.activeMode = mode.toString()
        root.expanded = false;
    }

    Component {
        id: compactComp
        PlasmaComponents.ToolButton {
            icon.source: plasmoid.icon
            onClicked: root.expanded = !root.expanded
        }
    }

    Component {
        id: fullComp

        RowLayout {
            id: mainLayout
            spacing: 10

            Layout.preferredWidth: implicitWidth
            Layout.preferredHeight: implicitHeight

            ColumnLayout {
                Layout.alignment: Qt.AlignBottom

                PlasmaComponents.Label {
                    text: "Radiator fan speed:"
                    font.pixelSize: 10
                    opacity: 0.7
                }
                PlasmaComponents.Label {
                    text: root.fanRadiatorSpeed + " rpm"
                }

                PlasmaComponents.Label {
                    text: "Water block fan speed:"
                    font.pixelSize: 10
                    opacity: 0.7
                }
                PlasmaComponents.Label {
                    text: root.fanWaterBlockSpeed + " rpm"
                }

                PlasmaComponents.Label {
                    text: "Pump speed:"
                    font.pixelSize: 10
                    opacity: 0.7
                }
                PlasmaComponents.Label {
                    text: root.pumpSpeed + " rpm"
                }

                PlasmaComponents.Label {
                    text: "Water temperature:"
                    font.pixelSize: 10
                    opacity: 0.7
                }
                PlasmaComponents.Label {
                    text: root.waterTemp+" ℃"
                }
            }

            ColumnLayout {
                spacing: 5
                Layout.preferredWidth: 100

                PlasmaComponents.Label {
                    text: "Cooling Mode"
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 5
                }

                // Buttons with "checked" state based on activeMode
                PlasmaComponents.Button {
                    text: "Silent"
                    icon.source: Qt.resolvedUrl("../icons/silent.png")
                    Layout.fillWidth: true
                    checkable: true
                    checked: root.activeMode === "0"
                    onClicked: root.runCmd(0)
                }

                PlasmaComponents.Button {
                    text: "Balance"
                    icon.source: Qt.resolvedUrl("../icons/balance.png")
                    Layout.fillWidth: true
                    checkable: true
                    checked: root.activeMode === "1"
                    onClicked: root.runCmd(1)
                }

                PlasmaComponents.Button {
                    text: "Game"
                    icon.name: Qt.resolvedUrl("../icons/game.png")
                    Layout.fillWidth: true
                    checkable: true
                    checked: root.activeMode === "2"
                    onClicked: root.runCmd(2)
                }

                PlasmaComponents.Button {
                    text: "Smart"
                    icon.source: Qt.resolvedUrl("../icons/smart.png")
                    Layout.fillWidth: true
                    checkable: true
                    checked: root.activeMode === "5"
                    onClicked: root.runCmd(5)
                }
            }
        }
    }
}
