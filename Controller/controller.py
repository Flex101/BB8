from GameController import GameController
from BallMixer import BallMixer
import serial
import pygame
import struct
import time
import signal
import os

RED = "\033[31m"
GREEN = "\033[32m"
ORANGE = "\033[33m"
YELLOW = "\033[93m"
BLUE = "\033[34m"
CYAN = "\033[36m"
RESET = "\033[0m"
PRINT = False   # Enabling this will reduce performance

terminate = False
serialPortRoll = None
serialPortTilt = None

class RcxBoards:
	roll = '/dev/rfcomm0'
	tilt = '/dev/rfcomm1'

class ControllerNames:
	ball = "PLAYSTATION(R)3 Controller (04:76:6E:99:8D:EF)"
	dome = "PLAYSTATION(R)3 Controller (00:26:43:C9:DD:9E)"

def clean_term(signum, frame):
	global terminate
	terminate = True

# Register the custom signal handler for SIGINT (Ctrl+C)
signal.signal(signal.SIGINT, clean_term)

def abort():
	print(f"")
	print(f"{RED}Program terminated.{RESET}")
	print(f"---------------------------------")
	print(f"")
	exit()

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

try:
	os.system('clear')
	print(f"{ORANGE}BB-8 CR-X Drive System Controller{RESET}")
	print(f"{ORANGE}---------------------------------{RESET}")
	print(f"")
	print(f"Looking for controllers...")

	pygame.init()
	pygame.joystick.init()
	controllerCount = pygame.joystick.get_count()
	print(f"Controllers found: {controllerCount}")

	controllers = []
	for i in range(pygame.joystick.get_count()):
		controller = GameController()
		controller.connect(i)
		controllers.append(controller)

	ballController:GameController|None = None
	domeController:GameController|None = None

	for controller in controllers:
		match controller.controller.get_name():
			case ControllerNames.ball:
				ballController = controller
				continue
			case ControllerNames.dome:
				domeController = controller
				continue

	if (ballController == None):
		print(f"{YELLOW}Couldn't find ball controller.{RESET}")
	if (domeController == None):
		print(f"{YELLOW}Couldn't find dome controller.{RESET}")
	if (ballController == None or domeController == None):
		abort()
	print(f"{GREEN}Found ball and dome controllers.{RESET}")
	print(f"")

	print(f"Looking for RC-X board bound serial ports...")
	gotRollBoard = os.path.exists(RcxBoards.roll)
	gotTiltBoard = os.path.exists(RcxBoards.tilt)
	if (gotRollBoard == False):
		print(f"{YELLOW}Couldn't find roll board bound port.{RESET}")
	if (gotTiltBoard == False):
		print(f"{YELLOW}Couldn't find tilt board bound port.{RESET}")
	if (gotRollBoard == False or gotTiltBoard == False):
		abort()
	print(f"{GREEN}Found roll and tilt board bound ports.{RESET}")
	print(f"")

	mixer = BallMixer(ballController)

	serialPortRoll = serial.Serial('/dev/rfcomm0', timeout=1)
	serialPortTilt = serial.Serial('/dev/rfcomm1', timeout=1)

	print("Connecting...")

	if not serialPortRoll.cts and not serialPortRoll.dsr:
		print(f"{YELLOW}Unable to open connection to roll board!{RESET}")
		abort()

	if not serialPortTilt.cts and not serialPortTilt.dsr:
		print(f"{YELLOW}Unable to open connection to tilt board!{RESET}")
		abort()

	print(f"{GREEN}Connected to roll and tilt boards.{RESET}")
	print(f"")

	print("Running...")

	while not terminate:
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

		# Use shoulder button as roll enable
		if (ballController.getShoulderButton() == False):
			roll = 0.0

		packet = buildPacket(roll)
		serialPortRoll.write(packet)
		packet = buildPacket(tilt)
		serialPortTilt.write(packet)
		time.sleep(0.01)

	print("Terminated cleanly")

finally:

	print("Closing serial ports...")
	if (serialPortRoll != None):
		serialPortRoll.close()
	if (serialPortTilt != None):
		serialPortTilt.close()
	print("Exiting...")

