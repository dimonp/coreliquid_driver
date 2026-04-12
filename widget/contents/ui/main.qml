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

    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground

    preferredRepresentation: (plasmoid.location === PlasmaCore.Types.Floating)
                             ? fullComp
                             : compactComp

    compactRepresentation: compactComp
    fullRepresentation: fullComp

    Plasma5Support.DataSource {
        id: executable
        engine: "executable"
        connectedSources: []
        onNewData: (sourceName, data) => {
            if (sourceName.indexOf("list-units") !== -1) {
                // Parse mode number from unit name like ...@5.service
                let match = data.stdout.match(/@(\d)/);
                activeMode = match ? match[1] : "-1";
            }
            disconnectSource(sourceName);
        }
    }

    // Timer to refresh the active mode indicator every 5 seconds
    Timer {
        interval: 5000; running: true; repeat: true; triggeredOnStart: true
        onTriggered: executable.connectSource("systemctl list-units --state=active --no-legend 'my_msi_coreliquid_driver@*'")
    }

    function runCmd(mode) {
        let command = "systemctl stop my_msi_coreliquid_driver@* && systemctl start my_msi_coreliquid_driver@" + mode;
        executable.connectSource(command);
        root.expanded = false;
        // Immediate update for better UX
        activeMode = mode.toString();
    }

    Component {
        id: compactComp
        PlasmaComponents.ToolButton {
            icon.name: Qt.resolvedUrl("../icons/fan.png")
            onClicked: root.expanded = !root.expanded
        }
    }

    Component {
        id: fullComp
        Item {
            Layout.minimumWidth: 100
            Layout.preferredWidth: 100
            Layout.maximumWidth: 100
            Layout.minimumHeight: mainLayout.implicitHeight
            Layout.preferredHeight: mainLayout.implicitHeight

            ColumnLayout {
                id: mainLayout
                anchors.centerIn: parent
                width: parent.width
                spacing: 8
                Layout.margins: 10

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
