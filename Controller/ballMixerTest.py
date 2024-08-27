from BallMixer import BallMixer
from GameController import GameController
import pygame

pygame.init()
pygame.joystick.init()

controller = GameController()
controller.connect(0)

mixer = BallMixer(controller)

while (True):
	pygame.event.get()
	controller.update()

	angle = mixer.getAngle()
	mag = mixer.getMagnitude()

	roll = mixer.getRoll()
	tilt = mixer.getTilt()
	print("{:.3f}".format(roll), "{:.3f}".format(tilt))