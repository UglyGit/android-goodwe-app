#!/usr/bin/env python3
import socket
import struct
import logging
import threading
import time
import math

logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(levelname)s] %(message)s")
log = logging.getLogger(__name__)

# =========================================================================
# DYNAMIC DIURNAL TIMELINE GENERATOR
# Total steps: 180 (6 minutes runtime at 2 seconds per tick)
# Simulates a continuous 24-hour cycle.
# =========================================================================
TOTAL_STEPS = 180
current_step = 0
current_soc = 45.0  # Volatile state tracking battery % across ticks
state_lock = threading.Lock()

def calculate_step_data(step):
    """Calculates balanced solar metrics for a specific point in time."""
    global current_soc
    
    # Map step index (0-179) directly onto a 24-hour day clock face
    sim_hour = (step / TOTAL_STEPS) * 24.0
    
    # 1. Solar curve: Sinusoidal shape during daylight window (6 AM to 6 PM)
    solar = 0.0
    if 6.0 <= sim_hour <= 18.0:
        # Peak production around 12:00 PM (~7.5 kW)
        solar = 7500.0 * math.pow(math.sin(math.pi * (sim_hour - 6.0) / 12.0), 1.5)
        
    # 2. Home load profile: Baseline (~450W) + Morning Peak + Heavy Evening Peak
    load = 450.0 + 150.0 * math.sin(math.pi * sim_hour / 12.0)
    # Morning appliance spike (centered at 8:30 AM)
    load += 1500.0 * math.exp(-math.pow((sim_hour - 8.5) / 1.0, 2))
    # Evening family peak (centered at 7:30 PM)
    load += 2800.0 * math.exp(-math.pow((sim_hour - 19.5) / 1.8, 2))
    
    # Add a minor deterministic ripple so values don't look completely frozen
    load += 40.0 * math.sin(sim_hour * 10.0)
    load = max(350.0, load) # Enforce household floor draw
    
    # 3. Resolve Battery & Grid loops using conservation of energy laws
    net_power = solar - load
    battery = 0.0
    grid = 0.0
    
    if net_power > 0:  # Excess production
        if current_soc < 100.0:
            # Charge the battery (max capacity throughput 3.5 kW)
            battery = -min(net_power, 3500.0)
            # Increment state of charge based on simulated storage absorption speed
            current_soc += (-battery * 0.03) / 100.0
            current_soc = min(100.0, current_soc)
        # Remainder is exported back to the grid
        grid = -(solar + battery - load)
    else:  # Energy deficit
        if current_soc > 10.0:
            # Discharge the battery to cover home deficit (max speed 3.0 kW)
            battery = min(-net_power, 3000.0)
            current_soc -= (battery * 0.03) / 100.0
            current_soc = max(10.0, current_soc)
        # Remaining structural load deficit falls back directly onto Grid Import
        grid = load - solar - battery

    # Human-readable status description for terminal tracing
    h = int(sim_hour)
    m = int((sim_hour % 1) * 60)
    desc = f"Time {h:02d}:{m:02d} | "
    if solar > 0 and battery < 0:
        desc += "Solar feeding Load + charging Battery"
    elif solar > 0:
        desc += "Solar feeding Load + exporting Excess"
    elif battery > 0:
        desc += "Night/Cloudy: Battery covering household draw"
    else:
        desc += "Night/Cloudy: Battery empty, drawing Grid Import"

    return {
        "solar": int(solar),
        "load": int(load),
        "battery": int(battery),
        "grid": int(grid),
        "soc": int(current_soc),
        "desc": desc
    }

def advance_timeline():
    """Timer thread that ticks forward every 2 seconds to simulate continuous runtime."""
    global current_step
    while True:
        time.sleep(2.0)
        with state_lock:
            current_step = (current_step + 1) % TOTAL_STEPS
            state = calculate_step_data(current_step)
            log.info(f"[TICK {current_step+1}/{TOTAL_STEPS}] {state['desc']} "
                     f"(Sol:{state['solar']}W, Lod:{state['load']}W, "
                     f"Grd:{state['grid']}W, Bat:{state['battery']}W, SOC:{state['soc']}%)")

def handle_modbus_request(data):
    if len(data) < 12:
        return None
    
    transaction_id, protocol_id, length, unit_id, function_code = struct.unpack(">HHHBB", data[0:8])
    start_register, register_count = struct.unpack(">HH", data[8:12])
    
    with state_lock:
        state = calculate_step_data(current_step)

    registers = {}
    
    # Register 512: Serial Number ASCII
    sn = b"GW9999KETA123456"
    for i in range(8):
        registers[512 + i] = struct.unpack(">H", sn[i*2:i*2+2])[0]

    # Helper function to convert native values into standard Modbus 32-bit registers
    def to_regs_32(value):
        unsigned_val = struct.unpack(">II", struct.pack(">ii", value, 0))[0] if value < 0 else value
        return (unsigned_val >> 16) & 0xFFFF, unsigned_val & 0xFFFF

    # Map state structures out using GoodWe Word-Swapped Big-Endian (CDAB) conventions
    bat_h, bat_l = to_regs_32(state["battery"])
    registers[35111] = bat_l
    registers[35112] = bat_h

    registers[35115] = state["soc"]

    grid_h, grid_l = to_regs_32(state["grid"])
    registers[35172] = grid_l
    registers[35173] = grid_h

    sol_h, sol_l = to_regs_32(state["solar"])
    registers[35179] = sol_l
    registers[35180] = sol_h

    lod_h, lod_l = to_regs_32(state["load"])
    registers[35183] = lod_l
    registers[35184] = lod_h

    # Pack raw response bytes frame array safely
    byte_count = register_count * 2
    response_bytes = b""
    for r in range(start_register, start_register + register_count):
        val = registers.get(r, 0x0000)
        response_bytes += struct.pack(">H", val)

    header = struct.pack(">HHHBB", transaction_id, protocol_id, len(response_bytes) + 2, unit_id, function_code)
    return header + struct.pack(">B", byte_count) + response_bytes

def run_server():
    ticker_thread = threading.Thread(target=advance_timeline, daemon=True)
    ticker_thread.start()

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("127.0.0.1", 5020))
    server.listen(5)
    
    log.info("Starting dynamically calculated 6-Minute GoodWe Simulator on 127.0.0.1:5020...")

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
