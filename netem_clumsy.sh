#!/bin/bash

# Define variables with default or empty values
INTERFACE=""
PORT=""
DELAY_VAL="0ms"   # Default to 0 delay
LOSS_VAL="0%"     # Default to 0% loss

# Function to display usage instructions
usage() {
    echo "Usage: sudo ./netem_flags.sh -i <interface> -p <port> [--delay <value>] [--loss <%>]"
    echo "  --delay and --loss are optional. If omitted, they default to 0."
    echo "  Examples:"
    echo "    sudo ./netem_flags.sh -i eth0 -p 8080 --delay 100ms --loss 5%"
    echo "    sudo ./netem_flags.sh -i lo -p 5000 --delay 50ms"
    exit 1
}

# Check for root privileges
if [ "$(id -u)" -ne 0 ]; then
   echo "This script must be run as root or with sudo."
   exit 1
fi

# Parse command-line arguments
while [ "$#" -gt 0 ]; do
    case "$1" in
        -i|--interface)
            INTERFACE="$2"
            shift 2
            ;;
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        --delay)
            DELAY_VAL="$2"
            shift 2
            ;;
        --loss)
            LOSS_VAL="$2"
            shift 2
            ;;
        *)
            echo "Unknown parameter: $1"
            usage
            ;;
    esac
done

# Check if required arguments are provided
if [ -z "$INTERFACE" ] || [ -z "$PORT" ]; then
    echo "Error: Missing required interface (-i) or port (-p)."
    usage
fi

cleanup() {
    echo -e "\nRestoring default network settings on $INTERFACE..."
    tc qdisc del dev "$INTERFACE" root 2>/dev/null
    echo "Network settings restored. Exiting."
    exit 0
}

# Trap Ctrl+C (SIGINT) to ensure cleanup runs if the script is interrupted
trap cleanup SIGINT

# --- Apply Configuration ---

echo "Applying configuration:"
echo "Interface: $INTERFACE, Port: $PORT, Delay: $DELAY_VAL, Loss: $LOSS_VAL"

tc qdisc del dev "$INTERFACE" root 2>/dev/null # Start fresh

# 1. Add parent HTB qdisc
tc qdisc add dev "$INTERFACE" root handle 1: htb default 1

# 2. Add default class (1:1) for normal traffic - explicitly set quantum
tc class add dev "$INTERFACE" parent 1: classid 1:1 htb rate 1000mbit quantum 1500

# 3. Add specific class (1:10) for conditioned traffic - explicitly set quantum
tc class add dev "$INTERFACE" parent 1: classid 1:10 htb rate 1000mbit quantum 1500

# 4. Attach netem qdisc with BOTH delay and loss parameters
tc qdisc add dev "$INTERFACE" parent 1:10 netem delay "$DELAY_VAL" loss "$LOSS_VAL"

# 5. Add a filter to direct traffic for the specific port to the conditioned class (1:10)
tc filter add dev "$INTERFACE" protocol ip parent 1:0 prio 1 u32 match ip dport "$PORT" 0xffff flowid 1:10

echo "Configuration applied."
echo "Press [Enter] or Ctrl+C to restore network defaults and exit."

# Wait for a keypress
read -n 1 -s keypress

cleanup
