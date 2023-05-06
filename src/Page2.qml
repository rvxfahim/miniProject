import QtQuick
import QtQuick.Controls 6.3
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 6.3
import QtCharts 2.5
Rectangle {
    id: windowPage2
    width: 1005
    height: 641
    property alias textArea: textArea
    
    Image {
        id: image
        //        width: parent.height
        //        height: parent.width
        source: "page2_bg.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    GroupBox {
        id: groupBox
        x: 0
        y: 0
        width: 400
        height: 641
        title: qsTr("")

        Grid {
            id: grid
            width: parent.width
            height: 400
            padding: 50
            spacing: 20
            rows: 2
            columns: 2


            Rectangle {
                id: rectangle
                width: parent.width
                height: 230
                color: "#ffffff"

                TextEdit {
                    id: textEdit
                    width: rectangle.width
                    height: rectangle.height
                    text: qsTr("")
                    font.pixelSize: 12
                }
            }

            Button {
                id: button
                text: qsTr("Send")
                onClicked: {
                    tempSeries.remove(0)
                    tempSeries.append(5,2)
                    backend.sendBtn()
                }
            }

            Rectangle {
                id: rectangle1
                width: parent.width
                height: 230
                color: "#ffffff"

                TextArea {
                    id: textArea
                    width: rectangle1.width
                    height: rectangle1.height
                    enabled: false
                    placeholderText: qsTr("Received From Port")
                }
            }

            ChartView {
                id: chartView
                width: 300
                height: 300
                LineSeries {
                         id: tempSeries
                         name: "SplineSeries"
                         axisX: axisX
                         axisY: yAxis

                     }
                ValueAxis {
                            id: axisX
                            min: 0
                            max: 50 // set the initial max value to 50
                            tickCount: 10
//                            labelFormat: "%.0f"
                        }

                        ValueAxis {
                            id: yAxis
                            min: 0
                            max: 50
                            tickCount: 5
//                            labelFormat: "%.0f"
                        }
            }
        }
    }
    Connections {
        target: backend
        function onTextReceived(in_temp, currTime){
            textArea.append(qsTr(in_temp) +" "+ qsTr(currTime))
            if(textArea.lineCount > 10)
            {
                textArea.clear();
            }
        }
        function onTempValue(in_temp, currTime) {
            console.log("got values " + currTime + " ; " + in_temp)
            tempSeries.append(currTime, in_temp)
            // update the x axis range if necessary
            if (currTime > axisX.max) {
                axisX.max = currTime
                axisX.min = currTime - 49
            }
        }
    }


}
