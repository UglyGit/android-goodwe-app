#!/usr/bin/env python3
import socket
import struct
import time
import sys

def decode_cdab_int32(high_reg, low_reg):
    """Recombines two 16-bit registers following GoodWe's CDAB word-swapping logic into a signed 32-bit int."""
    # Pack the registers back to bytes, but place the low register (least significant word) second
    # because the app expects the standard big-endian reconstruction layout.
    raw_bytes = struct.pack(">HH", low_reg, high_reg)
    return struct.unpack(">i", raw_bytes)[0]

def decode_cdab_uint32(high_reg, low_reg):
    """Recombines two 16-bit registers following GoodWe's CDAB word-swapping logic into an unsigned 32-bit int."""
    raw_bytes = struct.pack(">HH", low_reg, high_reg)
    return struct.unpack(">I", raw_bytes)[0]

def poll_inverter():
    host = "127.0.0.1"
    port = 5020
    polling_interval = 2.0 # Seconds between polls
    
    # Modbus request parameters
    start_register = 35111
    register_count = 74 # Covers 35111 through 35184 inclusive
    
    # Build standard Modbus TCP Request Frame (MBAP + PDU)
    # Transaction ID: 0x0001, Protocol ID: 0x0000, Length: 6 bytes, Unit ID: 0x01
    # Function Code: 0x03 (Read Holding Registers), Address: 35111 (0x8927), Count: 74 (0x004A)
    request_frame = struct.pack(">HHHBBHH", 1, 0, 6, 1, 3, start_register, register_count)

    print(f"Connecting to simulator at {host}:{port}...")
    print(f"Polling block: Read {register_count} registers starting at {start_register}.")
    print("Press Ctrl+C to stop.\n")

    try:
        # Open persistent socket connection
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((host, port))
            
            while True:
                s.sendall(request_frame)
                response = s.recv(1024)
                
                if not response or len(response) < 9:
                    print("Error: Invalid or empty response from server.")
                    break
                
                # Modbus TCP Header contains 9 bytes before raw data array starts
                raw_data = response[9:]
                
                # Unpack byte array into a list of 16-bit unsigned shorts
                # format string: > followed by 74 'H's
                fmt = f">{register_count}H"
                registers = struct.unpack(fmt, raw_data)
                
                # Construct a local offset dictionary relative to Modbus register addresses
                # registers[0] corresponds to index 35111, registers[1] to 35112, etc.
                reg_map = {start_register + idx: val for idx, val in enumerate(registers)}
                
                # Extract and decode live metrics using the corrected endianness logic
                battery_raw = decode_cdab_int32(reg_map[35111], reg_map[35112])
                battery_soc = reg_map[35115]
                grid_raw = decode_cdab_int32(reg_map[35172], reg_map[35173])
                solar_raw = decode_cdab_uint32(reg_map[35179], reg_map[35180])
                load_raw = decode_cdab_uint32(reg_map[35183], reg_map[35184])
                
                # Convert raw register scalings (Watts) directly to UI-ready Kilowatts (kW)
                print("--- Inverter Refresh ---")
                print(f"☀ Solar Power:   {solar_raw / 1000.0:>6.2f} kW")
                print(f"  Grid Power:    {grid_raw / 1000.0:>6.2f} kW ({'Import' if grid_raw >= 0 else 'Export'})")
                print(f"  Load Power:    {load_raw / 1000.0:>6.2f} kW")
                print(f"🔋 Battery SOC:   {battery_soc:>6}%")
                print(f"  Battery Power: {battery_raw / 1000.0:>6.2f} kW ({'Charging' if battery_raw >= 0 else 'Discharging'})\n")
                
                time.sleep(polling_interval)
                
    except KeyboardInterrupt:
        print("\n[Ctrl+C] Stopping client polling loop gracefully. Exiting.")
        sys.exit(0)
    except ConnectionRefusedError:
        print(f"Error: Could not connect to simulator at {host}:{port}. Is sim_inverter.py running?")
        sys.exit(1)

if __name__ == "__main__":
    poll_inverter()
