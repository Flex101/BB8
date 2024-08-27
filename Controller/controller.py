from GameController import GameController
from BallMixer import BallMixer
import serial
import pygame
import struct
import time

PRINT = False   # Enabling this will reduce performance

class ControllerNames:
	ball = "PLAYSTATION(R)3 Controller (04:76:6E:99:8D:EF)"
	dome = "PLAYSTATION(R)3 Controller (00:26:43:C9:DD:9E)"


def filterAxis(value:float, center:float, deadzone:float, invert:bool):
	
	if (value > center): value = (value-center)/(1-center)
	else: value = (value-center)/(1+center)
	
	if (abs(value) < deadzone): value = 0.0
	else:
		if (value > 0):
			value = (value - deadzone) / (1.0 - deadzone)
		else:
			value = -((value + deadzone) / (-1.0 + deadzone))

	if (value > 1): value = 1.0
	if (value < -1): value = -1.0
	
	if (invert): return -value
	else: return value


def buildPacket(value:float):
	packet = bytearray()
	packet += struct.pack("f", value)
	return packet


pygame.init()
pygame.joystick.init()
controllerCount = pygame.joystick.get_count()
print(f"Controllers found: {controllerCount}")

controllers = []
for i in range(pygame.joystick.get_count()):
	controller = GameController()
	controller.connect(i)
	controllers.append(controller)

ballController = None
domeController = None

for controller in controllers:
	match controller.controller.get_name():
		case ControllerNames.ball:
			ballController = controller
			continue
		case ControllerNames.dome:
			domeController = controller
			continue

if (ballController == None):
	print("Couldn't find ball controller")
if (domeController == None):
	print("Couldn't find dome controller")
if (ballController == None or domeController == None):
	exit()
print("Found ball and dome controllers")

mixer = BallMixer(ballController)

serialPortRoll = serial.Serial('/dev/rfcomm0')
serialPortTilt = serial.Serial('/dev/rfcomm1')

while (True):
	pygame.event.get()
	ballController.update()
	domeController.update()

	if PRINT:
		print("Ball:", end=' ')
		for i in range(len(ballController.axis_data)):
			print("{:.3f}".format(ballController.axis_data[i]).rjust(6), end=' ')
		for i in range(len(ballController.button_data)):
			print(ballController.button_data[i], end=' ')
		print()

		print("Dome:", end=' ')
		for i in range(len(domeController.axis_data)):
			print("{:.3f}".format(domeController.axis_data[i]).rjust(6), end=' ')
		for i in range(len(domeController.button_data)):
			print(domeController.button_data[i], end=' ')
		print()

	roll = mixer.getRoll()
	tilt = mixer.getTilt()

	packet = buildPacket(roll)
	serialPortRoll.write(packet)
	packet = buildPacket(tilt)
	serialPortTilt.write(packet)
	time.sleep(0.01)
