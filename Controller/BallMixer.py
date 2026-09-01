from GameController import GameController
import math

toDegrees = 180 / math.pi

class BallMixer:

	def __init__(self, ballController:GameController):
		self.ballController = ballController


	def getRoll(self) -> float:
		reverseSectorAngle = 90
		reverseSectorBlendAngle = 30

		angle = self.getAngle()
		mag = self.getMagnitude()

		blendStart = 180 - ((reverseSectorAngle / 2) + (reverseSectorBlendAngle / 2))
		blendEnd = 180 - ((reverseSectorAngle / 2) - (reverseSectorBlendAngle / 2))

		# In forward sector
		if (abs(angle) < blendStart):
			return mag
		
		# In reverse sector
		if (abs(angle) > blendEnd):
			return -mag
		
		# Blending between
		blend = (abs(angle) - blendStart) / ((blendEnd - blendStart) / 2)
		blend = (-blend) + 1
		return mag * blend
	

	def getTilt(self) -> float:
		x = self.ballController.getX()

		deadzone = 0.25

		# Dead zone
		if abs(x < deadzone):
			return 0

		if (x > 0):
			x = (x - deadzone) / (1 - deadzone)
		if (x < 0):
			x = (x + deadzone) / (1 - deadzone)

		return x


	def getAngle(self) -> float:
		x = self.ballController.getX()
		y = self.ballController.getY()

		angle = math.atan(x/y) * toDegrees
		if (y < 0):
			if (x > 0):
				angle = 180 + angle
			else:
				angle = -180 + angle

		return angle
	

	def getMagnitude(self) -> float:
		x = self.ballController.getX()
		y = self.ballController.getY()
		mag = math.sqrt((x * x) + (y * y))
		if (mag > 1): mag = 1
		return mag