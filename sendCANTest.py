import can

with can.Bus(channel = 0, interface = "vector", app_name = "CANalyzer", bitrate=500000) as bus:
    msg = can.Message(
        arbitration_id= 0x33,
        data=[0x10, 0x0A, 1, 3, 3, 4, 5, 6],
        is_extended_id=False
    )
    try:
        bus.send(msg)
        print(f"sent on {bus.channel_info}")
    except Exception:
        print("Failed")