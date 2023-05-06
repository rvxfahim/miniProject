# This Python file uses the following encoding: utf-8

import sys

from pathlib import Path
import serial
# from PySide6.QtGui import QGuiApplication
from PySide6.QtWidgets import QApplication
from PySide6.QtQml import QQmlApplicationEngine
from PySide6.QtCore import Qt, QObject, Signal, Slot, Property, QThread
from PySide6 import QtCharts
import time
import json
def serial_ports():
    """ Lists serial port names
        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port, baudrate = 115200, timeout = 0)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

#thread for reading serial data
class SerialThread(QThread):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.ser = serial.Serial()

    def run(self):
        while True:
            if main.connectionEstablished:
                try:
                    jsonString = self.ser.readline().decode('utf-8')
                    # try to load json string, may not be valid json
                    try:
                        jsonData = json.loads(jsonString)
                        main.textReceived.emit(jsonData['inT'],jsonData['tm'])
                        main.tempValue.emit(jsonData['inT'],jsonData['tm'])
                    except:
                        print("failed to load json string")
                except:
                    print("failed to read serial data")
                    #try to stop serial thread
                    try:
                        self.terminate()
                    except:
                        print("thread probably not running")
            else:
                print("connection not established")
                #try to stop serial thread
                try:
                    self.terminate()
                except:
                    print("thread probably not running")
class MainWindow(QObject):
    
    def __init__(self):
        QObject.__init__(self)
        self.serialThread = SerialThread(self)
    listofserialportsReceived = Signal(list)
    textReceived = Signal(float, float)
    tempValue = Signal(float, float)
    appLoad = Signal()
    connectionEstablished = False
    ser = None
    #getPorts called after app is loaded
    @Slot()
    def getPorts(self):
        ports = serial_ports()
        print(ports)
        self.listofserialportsReceived.emit(ports)
        print("signal emitted & getPorts called from QML" + str(ports))
    #called when user press Send button on Page 2
    @Slot()
    def sendBtn(self):
        print("sendbtn called from QML")
    @Slot(str, str)
    def connectSerial(self, port, baud):
        #try to connect to serial port
        try:
            self.ser = serial.Serial(port, baud)
            self.ser.close()
            self.ser.open()
            print("connected to serial port")
            #clear serial buffer
            self.ser.reset_input_buffer()
            self.connectionEstablished = True
            self.serialThread.ser = self.ser
            self.serialThread.start()
        except:
            print("failed to connect to serial port")
            self.connectionEstablished = False
            #try to stop serial thread
            try:
                self.serialThread.terminate()
            except:
                print("thread probably not running")
    @Slot()
    def disconnectSerial(self):
        print("disconnectSerial called from QML")
        #try to stop serial thread
        try:
            self.serialThread.terminate()
        except:
            print("thread probably not running")
        #try to close serial port
        try:
            self.ser.close()
            print("serial port closed")
        except:
            print("failed to close serial port")
        self.connectionEstablished = False            
if __name__ == "__main__":

    # app = QGuiApplication(sys.argv)
    app = QApplication(sys.argv)
    engine = QQmlApplicationEngine()
    main = MainWindow()
    qml_file = Path(__file__).resolve().parent / "main.qml"
    # need to add backend context to the engine
    engine.rootContext().setContextProperty("backend", main)
    engine.load(qml_file)
    # main.appLoad.connect(main.getPorts)
    main.appLoad.emit()
    if not engine.rootObjects():

        sys.exit(-1)

    sys.exit(app.exec())

