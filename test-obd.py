import serial
import time

PORT = "/dev/cu.usbserial-A7B88TE5"
BAUD = 38400

RESET = b"ATZ\r"
DEVICE_INFO = b"ATI\r"
ENGINE_SPEED = b"010C\r"
COOLANT_TEMP = b"0105\r"
INTAKE_TEMP = b"010F\r"


def decode_engine_speed(response):
    """
    Decode OBD-II PID 010C (Engine RPM).

    Response format:
        41 0C A B

    RPM = ((A * 256) + B) / 4
    """
    try:
        data = response.replace(" ", "").replace("\r", "").replace("\n", "")

        # Find the 410C response
        index = data.find("410C")

        if index == -1:
            return None

        # 410C + 4 hex characters (AABB)
        value = data[index + 4:index + 8]

        if len(value) != 4:
            return None

        a = int(value[0:2], 16)
        b = int(value[2:4], 16)

        return ((a * 256) + b) / 4

    except (ValueError, IndexError):
        return None


def decode_coolant_temp(response):
    """
    Decode OBD-II PID 0105 (Engine Coolant Temperature).

    Response format:
        41 05 A

    Temperature = A - 40
    """
    try:
        data = response.replace(" ", "").replace("\r", "").replace("\n", "")

        # Find the 4105 response
        index = data.find("4105")

        if index == -1:
            return None

        # 4105 + 2 hex characters
        value = data[index + 4:index + 6]

        if len(value) != 2:
            return None

        a = int(value, 16)

        return a - 40

    except (ValueError, IndexError):
        return None


with serial.Serial(PORT, BAUD, timeout=2) as elm:

    # Reset ELM327
    elm.write(RESET)
    print(elm.read_until(b">").decode(errors="replace"))

    # Get device information
    elm.write(DEVICE_INFO)
    print(elm.read_until(b">").decode(errors="replace"))

    while True:
        time.sleep(0.2)

        # Engine RPM
        elm.write(ENGINE_SPEED)
        rpm_response = elm.read_until(b">").decode(errors="replace")

        rpm = decode_engine_speed(rpm_response)

        # Coolant temperature
        elm.write(COOLANT_TEMP)
        coolant_response = elm.read_until(b">").decode(errors="replace")

        coolant_temp = decode_coolant_temp(coolant_response)

        print(f"RPM: {rpm:.0f} | Coolant: {coolant_temp} °C")