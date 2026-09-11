#!/usr/bin/env python3
import socket
import struct
import logging

logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(levelname)s] %(message)s")
log = logging.getLogger(__name__)

def handle_modbus_request(data):
    if len(data) < 12:
        return None
    
    # Parse Modbus TCP Application Header (MBAP)
    transaction_id, protocol_id, length, unit_id, function_code = struct.unpack(">HHHBB", data[0:8])
    start_register, register_count = struct.unpack(">HH", data[8:12])
    
    log.info(f"Received Read Request - Unit: {unit_id}, Reg: {start_register}, Count: {register_count}")

    # Build response payload matrix matching the PRD data layout rules
    registers = {}
    
    # Register 512: Serial Number ASCII (16 bytes = 8 registers)
    sn = b"GW9999KETA123456"
    for i in range(8):
        registers[512 + i] = struct.unpack(">H", sn[i*2:i*2+2])[0]

    # Register 35111: Battery Power (INT32) -> -1.64 kW Discharging (-1640)
    # -1640 in Two's Complement: 0xFFFFFAF8
    registers[35111] = 0xFFFF
    registers[35112] = 0xFAF8

    # Register 35115: Battery SOC (UINT16) -> 82%
    registers[35115] = 82

    # Register 35172: Grid Power (INT32) -> 0.82 kW Import (+820)
    registers[35172] = 0x0000
    registers[35173] = 0x0334

    # Register 35179: Solar Power (UINT32) -> 3.25 kW (+3250)
    registers[35179] = 0x0000
    registers[35180] = 0x0CB2

    # Register 35183: Load Power (UINT32) -> 2.43 kW (+2430)
    registers[35183] = 0x0000
    registers[35184] = 0x097E

    # Assemble raw response data stream block
    byte_count = register_count * 2
    response_bytes = b""
    
    for r in range(start_register, start_register + register_count):
        val = registers.get(r, 0x0000)
        response_bytes += struct.pack(">H", val)

    # Build native MBAP response frame matching Modbus protocol wire formats
    header = struct.pack(">HHHBB", transaction_id, protocol_id, len(response_bytes) + 2, unit_id, function_code)
    return header + struct.pack(">B", byte_count) + response_bytes

def run_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("127.0.0.1", 5020))
    server.listen(5)
    
    log.info("Starting pure socket GoodWe Modbus Simulator on 127.0.0.1:5020...")
    log.info("Dependency loop broken. Ready for Android TV Emulator connections.")

    try:
        while True:
            client_sock, addr = server.accept()
            while True:
                data = client_sock.recv(1024)
                if not data:
                    break
                response = handle_modbus_request(data)
                if response:
                    client_sock.sendall(response)
            client_sock.close()
    except KeyboardInterrupt:
        log.info("Shutting down simulator server.")
    finally:
        server.close()

if __name__ == "__main__":
    run_server()
