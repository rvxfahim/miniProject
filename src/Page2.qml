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
    Button {
        id: roundButton
        x: 205
        y: 55
        width: 100
        height: 100
        text: "+"
        checkable: true
        background: Image {
            id: btnImageA
            source: roundButton.checked ? "led_green.png" : "led_red.png"
        }
        Text {
            id: text1
            x: -30
            y: -38
            width: 160
            height: 28
            text: qsTr("Compressor A")
            font.pixelSize: 28
            verticalAlignment: Text.AlignVCenter
        }
    }

    Button {
        id: roundButton1
        x: 557
        y: 55
        width: 100
        height: 100
        text: "+"
        checkable: true
        background: Image {
            id: btnImageB
            source: roundButton1.checked ? "led_green.png" : "led_red.png"
        }
        Text {
            id: text2
            x: -36
            y: -43
            text: qsTr("Compressor B")
            font.pixelSize: text1.font.pixelSize
        }
    }

    Button {
        id: roundButton2
        x: 920
        y: 55
        width: 100
        height: 100
        text: "+"
        onClicked:
        {
            backend.startController()
        }
        Text {
            id: text3
            x: 18
            y: -41
            width: 64
            height: 38
            text: qsTr("Start")
            font.pixelSize: text2.font.pixelSize
        }
    }

    Connections {
        target: backend
        function onTextReceived(in_temp, currTime){
            //            textArea.append(qsTr(in_temp) +" "+ qsTr(currTime))
            //            if(textArea.lineCount > 10)
            //            {
            //                textArea.clear();
            //            }
        }
        function onTempValue(in_temp, currTime) {
            //            console.log("got values " + currTime + " ; " + in_temp)
            tempSeries.append(currTime, in_temp)
            // update the x axis range if necessary
            if (currTime > axisX.max) {
                axisX.max = currTime
                axisX.min = currTime - 49
            }
        }
        function onComA(val){
            if(val===1)
            {
                roundButton.checked = true
            }
            else
                roundButton.checked = false
        }
        function onComB(val){
            if(val===1)
            {
                roundButton1.checked = true
                console.log("com b set")
            }
            else
            {
                roundButton1.checked = false
                console.log("com b unset")
            }
        }
    }

    Button {
        id: roundButton3
        x: 1129
        y: 55
        width: 100
        height: 100
        text: "+"
        onClicked:
        {
            backend.stopController()
        }
        Text {
            x: 18
            y: -41
            width: 64
            height: 38
            text: qsTr("Stop")
            font.pixelSize: text2.font.pixelSize
        }
    }








}
