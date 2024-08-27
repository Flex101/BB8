import serial
import pygame
import pprint
import threading
import struct
import time
import os

PRINT = False   # Enabling this will reduce performance

class ControllerNames:
	ball = "PLAYSTATION(R)3 Controller (04:76:6E:99:8D:EF)"
	dome = "PLAYSTATION(R)3 Controller (00:26:43:C9:DD:9E)"

class GameController(object):

	def __init__(self):
		self.controller = None
		self.axis_data = None
		self.button_data = None
		self.hat_data = None

	def connect(self, index:int=0):
		"""Initialize the joystick components"""
		self.controller = pygame.joystick.Joystick(index)
		self.controller.init()
		self.button_data = [False] * self.controller.get_numbuttons()
		self.hat_data = [(0, 0)] * self.controller.get_numhats()
		self.axis_data = [0.0] * 2 #self.controller.get_numaxes()

	def setAxisCenter(self, axisIndex:int, center:float):
		self.axis_data[axisIndex] = center

	def update(self):
		for i in range(len(self.axis_data)):
			self.axis_data[i] = self.controller.get_axis(i)
		for i in range(len(self.button_data)):
			self.button_data[i] = self.controller.get_button(i)

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


def buildPacket(controller:GameController, axis:int):

	demand = filterAxis(controller.axis_data[axis], 0.2, 0.2, True)

	packet = bytearray()
	packet += struct.pack("f", demand)
	
	return packet


pygame.init()
pygame.joystick.init()
controllerCount = pygame.joystick.get_count()
print(f"Controllers found: {controllerCount}")

controllers = []
for i in range(pygame.joystick.get_count()):
	controller = GameController()
	controller.connect(i)
	controller.setAxisCenter(1, 0.2)
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

	packet = buildPacket(ballController, 1)
	serialPortRoll.write(packet)
	packet = buildPacket(ballController, 0)
	serialPortTilt.write(packet)
	time.sleep(0.01)
