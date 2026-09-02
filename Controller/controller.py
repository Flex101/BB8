from GameController import GameController
from BallMixer import BallMixer
from enum import Enum
import serial
import pygame
import struct
import time
import signal
import subprocess
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

class PortState(Enum):
	UNKNOWN = 0
	CONFIGURING = 1
	CONNECTED = 2
	CLOSED = 3

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

def getPortState(port):
	index = port.replace("/dev/rfcomm","")
	output = subprocess.check_output(["rfcomm", "show", index], stderr=subprocess.STDOUT).decode('utf-8')
	if "config" in output:
		return PortState.CONFIGURING
	if "connected" in output:
		return PortState.CONNECTED
	if "closed" in output:
		return PortState.CLOSED
	return PortState.UNKNOWN

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

	serialPortRoll = serial.Serial(RcxBoards.roll, timeout=1)
	serialPortTilt = serial.Serial(RcxBoards.tilt, timeout=1)

	rollPortState = PortState.UNKNOWN
	tiltPortState = PortState.UNKNOWN

	print("Connecting to RC-X boards...", end="", flush=True)

	while (rollPortState != PortState.CONNECTED) or (tiltPortState != PortState.CONNECTED):
		rollPortState = getPortState(RcxBoards.roll)
		tiltPortState = getPortState(RcxBoards.tilt)

		if (rollPortState == PortState.CLOSED):
			print("")
			print(f"{YELLOW}Failed to connect to roll board.{RESET}")
			abort()

		if (tiltPortState == PortState.CLOSED):
			print("")
			print(f"{YELLOW}Failed to connect to tilt board.{RESET}")
			abort()

		print(".", end="", flush=True)
		time.sleep(0.5)

	print("")
	print(f"{GREEN}Successfully connected to roll and tilt boards.{RESET}")
	print("")

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


		# Use shoulder trigger as roll and tilt enable
		if (ballController.getShoulderTrigger() == True):
			roll = mixer.getRoll()
			tilt = mixer.getTilt()

		# Use shoulder button as tilt enable
		elif (ballController.getShoulderButton() == True):
			tilt = mixer.getTilt()

		else:
			roll = 0.0
			tilt = 0.0
			

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

