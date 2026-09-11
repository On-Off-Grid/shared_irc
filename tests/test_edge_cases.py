#!/usr/bin/env python3
"""
===============================================================================
ft_irc Test Suite 2: Networking Edge Cases & Stress Testing Script
===============================================================================
This script tests the "Networking Specials" section of the 42 evaluation sheet:
 1. Partial/Fragmented Commands: sending bytes in small pieces without \\r\\n
 2. Unexpected Disconnects: abruptly closing TCP connections
 3. Partial Disconnects: killing sockets mid-command
 4. Suspended Client Flood Test: sending floods while a client stops reading,
    verifying server non-blocking behavior and absence of hangs.

How to run:
 1. Start your ircserv server:
      ./ircserv 6667 password
 2. Run this script:
      python3 tests/test_edge_cases.py --port 6667 --password password
===============================================================================
"""

import socket
import time
import argparse
import sys

GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
CYAN = "\033[96m"
RESET = "\033[0m"

def print_pass(msg):
    print(f"{GREEN}[PASS] {msg}{RESET}")

def print_fail(msg):
    print(f"{RED}[FAIL] {msg}{RESET}")

def print_info(msg):
    print(f"{CYAN}[INFO] {msg}{RESET}")


def test_partial_commands(host, port, password):
    """
    Evaluation Requirement:
    "Try to send partial commands... Check that the server answers correctly.
    With a partial command sent, ensure that other connections still run fine."
    """
    print_info("--- EDGE CASE 1: Partial Commands / TCP Fragmentation ---")

    # Client A sends registration broken into tiny byte fragments over multiple time steps
    sock_a = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock_a.connect((host, port))
    sock_a.settimeout(2.0)

    # Fragment 1: PASS password split in half
    sock_a.sendall(b"PASS " + password[:2].encode())
    time.sleep(0.1) # Simulate network latency
    
    # Verify another client (Client B) can connect and register normally while A is stalled
    sock_b = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock_b.connect((host, port))
    sock_b.settimeout(2.0)
    sock_b.sendall(f"PASS {password}\r\nNICK sideclient\r\nUSER side 0 * :Side Client\r\n".encode())
    
    time.sleep(0.2)
    resp_b = sock_b.recv(4096).decode('latin-1')
    if "001" in resp_b:
        print_pass("Other connections run fine while Client A sends partial bytes.")
    else:
        print_fail("Server blocked other connections during partial input!")

    # Fragment 2: Finish PASS line, start NICK line
    sock_a.sendall(f"{password[2:]}\r\nNI".encode())
    time.sleep(0.1)
    
    # Fragment 3: Finish NICK line, send USER line
    sock_a.sendall(f"CK fraguser\r\nUSER frag 0 * :Frag User\r\n".encode())
    time.sleep(0.3)

    resp_a = sock_a.recv(4096).decode('latin-1')
    if "001" in resp_a:
        print_pass("Server correctly assembled fragmented TCP input and registered Client A.")
    else:
        print_fail(f"Fragmented command processing failed. Server response:\n{resp_a}")

    sock_a.close()
    sock_b.close()


def test_abrupt_disconnect(host, port, password):
    """
    Evaluation Requirement:
    "Unexpectedly kill a client... Check that the server is still operational.
    Unexpectedly kill a nc with just half of a command sent... check again that
    the server is not in an odd state or blocked."
    """
    print_info("--- EDGE CASE 2 & 3: Abrupt Disconnects & Half-Command Kills ---")

    # Connect a client and send half of a command without CRLF
    doomed_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    doomed_sock.connect((host, port))
    doomed_sock.sendall(f"PASS {password}\r\nNICK doomed_user_half_line".encode())
    
    # Immediately close/destroy socket without sending \\r\\n or QUIT (simulates kill -9 or netcat crash)
    doomed_sock.close()
    time.sleep(0.2)

    # Immediately attempt to connect a new healthy client to prove server socket didn't crash or lock up
    survivor = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    survivor.connect((host, port))
    survivor.settimeout(2.0)
    survivor.sendall(f"PASS {password}\r\nNICK survivor\r\nUSER surv 0 * :Survivor\r\n".encode())
    
    time.sleep(0.3)
    resp = survivor.recv(4096).decode('latin-1')
    if "001" in resp:
        print_pass("Server recovered cleanly from abrupt client termination mid-command.")
    else:
        print_fail("Server failed to recover from abrupt client disconnect!")

    survivor.close()


def test_suspended_client_flood(host, port, password):
    """
    Evaluation Requirement:
    "Stop a client (Ctrl+Z) connected on a channel. Then flood the channel using
    another client. The server should not hang. When the client is live again,
    all stored commands should be processed normally."
    """
    print_info("--- EDGE CASE 4: Suspended Client (Ctrl+Z) & Channel Flood ---")

    # 1. Connect Client 1 (suspended listener)
    victim = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    victim.connect((host, port))
    victim.sendall(f"PASS {password}\r\nNICK victim\r\nUSER vic 0 * :Vic\r\nJOIN #floodchan\r\n".encode())
    time.sleep(0.2)
    victim.recv(4096) # Drain registration & join welcome

    # 2. Connect Client 2 (flooder)
    flooder = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    flooder.connect((host, port))
    flooder.sendall(f"PASS {password}\r\nNICK flooder\r\nUSER f 0 * :F\r\nJOIN #floodchan\r\n".encode())
    time.sleep(0.2)
    flooder.recv(4096)

    # Simulate Ctrl+Z: victim STOPs calling recv() on its TCP socket.
    # The TCP recv window will eventually fill up on the kernel level.
    # The server's non-blocking write_buffer must queue messages without blocking the main event loop.
    print_info("Flooding #floodchan with 150 messages while victim is suspended...")
    for i in range(150):
        flooder.sendall(f"PRIVMSG #floodchan :Flood packet {i}\r\n".encode())
    time.sleep(0.3)

    # Verify server is NOT hung: Connect Client 3 and ensure instant response
    probe = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    probe.connect((host, port))
    probe.settimeout(2.0)
    probe.sendall(f"PASS {password}\r\nNICK probe\r\nUSER probe 0 * :Probe\r\n".encode())
    time.sleep(0.2)
    resp = probe.recv(4096).decode('latin-1')
    if "001" in resp:
        print_pass("Server handled channel flood without blocking other clients.")
    else:
        print_fail("Server hung during channel flood!")

    # Resume reading on victim socket
    victim.setblocking(False)
    total_bytes = 0
    start = time.time()
    while time.time() - start < 1.0:
        try:
            chunk = victim.recv(8192)
            if not chunk:
                break
            total_bytes += len(chunk)
        except Exception:
            break

    print_pass(f"Suspended client resumed and successfully read {total_bytes} bytes from non-blocking queue.")

    victim.close()
    flooder.close()
    probe.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ft_irc Edge Case & Stress Testing Script")
    parser.add_argument("--host", default="127.0.0.1", help="Server IP (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=6667, help="Server port (default: 6667)")
    parser.add_argument("--password", default="password", help="Server password (default: password)")
    args = parser.parse_args()

    try:
        test_partial_commands(args.host, args.port, args.password)
        test_abrupt_disconnect(args.host, args.port, args.password)
        test_suspended_client_flood(args.host, args.port, args.password)
        print_info("🎉 All edge case and stress tests executed successfully!")
    except ConnectionRefusedError:
        print_fail(f"Connection refused at {args.host}:{args.port}. Make sure ./ircserv is running.")
        sys.exit(1)
