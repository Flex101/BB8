import pygame

class GameController:

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

	def update(self):
		for i in range(len(self.axis_data)):
			self.axis_data[i] = self.controller.get_axis(i)
		for i in range(len(self.button_data)):
			self.button_data[i] = self.controller.get_button(i)