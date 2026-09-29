import serial
import time

PORT = "/dev/cu.usbserial-A7B88TE5"
BAUD = 38400

RESET = b"ATZ\r"
DEVICE_INFO = b"ATI\r"
ENGINE_SPEED = b"010C\r"
COOLANT_TEMP = b"0105\r"

with serial.Serial(PORT, BAUD, timeout=2) as elm:
    elm.write(RESET)
    print(elm.read_until(b">").decode())

    elm.write(DEVICE_INFO)
    print(elm.read_until(b">").decode())

    while True:
        time.sleep(0.2)

        elm.write(ENGINE_SPEED)
        print(elm.read_until(b">").decode(), end=('\r'))

        elm.write(COOLANT_TEMP)
        print(elm.read_until(b">").decode(), end=('\r'))
    