from obdii import Connection, commands

with Connection(("127.0.0.1", 3500)) as conn:
    rpm = conn.query(commands.ENGINE_SPEED)
    engine_coolant_temp = conn.query(commands.ENGINE_COOLANT_TEMP)

    print(f"RPM: {rpm.value}")
    print(f"Units: {rpm.units}")
    print(f"Engine Coolant Temp: {engine_coolant_temp.value} {engine_coolant_temp.units}")