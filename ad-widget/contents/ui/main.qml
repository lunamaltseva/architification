import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PC3
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    preferredRepresentation: fullRepresentation

    implicitWidth: 300
    implicitHeight: 260

    readonly property string effectiveSource:
        plasmoid.configuration.imagePath.length > 0
            ? plasmoid.configuration.imagePath
            : Qt.resolvedUrl("../../horrible_ad.png")

    readonly property string effectiveUrl:
        plasmoid.configuration.clickUrl.length > 0
            ? plasmoid.configuration.clickUrl
            : "application://arch-info-center.desktop"

    fullRepresentation: Rectangle {
        id: container
        color: "transparent"
        anchors.fill: parent

        // Ad image
        Image {
            id: adImage
            anchors.fill: parent
            anchors.bottomMargin: plasmoid.configuration.showLabel ? 20 : 0
            source: root.effectiveSource
            fillMode: Image.PreserveAspectCrop
            smooth: true
            asynchronous: true

            // Click to open URL
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Qt.openUrlExternally(root.effectiveUrl)
            }
        }

        // Placeholder when no image is set
        Rectangle {
            anchors.fill: adImage
            color: "#1a1a2e"
            visible: adImage.status !== Image.Ready

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 8

                Kirigami.Icon {
                    source: "image-x-generic"
                    Layout.alignment: Qt.AlignHCenter
                    implicitWidth: 48
                    implicitHeight: 48
                    opacity: 0.5
                }

                Label {
                    text: "📢 AD SPACE AVAILABLE"
                    color: "white"
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Right-click → Configure\nto set your ad image"
                    color: "#aaaaaa"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }

                Rectangle {
                    width: 140
                    height: 28
                    radius: 14
                    color: "#1793d1"
                    Layout.alignment: Qt.AlignHCenter

                    Label {
                        anchors.centerIn: parent
                        text: "CONFIGURE NOW →"
                        color: "white"
                        font.bold: true
                        font.pixelSize: 10
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: plasmoid.internalAction("configure").trigger()
                    }
                }
            }
        }

        // "Sponsored" label
        Rectangle {
            visible: plasmoid.configuration.showLabel
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 4
            width: sponsoredLabel.implicitWidth + 8
            height: 18
            radius: 3
            color: "#cc000000"

            Label {
                id: sponsoredLabel
                anchors.centerIn: parent
                text: "Sponsored"
                color: "#cccccc"
                font.pixelSize: 9
            }
        }

        // "✕" close hint (purely cosmetic — you can't actually close it)
        Rectangle {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 4
            width: 16
            height: 16
            radius: 8
            color: "#99000000"
            visible: adImage.status === Image.Ready

            Label {
                anchors.centerIn: parent
                text: "✕"
                color: "white"
                font.pixelSize: 9
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    // "Close" just refreshes the ad
                    adImage.source = ""
                    adImage.source = root.effectiveSource
                }
            }
        }
    }
}
