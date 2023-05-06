import QtQuick
import QtQuick.Controls 6.3
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 6.3
import QtCharts 2.5
Rectangle {
    id: windowPage2
    width: 1280
    height: 641
    


    Image {
        id: image
        x: 0
        y: 0
        width: 1280
        height: 641
        //        width: parent.height
        //        height: parent.width
        source: "page2_bg.jpg"
        enabled: true
        fillMode: Image.PreserveAspectCrop

    }
    DialControl {
        id: tempGauge
        x: 79
        y: 333
    }
    Slider {
        id: slider
        x: 28
        y: 185
        rotation: -90
        value: 0.5

        Text {
            id: text1
            y: 12
            text: qsTr("Set Temperature")
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            font.pixelSize: 18
            anchors.leftMargin: 150
            anchors.bottomMargin: 12
            rotation: 90
        }
    }

    ChartView {
        id: chartView
        x: 469
        y: 216
        width: 803
        height: 417
        LineSeries {
            id: tempSeries
            name: "Inside Temperature"
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
