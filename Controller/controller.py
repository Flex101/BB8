import serial
import pygame
import pprint
import threading
import struct
import time
import os


class GameController(object):

	controller = None
	axis_data = None
	button_data = None
	hat_data = None

	def init(self):
		"""Initialize the joystick components"""

		pygame.init()
		pygame.joystick.init()
		self.controller = pygame.joystick.Joystick(0)
		self.controller.init()

		self.button_data = [False] * self.controller.get_numbuttons()
		self.hat_data = [(0, 0)] * self.controller.get_numhats()
		self.axis_data = [0.0] * self.controller.get_numaxes()

	def setAxisCenter(self, axisIndex:int, center:float):
		self.axis_data[axisIndex] = center

	def update(self):
		for event in pygame.event.get():
			if event.type == pygame.JOYAXISMOTION:
				self.axis_data[event.axis] = event.value
			elif event.type == pygame.JOYBUTTONDOWN:
				self.button_data[event.button] = True
			elif event.type == pygame.JOYBUTTONUP:
				self.button_data[event.button] = False
			elif event.type == pygame.JOYHATMOTION:
				self.hat_data[event.hat] = event.value


def filterAxis(value:float, center:float, deadzone:float, invert:bool):
	
	if (value > center): value = (value-center)/(1-center)
	else: value = (value-center)/(1+center)
	
	if (abs(value) < deadzone): value = 0.0
	if (value > 1): value = 1.0
	if (value < -1): value = -1.0
	
	if (invert): return -value
	else: return value


def buildPacket(controller:GameController):

	roll_demand = filterAxis(controller.axis_data[1], 0.2, 0.1, True)

	print(roll_demand)

	packet = bytearray()
	packet += struct.pack("f", roll_demand)
	
	return packet


controller = GameController()
controller.init()
controller.setAxisCenter(1, 0.2)

serialPort = serial.Serial('/dev/rfcomm0')

while (True):
	controller.update()
	packet = buildPacket(controller)
	serialPort.write(packet)
	time.sleep(0.01)
