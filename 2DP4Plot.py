import matplotlib.pyplot as plt
import matplotlib.animation as animation
import serial
ser = serial.Serial("COM2", 19200, timeout=1) ## open COM port 2, baudrate set to 19200, timeout when using readline() = 1

display = plt.figure()
graph = display.add_subplot(1,1,1)
xAxis = [] ## store time here
yAxis = [] ## store accelerometer angles here

def livePlot(i):
    angle = ser.readline()
    x = int(angle[10:])

    xAxis.append(x)
    yAxis.append(len(xAxis))

    graph.clear()
    graph.plot(yAxis, xAxis)
    graph.set_title("ADXL337 Acclerometer Angle with respect to Time")
    graph.set_xlabel("Time")
    graph.set_ylabel("Angle (degrees)")

liveDisplay = animation.FuncAnimation(display, livePlot, interval = 500)
plt.show()
