# This Python file uses the following encoding: utf-8

import sys

from pathlib import Path
import serial
# from PySide6.QtGui import QGuiApplication
from PySide6.QtWidgets import QApplication
from PySide6.QtQml import QQmlApplicationEngine
from PySide6.QtCore import Qt, QObject, Signal, Slot, Property, QThread
from PySide6 import QtCharts
from PySide6.QtCore import QTimer
from numba import jit
import time
import json
timeout = False
def ackTimeout(self):
        global timeout
        timeout = True
        print("ack timeout")
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
        print("serial thread started")
        while True:
            if main.connectionEstablished:
                try:
                    jsonString = self.ser.readline().decode('utf-8')
                    # try to load json string, may not be valid json
                    try:
                        jsonData = json.loads(jsonString)
                        #if json has inT and tm keys, emit signal
                        if 'inT' in jsonData and 'tm' in jsonData:
                            main.textReceived.emit(jsonData['inT'],jsonData['tm'])
                            main.tempValue.emit(jsonData['inT'],jsonData['tm'])
                        elif 'comA' in jsonData:
                            main.comA.emit(jsonData['comA'])
                        elif 'comB' in jsonData:
                            main.comB.emit(jsonData['comB'])
                        elif 'ack' in jsonData:
                            main.ack = jsonData['ack']
                            print("ack received")
                        else:
                            print(jsonData)
                        # print("got some string")
                    except:
                        # print("failed to load json string ")
                        pass
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
            
    
class SerialsendThread(QThread):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.ser = serial.Serial()
    
    def run(self):
        print("serialsend thread running")
        global timeout
        while True:
            self.jitSend()
            QThread.msleep(25)
    # @jit(nopython=True)
    def jitSend(self):
        start_time = time.time()
        if main.connectionEstablished:
            #keep a backup of the 1st item in queue
            backup = main.queue[0]
            self.sendSerial()
            # if main.ack is not 1 or timeout is True, stay in while loop
            print("waiting for ack or timeout in while loop")
            while main.ack != 1:
                difference = time.time()-start_time
                print(difference)
                if difference > 1:
                    #if timeout, restore backup in queue
                    main.queue.insert(0, backup)
                    break
                # print(time.time()-start_time)
            start_time = time.time()
            print("ack reset to None")
            main.ack = None
        else:
            print("connection not established")
            #try to stop serial thread
            try:
                self.terminate()
            except:
                print("thread probably not running")
    def sendSerial(self):
        print("sendSerial called")
        if len(main.queue) > 0:
            try:
                self.ser.write(main.queue.pop(0).encode('utf-8'))
                #send crlf
                self.ser.write(b'\r')
                print("json string sent to serial port by sendSerial")
            except:
                print("failed to send json string to serial port")
    

class MainWindow(QObject):
    
    def __init__(self):
        QObject.__init__(self)
        self.serialThread = SerialThread(self)
        self.serialsendThread = SerialsendThread(self)
    listofserialportsReceived = Signal(list)
    textReceived = Signal(float, float)
    tempValue = Signal(float, float)
    appLoad = Signal()
    comA = Signal(int)
    comB = Signal(int)
    connectionEstablished = False
    ser = None
    busBusy = False
    ack = None
    queue = []
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
            self.ser = serial.Serial(port, baud, timeout=5, write_timeout=0)
            self.ser.close()
            self.ser.open()
            print("connected to serial port")
            #clear serial buffer
            self.ser.reset_input_buffer()
            self.connectionEstablished = True
            self.serialThread.ser = self.ser
            self.serialsendThread.ser = self.ser
            self.serialThread.start()
            self.serialsendThread.start()
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
    @Slot(float)
    def setTemp(self, val):
        #format value to 2 decimal places
        print("setTemp called from QML")
        val = round(val, 2)
        #make a json string with temperature value
        jsonString = json.dumps({"setT": val})
        #add json string to queue, replace if queue has previous setT json string
        for i in range(len(self.queue)):
            if 'setT' in self.queue[i]:
                self.queue[i] = jsonString
                break
        self.queue.append(jsonString)
    @Slot()
    def startController(self):
        print("startController called from QML")
        jsonString = json.dumps({"start": 1})
        for i in range(len(self.queue)):
            if 'start' in self.queue[i]:
                self.queue[i] = jsonString
                break
        self.queue.append(jsonString)
    @Slot()
    def stopController(self):
        print("stopController called from QML")
        jsonString = json.dumps({"start": 0})
        for i in range(len(self.queue)):
            if 'stop' in self.queue[i]:
                self.queue[i] = jsonString
                break
        self.queue.append(jsonString)
        
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

