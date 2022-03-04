#!/usr/bin/env python3

"""
Interact with an app.

2 modes are available:

- through the 'streamer' (which communicates with a real device or speculos)
- with a native app, which is launched directly.
"""

import argparse
import ctypes
import importlib.util
import signal
import socket
import subprocess
import sys
import time


def set_pdeath(sig=signal.SIGTERM):
    """Set the parent death signal of the calling process."""

    PR_SET_PDEATHSIG = 1
    libc = ctypes.cdll.LoadLibrary('libc.so.6')
    libc.prctl(PR_SET_PDEATHSIG, sig)


class Client:
    def __init__(self, plugin_path, path=None):
        self.plugin = Client.load_plugin(plugin_path)
        if path:
            self.proc = subprocess.Popen(path, stdin=subprocess.PIPE, stdout=subprocess.PIPE, preexec_fn=set_pdeath)
        else:
            self.proc = None

    @staticmethod
    def load_plugin(plugin_path):
        module_name = "plugin"
        spec = importlib.util.spec_from_file_location(module_name, plugin_path)
        module = importlib.util.module_from_spec(spec)
        sys.modules[module_name] = module
        spec.loader.exec_module(module)

        from plugin import Plugin
        return Plugin()

    def prepare_request(self, name):
        method_name = f"{name}_prepare_request"
        method = getattr(self.plugin, method_name)
        return method()

    def parse_response(self, name, data):
        method_name = f"{name}_parse_response"
        method = getattr(self.plugin, method_name)
        return method(data)

    @staticmethod
    def _recvall(s, size):
        data = b""
        while size > 0:
            chunk = s.recv(size)
            if not chunk:
                raise Exception("Server: Connection closed with client")
            data += chunk
            size -= len(chunk)
        return data

    @staticmethod
    def _readall(fp, size):
        data = b""
        while size > 0:
            chunk = fp.read(size)
            if not chunk:
                raise Exception("Server: Connection closed with client")
            data += chunk
            size -= len(chunk)
        return data

    def call_device(self, request):
        connected = False
        while not connected:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                s.connect(("127.0.0.1", 1111))
            except ConnectionRefusedError:
                print("connection refused...")
                time.sleep(0.5)
            else:
                connected = True

        size = len(request).to_bytes(4, "little")
        s.sendall(size)
        s.sendall(request)

        data = Client._recvall(s, 4)
        size = int.from_bytes(data, "little")
        data = Client._recvall(s, size)

        s.close()

        return data

    def call_native(self, request):
        size = len(request).to_bytes(4, "little")
        self.proc.stdin.write(size)
        self.proc.stdin.write(request)
        self.proc.stdin.flush()

        data = Client._readall(self.proc.stdout, 4)
        size = int.from_bytes(data, "little")
        data = Client._readall(self.proc.stdout, size)

        return data

    def call(self, name):
        """Send a request to the app and return the response."""

        request = self.prepare_request(name)

        if self.proc:
            data = self.call_native(request)
        else:
            data = self.call_device(request)

        response = self.parse_response(name, data)
        print(f"[*] {response}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="RISC-V client.")
    parser.add_argument("--app", type=str, help="native application path")
    parser.add_argument("--plugin", type=str, required=True, help=".py plugin file")

    args = parser.parse_args()

    client = Client(args.plugin, args.app)

    if client.plugin.name == "boilerplate":
        client.call("get_version")
        client.call("sign_tx")
    elif client.plugin.name == "ethereum":
        #client.call("get_pubkey")
        #client.call("sign_tx")
        #client.call("sign_msg")
        client.call("sign_eip712")
    elif client.plugin.name == "sha256":
        client.call("hash1")
        client.call("hash2")
    else:
        print("unknown plugin")
