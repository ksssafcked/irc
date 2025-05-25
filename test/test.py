import socket
import subprocess
import time
import os
import signal

server_proc = None

ANSI_YELLOW = "\033[0;33m"
ANSI_RED = "\033[0;31m"
ANSI_GREEN = "\033[0;32m"
ANSI_CYAN = "\033[0;36m"
ANSI_WHITE = "\033[0:36m"
ANSI_RESET = "\033[0m"

SERVER_EXECUTABLE = "./build/bin/ircserv"
SERVER_PORT = 6667
SERVER_PASSWORD = "42"

def start_server():
    print(ANSI_CYAN + "[info][test]: starting server" + ANSI_RESET)
    return subprocess.Popen([SERVER_EXECUTABLE, str(SERVER_PORT), SERVER_PASSWORD])

def stop_server(proc):
    print(ANSI_CYAN + "[info][test]: stopping server" + ANSI_RESET)
    try:
        os.kill(proc.pid, signal.SIGINT)  # or signal.SIGTERM
        proc.wait(timeout=2)
    except subprocess.TimeoutExpired:
        print(ANSI_YELLOW + "[warning][test]: stopping server" + ANSI_RESET)
        proc.kill()

def connect_client():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', SERVER_PORT))
    return sock

def send(sock, msg):
    sock.sendall((msg + '\r\n').encode())

def recv(sock, timeout=2):
    sock.settimeout(timeout)
    try:
        return sock.recv(4096).decode()
    except socket.timeout:
        return ''

def basic_connection():
    print(ANSI_CYAN + "[info][test]: connecting client" + ANSI_RESET)
    client = connect_client()

    send(client, f"PASSWORD {SERVER_PASSWORD}")

    response = recv(client)
    print("response:" + response)

    client.close()

if __name__ == "__main__":
    try:
        server_proc = start_server()
        time.sleep(1)

        basic_connection()
    finally:
        stop_server(server_proc)

