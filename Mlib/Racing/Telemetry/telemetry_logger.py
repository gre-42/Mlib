#!/usr/bin/env python3

import socket
import struct
import sys

def log_metric_positions(port=20777, filename='/tmp/telemetry_log.csv'):

    print(r'''Make sure that "%USERPROFILE%\Documents\My Games\DiRT Rally\hardwaresettings\hardware_settings_config.xml" contains
    <udp enabled="true" extradata="3" ip="127.0.0.1" port="20777" delay="1" />
    ''', file=sys.stderr)
    print(f'Logging metric coordinates to "{filename}". Units: Meters.', file=sys.stderr)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket, open(filename, 'w') as f:
        udp_socket.bind(('127.0.0.1', port))
        
        print('Press Ctrl+C to stop.', file=sys.stderr)

        # CSV Header
        f.write('time_sec,x_m,y_m,z_m,speed_ms,g_lat_ms2,g_lon_ms2,g_ver_ms2\n')
        
        try:
            while True:
                # Receive packet (1024 is plenty for DR2 packets)
                data, addr = udp_socket.recvfrom(1024)

                if len(data) != 264: # // 66 floats * 4 bytes = 264 bytes
                    print(f'Expected packet size of 264. Actual: {len(data)}. Please make sure extradata="3"', file=sys.stderr)
                    continue

                # Unpack relevant fields
                # Byte 0: Time [s]
                # Byte 16-44: Position X [m], Y [m], Z [m],
                #             Speed [m/s],
                #             G-force lateral (Local space)
                #             G-force longitudinal (Local space)
                #             G-force vertical (Local space)
                time_s = struct.unpack('<f', data[0:4])[0]
                pos_x_m, pos_y_m, pos_z_m, \
                speed_ms, \
                g_lat_ms2, g_lon_ms2, g_ver_ms2 = struct.unpack('<fffffff', data[16:44])
                
                # Write and show progress
                f.write(f'{time_s:.4f},{pos_x_m:.4f},{pos_y_m:.4f},{pos_z_m:.4f},{speed_ms:.4f},{g_lat_ms2:.4f},{g_lon_ms2:.4f},{g_ver_ms2:.4f}\n')
                print(f'Time: {time_s:0.2f}s | Pos: X={pos_x_m:0.1f}m, Y={pos_y_m:0.1f}m, Z={pos_z_m:0.1f}m', file=sys.stderr)
                
        except KeyboardInterrupt:
            print('Logging finished.', file=sys.stderr)

if __name__ == '__main__':
    log_metric_positions()
