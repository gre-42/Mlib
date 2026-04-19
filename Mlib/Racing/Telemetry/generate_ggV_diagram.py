#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt

def generate_ggv_diagram(csv_file='/tmp/telemetry_log.csv', g=9.8, radius=1, plot_limit=5):
    # Load your logged telemetry data
    df = pd.read_csv(csv_file)
    
    # Ensure the G-force columns exist (Offsets 136 and 140)
    if 'g_lat_ms2' not in df.columns or \
            'g_lon_ms2' not in df.columns or \
            'g_ver_ms2' not in df.columns:
        raise ValueError("Error: CSV must contain 'g_lat_ms2', 'g_lon_ms2', 'g_ver_ms2' columns.")

    plt.figure(figsize=(8, 8))
    
    # Scatter plot of all acceleration points
    plt.scatter(df['g_lat_ms2'] / g, df['g_lon_ms2'] / g,
                c=df['speed_ms'], cmap='viridis',
                alpha=0.4, s=2, label='Telemetry Points')

    # Add axis lines for clarity
    plt.axhline(y=0, color='black', linewidth=1)
    plt.axvline(x=0, color='black', linewidth=1)

    # Formatting the plot
    plt.title('ggV Diagram (Friction Circle colored by Speed)')
    plt.xlabel('Lateral G (Cornering)')
    plt.ylabel('Longitudinal G (Braking/Accel)')
    plt.grid(visible=True, linestyle='--', alpha=0.6)
    plt.legend()

    circle1 = plt.Circle((0, 0), radius=radius, edgecolor='k', fill=False)
    plt.gca().add_patch(circle1)

    plt.xlim(-plot_limit, plot_limit)
    plt.ylim(-plot_limit, plot_limit)

    # Keep the aspect ratio equal to see the true "circle" shape
    # plt.axis('equal')

    plt.show()

if __name__ == '__main__':
    generate_ggv_diagram()
