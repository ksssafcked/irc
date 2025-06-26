import socket
import subprocess
import time
import os
import signal

server_proc = None

ANSI_YELLOW = "\033[1;33m"
ANSI_RED = "\033[1;31m"
ANSI_GREEN = "\033[1;32m"
ANSI_CYAN = "\033[1;36m"
ANSI_WHITE = "\033[1:36m"
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
    print(ANSI_CYAN + "[info][test]: sending [" + msg + "]\n" + ANSI_RESET)
    sock.sendall((msg + '\r\n').encode())

def send_no_crlf(sock, msg):
    print(ANSI_CYAN + "[info][test]: sending (no crlf) [" + msg + "]\n" + ANSI_RESET)
    sock.sendall((msg).encode())


def recv(sock, timeout=2):
    sock.settimeout(timeout)
    try:
        return sock.recv(4096).decode()
    except socket.timeout:
        return ''

def connect_and_join(client, message):
    send_no_crlf(client, "CAP LS 302\r\nPA\x01S\x07S 42\r\nNICK " + message + "\r\nUSER "+ message + " "+ message + "\r\n")
    send_no_crlf(client, "JOIN #ledion\r\n")

def message_format():
    print("[test][message_format]")
    print(ANSI_CYAN + "[info][test]: connecting client" + ANSI_RESET)
    client = connect_client()
    client2 = connect_client()
    

    connect_and_join(client, "ledion")
    time.sleep(1)
    connect_and_join(client2, "test")
    time.sleep(1)
    send_no_crlf(client, "PRIVMSG #ledion :burek\r\n")
    time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\r\nPA\x01S\x07S 42\r\nNICK bruh\r\nUSER bruh b\r\n")
    # time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\r\nPA\x01S\x07S 42\r\nNICK bruh\r\nUSER bruh b\r\n")
    # time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\r\nPA\x01S\x07S 42\r\nNICK bruh\r\nUSER\tbruh b\r\n")
    # time.sleep(1)
    # send_no_crlf(client, "\r\r\r\x07")
    # time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\r\nPA\x01S\x07S 42\r\nNICK bruh\r\nUSER bruh b\r\n")
    # send_no_crlf(client, "CAP LS 302\n")
    # time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\r")
    # time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\r\n")
    # time.sleep(1)
    #
    # send_no_crlf(client, "CAP LS 302\n     \n\n\n")
    # time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\rNICK bruh\rUSER bruh\r")
    # time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\nNICK bruh\nUSER bruh\n")
    # time.sleep(1)
    # send_no_crlf(client, "CAP LS 302\r\nNICK bruh\r\nUSER bruh\r\n")
    # time.sleep(1)

    # response = recv(client)
    # print("response:" + response)

    client.close()
    client2.close()

def basic_connection():
    print("[test][basic_connection]")
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

        message_format()
        # basic_connection()
    finally:
        stop_server(server_proc)

