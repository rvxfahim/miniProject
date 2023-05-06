import QtQuick
import QtQuick.Controls 6.3
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 6.3

Window {
    width: 1280
    height: 720
    visible: true
    title: qsTr("HVAC Controller")
    ToolBar {
        id: toolBar
        x: 0
        y: 0
        width: 1005
        height: 79

        TabButton {
            id: tabButton
            x: 0
            y: 0
            height: parent.height
            width: 200
            text: qsTr("Configuration")
            font.pointSize: 14
            focusPolicy: Qt.ClickFocus
            onClicked: {
                stackLayout.currentIndex = 0
            }
        }

        TabButton {
            id: nav_pg2
            y: 1
            height: parent.height
            width: 200
            text: qsTr("Control")
            font.pointSize: 14
            anchors.left: tabButton.right
            anchors.leftMargin: 0
            onClicked: {
                stackLayout.currentIndex = 1
            }
        }
    }
    StackLayout {
        id: stackLayout
        x: 0
        y: 79
        width: parent.width
        height: 641
        currentIndex: 0
        Page {
            id: page
            width: parent.width
            height: parent.height
            Image {
                id: image2
                width: parent.width
                height: parent.height
                source: "serial-port.jpg"
                fillMode: Image.PreserveAspectFit
            }

            GroupBox {
                id: groupBox
                x: 23
                y: 63
                width: 275
                height: 514
                font.pointSize: 24
                title: qsTr("Port Settings")

                Grid {
                    id: grid
                    width: parent.width
                    height: parent.height
                    spacing: 20
                    flow: Grid.LeftToRight
                    rows: 0
                    columns: 1
                    topPadding: 50


                    Text {
                        id: text1
                        y: 25
                        text: qsTr("Port")
                        font.pixelSize: 24
                    }
                    ComboBox {
                        id: comboBoxPorts
                        width: parent.width
                        height: 61
                        currentIndex: 0
                        model: [""]
                    }

                    Text {
                        id: text2
                        text: qsTr("Baud Rate")
                        font.pixelSize: 24
                    }

                    ComboBox {
                        id: comboBoxBauds
                        width: parent.width
                        height: 61
                        model: ["9600", "115200"]
                    }

                    Button {
                        id: connectButton
                        text: qsTr("Connect")
                        width: parent.width
                        height: 61
                        onClicked: {
                            backend.connectSerial(comboBoxPorts.displayText, comboBoxBauds.displayText)
                            connectButton.enabled = false
                        }
                    }

                    Button {
                        id: disconnectButton
                        text: qsTr("disconnect")
                        width: parent.width
                        height: 61
                        onClicked: {
                            backend.disconnectSerial()
                            connectButton.enabled = true
                        }
                    }



                }
            }
        }

        Page2{
            id: page2
        }
    }
    Connections{
        target: backend
        function onListofserialportsReceived(ports){
                console.log("list of serial ports:" + ports)
                comboBoxPorts.model = ports
            }
        onListofserialportsReceived: {
                onListofserialportsReceived(arguments[0])
            }
//        onAppLoad: {
//            getPorts()
//        }
//        function getPorts(){
//            backend.getPorts()
//        }
    }
    Component.onCompleted: {
            backend.getPorts()
        }
}
