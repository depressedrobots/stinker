# pip install pyserial, no serial!
import serial

def getValueString(string):
    valueStart = string.find(": ")
    if valueStart == -1:
        return ""

    return string[valueStart+1:-2]    # substring from after ": ", leave out last to characters

ser = serial.Serial('/dev/ttyUSB0', 9600)
while True :
    try:
        line = ser.readline()
        string = line.decode('utf-8')
        if "ccs811CO2" in string:
            print("CO2: " + getValueString(string))
    except Exception as ex:
        print(ex)

