import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root

    // ── Palette (aligned with qr_ui / stash) ──────────────────────────────
    readonly property color bgPrimary:     "#171717"
    readonly property color bgSecondary:   "#262626"
    readonly property color textPrimary:   "#FFFFFF"
    readonly property color textSecondary: "#A4A4A4"
    readonly property color textMuted:     "#5D5D5D"
    readonly property color accentOrange:  "#FF5000"
    readonly property color errorRed:      "#FB3748"
    readonly property color successGreen:  "#22C55E"
    readonly property color borderColor:   "#383838"

    property string statusMsg:   ""
    property bool   statusIsOk:  false

    // logos.callModule returns a double-JSON-encoded string; unwrap to a boolean.
    function parseBool(raw) {
        try {
            var v = JSON.parse(raw)
            if (typeof v === "string") v = JSON.parse(v)
            return v === true || v === "true"
        } catch (e) { return false }
    }

    // Store the typed Chat ID in the chat_id_showcase core module, then ping it so
    // any already-open QR Generator receives it immediately (the QR Generator also
    // pulls it on open, so this works whether or not it's currently visible).
    function publishChatId() {
        statusMsg = ""
        var id = input.text
        if (!id || id.length === 0) {
            statusIsOk = false
            statusMsg = "Enter a Chat ID first."
            return
        }
        if (typeof logos === "undefined") {
            statusIsOk = false
            statusMsg = "Module bridge unavailable (standalone preview)."
            return
        }
        var stored = parseBool(logos.callModule("chat_id_showcase", "setIntroBundle", [id]))
        if (!stored) {
            statusIsOk = false
            statusMsg = "Couldn't reach chat_id_showcase module. Is it installed?"
            return
        }
        logos.callModule("chat_id_showcase", "createIntroBundle", [])  // push to any open QR Generator
        statusIsOk = true
        statusMsg = "Chat ID published. Open the QR Generator to see the QR."
    }

    Rectangle {
        anchors.fill: parent
        color: bgPrimary

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 64, 440)
            spacing: 20

            Label {
                text: "Chat ID Showcase"
                color: textPrimary
                font.pixelSize: 22
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: "Enter a Chat ID (intro bundle) and publish it. The QR Generator "
                    + "reads it over the same IPC API the real chat module uses."
                color: textSecondary
                font.pixelSize: 13
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            TextField {
                id: input
                Layout.fillWidth: true
                placeholderText: "Enter a Chat ID…"
                color: textPrimary
                placeholderTextColor: textMuted
                font.pixelSize: 15
                padding: 12
                selectByMouse: true
                background: Rectangle {
                    color: bgSecondary
                    radius: 8
                    border.color: input.activeFocus ? accentOrange : borderColor
                    border.width: 1
                }
                onAccepted: root.publishChatId()
            }

            Button {
                id: genButton
                text: "Generate"
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                onClicked: root.publishChatId()
                contentItem: Text {
                    text: genButton.text
                    color: textPrimary
                    font.pixelSize: 15
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    radius: 8
                    color: genButton.pressed ? "#CC4000"
                         : genButton.hovered ? "#FF6420" : accentOrange
                }
            }

            Label {
                text: statusMsg
                visible: statusMsg.length > 0
                color: statusIsOk ? successGreen : errorRed
                font.pixelSize: 12
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
        }
    }
}
