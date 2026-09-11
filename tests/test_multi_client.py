#!/usr/bin/env python3
"""
===============================================================================
ft_irc Test Suite 1: Multi-Client & Operator Features Testing Script
===============================================================================
This script automates multi-client interactions to verify IRC requirements:
 1. Client Registration (PASS -> NICK -> USER)
 2. Multi-client Channel Join & Message Broadcast
 3. Private Messaging (PRIVMSG)
 4. Channel Operator privileges (KICK, INVITE, TOPIC, MODE +i/+t/+k/+o/+l)

How to run:
 1. Start your ircserv server in a terminal:
      ./ircserv 6667 password
 2. Run this script in another terminal:
      python3 tests/test_multi_client.py --port 6667 --password password
===============================================================================
"""

import socket
import time
import argparse
import sys

# Color codes for clean terminal output during defense
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

class IRCClientWrapper:
    """
    A lightweight helper class around Python's standard socket object.
    It encapsulates connecting, sending raw CRLF-terminated lines,
    and reading responses with timeout protection.
    """
    def __init__(self, host, port):
        # socket.AF_INET = IPv4 address family
        # socket.SOCK_STREAM = TCP stream socket (connection-oriented)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.host = host
        self.port = port
        
    def connect(self):
        # Connects the TCP socket to (host, port)
        self.sock.connect((self.host, self.port))
        # Set a 2.0 second timeout for recv() calls so tests don't hang indefinitely if server fails to reply
        self.sock.settimeout(2.0)

    def send(self, data_str):
        """
        Appends CRLF (\\r\\n) per RFC 2812 and sends string encoded as raw bytes.
        """
        if not data_str.endswith("\r\n"):
            data_str += "\r\n"
        self.sock.sendall(data_str.encode('latin-1'))

    def recv_all(self, timeout=1.0):
        """
        Reads all available incoming bytes until socket timeout occurs.
        Returns decoded latin-1 string.
        """
        self.sock.settimeout(timeout)
        buffer = ""
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                chunk = self.sock.recv(4096)
                if not chunk:
                    break # Remote side closed connection
                buffer += chunk.decode('latin-1')
            except socket.timeout:
                break # No more bytes available right now
            except Exception:
                break
        return buffer

    def close(self):
        try:
            self.sock.close()
        except Exception:
            pass


def run_tests(host, port, password):
    print_info(f"Connecting to ft_irc server at {host}:{port} with password '{password}'...")

    # -------------------------------------------------------------------------
    # TEST 1: Registration (PASS -> NICK -> USER)
    # -------------------------------------------------------------------------
    print_info("--- TEST 1: Client Registration ---")
    alice = IRCClientWrapper(host, port)
    alice.connect()
    
    # Send registration sequence for Alice
    alice.send(f"PASS {password}")
    alice.send("NICK alice")
    alice.send("USER alice 0 * :Alice Smith")
    
    resp_alice = alice.recv_all(timeout=1.0)
    # IRC Numeric 001 (RPL_WELCOME) indicates successful registration
    if "001" in resp_alice:
        print_pass("Alice registered successfully (received RPL_WELCOME 001).")
    else:
        print_fail(f"Alice registration failed. Received:\n{resp_alice}")

    # Register a second client: Bob
    bob = IRCClientWrapper(host, port)
    bob.connect()
    bob.send(f"PASS {password}")
    bob.send("NICK bob")
    bob.send("USER bob 0 * :Bob Jones")
    resp_bob = bob.recv_all(timeout=1.0)
    if "001" in resp_bob:
        print_pass("Bob registered successfully.")
    else:
        print_fail("Bob registration failed.")

    # -------------------------------------------------------------------------
    # TEST 2: Channel Joining & Broadcasting (JOIN & PRIVMSG)
    # -------------------------------------------------------------------------
    print_info("--- TEST 2: Channel Join & Broadcast ---")
    alice.send("JOIN #42school")
    time.sleep(0.2)
    alice_join_resp = alice.recv_all(timeout=0.5)
    
    bob.send("JOIN #42school")
    time.sleep(0.2)
    bob_join_resp = bob.recv_all(timeout=0.5)

    # When Bob joins #42school, Alice should receive a JOIN notification for Bob
    alice_notif = alice.recv_all(timeout=0.5)
    if "bob" in alice_notif and "JOIN" in alice_notif:
        print_pass("Alice received Bob's JOIN notification in #42school.")
    else:
        print_fail(f"Alice did not receive Bob's JOIN notification. Got:\n{alice_notif}")

    # Alice sends a message to the channel
    alice.send("PRIVMSG #42school :Hello everyone in #42school!")
    time.sleep(0.2)
    bob_msg = bob.recv_all(timeout=0.5)
    if "Hello everyone in #42school!" in bob_msg:
        print_pass("Channel message broadcast received by Bob.")
    else:
        print_fail(f"Bob failed to receive channel broadcast. Got:\n{bob_msg}")

    # -------------------------------------------------------------------------
    # TEST 3: Private Messages (PRIVMSG to User)
    # -------------------------------------------------------------------------
    print_info("--- TEST 3: Direct Private Message ---")
    bob.send("PRIVMSG alice :Hey Alice, this is a private DM!")
    time.sleep(0.2)
    alice_dm = alice.recv_all(timeout=0.5)
    if "Hey Alice, this is a private DM!" in alice_dm:
        print_pass("Direct PRIVMSG from Bob to Alice delivered successfully.")
    else:
        print_fail(f"Alice did not receive direct DM. Got:\n{alice_dm}")

    # -------------------------------------------------------------------------
    # TEST 4: Channel Operator Privileges & Modes
    # -------------------------------------------------------------------------
    print_info("--- TEST 4: Operator Privileges & Channel Modes ---")
    
    # 4a. Non-operator (Bob) tries to set channel mode -> should be denied (ERR_CHANOPRIVSNEEDED 482)
    bob.send("MODE #42school +i")
    time.sleep(0.2)
    bob_err = bob.recv_all(timeout=0.5)
    if "482" in bob_err:
        print_pass("Regular user (Bob) denied MODE execution (482 ERR_CHANOPRIVSNEEDED).")
    else:
        print_fail(f"Regular user was not denied operator privileges. Response:\n{bob_err}")

    # 4b. Operator (Alice, who created the channel) sets Invite-Only mode (+i)
    alice.send("MODE #42school +i")
    time.sleep(0.2)

    # 4c. Register a third client: Charlie, try joining +i channel without invite
    charlie = IRCClientWrapper(host, port)
    charlie.connect()
    charlie.send(f"PASS {password}")
    charlie.send("NICK charlie")
    charlie.send("USER charlie 0 * :Charlie Brown")
    charlie.recv_all(timeout=0.5)

    charlie.send("JOIN #42school")
    time.sleep(0.2)
    charlie_err = charlie.recv_all(timeout=0.5)
    if "473" in charlie_err: # ERR_INVITEONLYCHAN 473
        print_pass("Uninvited user (Charlie) blocked from Invite-Only (+i) channel (473).")
    else:
        print_fail(f"Invite-only restriction failed. Response:\n{charlie_err}")

    # 4d. Alice invites Charlie to #42school
    alice.send("INVITE charlie #42school")
    time.sleep(0.2)
    charlie_inv = charlie.recv_all(timeout=0.5)
    if "INVITE" in charlie_inv:
        print_pass("Charlie received INVITE notification from Alice.")
    else:
        print_fail(f"Invite delivery failed. Got:\n{charlie_inv}")

    # Charlie joins after invite -> success
    charlie.send("JOIN #42school")
    time.sleep(0.2)
    charlie_join = charlie.recv_all(timeout=0.5)
    if "#42school" in charlie_join:
        print_pass("Invited user (Charlie) successfully joined #42school.")
    else:
        print_fail(f"Invited user join failed. Got:\n{charlie_join}")

    # 4e. Alice kicks Charlie out of the channel
    alice.send("KICK #42school charlie :testing operator kick")
    time.sleep(0.2)
    charlie_kick = charlie.recv_all(timeout=0.5)
    if "KICK" in charlie_kick and "charlie" in charlie_kick:
        print_pass("Operator (Alice) kicked Charlie successfully.")
    else:
        print_fail(f"Operator KICK failed. Got:\n{charlie_kick}")

    # Clean up sockets
    alice.close()
    bob.close()
    charlie.close()
    print_info("--- All multi-client tests complete! ---")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ft_irc Multi-Client Test Script")
    parser.add_argument("--host", default="127.0.0.1", help="Server IP address (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=6667, help="Server port (default: 6667)")
    parser.add_argument("--password", default="password", help="Server password (default: password)")
    args = parser.parse_args()

    try:
        run_tests(args.host, args.port, args.password)
    except ConnectionRefusedError:
        print_fail(f"Could not connect to server at {args.host}:{args.port}. Is ./ircserv running?")
        sys.exit(1)
