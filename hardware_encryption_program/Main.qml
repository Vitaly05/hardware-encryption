import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import backend

ApplicationWindow {
    id: window
    width: 1040
    height: 480
    minimumWidth: 500
    minimumHeight: 300
    visible: true

    title: qsTr("Hardware encryption")

    Backend {
        id: backend
    }

    Connections {
        target: backend

        function onShowAlert(title, message) {
            msgDialog.title = title
            msgDialog.text = message
            msgDialog.open()
        }
    }

    menuBar: MenuBar {
        Menu {
            title: "File"
            MenuItem {
                text: "Open"
                onTriggered: fileOpenDialog.open()
            }

            MenuItem {
                text: "New"
                onTriggered: backend.createNewFile()
            }
        }
    }

    MessageDialog {
        id: msgDialog
        buttons: MessageDialog.Ok
    }

    FileDialog {
        id: fileOpenDialog
        title: "Select file"
        nameFilters: ["(*.lhef)"]
        onAccepted: {
            var path = fileOpenDialog.selectedFile;
            backend.openFile(path);
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 15

        ColumnLayout {
            GroupBox {
                title: "Select device"
                Layout.fillHeight: true
                Layout.preferredWidth: 250

                Label {
                    text: "Devices not found"
                    font.pointSize: 11
                    anchors.centerIn: parent

                    visible: backend.hidSerials.length === 0
                }

                ListView {
                    anchors.fill: parent
                    model: backend.hidSerials
                    clip: true

                    Component.onCompleted: currentIndex = -1

                    delegate: ItemDelegate {
                        width: parent.width
                        text: modelData
                        highlighted: ListView.isCurrentItem

                        onClicked: {
                            ListView.view.currentIndex = index;
                            backend.selectedHidSerial = modelData;
                        }
                    }
                }
            }

            Button {
                text: "Refresh devices"

                onClicked: backend.refreshDevices()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                TextArea {
                    id: richTextArea
                    placeholderText: "Enter text"
                    textFormat: TextArea.PlainText
                    wrapMode: TextArea.Wrap
                    selectByMouse: true

                    text: backend.fileContent

                    onTextChanged: {
                        if (backend.fileContent !== text) {
                            backend.fileContent = text;
                        }
                    }
                }
            }

            Button {
                text: "Encrypt"
                anchors.right: parent.right
                enabled: backend.selectedHidSerial !== ""

                onClicked: {
                    backend.encrytpAndSaveFile()
                }
            }
        }
    }

    footer: ToolBar {
        background: Rectangle {
            color: "transparent"

            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: "#171717"
            }
        }


        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            Label {
                text: "File: " + (backend.isNewFile ? "new file" : backend.currentFilePath)
                font.pointSize: 10
                Layout.fillWidth: true
            }
        }
    }
}
