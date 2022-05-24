import ctypes
import signal
import subprocess

from typing import Optional

from streamer import StreamerABC, setup_logging


def set_pdeath(sig=signal.SIGTERM) -> None:
    """Set the parent death signal of the calling process."""

    PR_SET_PDEATHSIG = 1
    libc = ctypes.cdll.LoadLibrary('libc.so.6')
    libc.prctl(PR_SET_PDEATHSIG, sig)


class NativeStreamer(StreamerABC):
    """
    Run a native app (usually x86-64).

    There's no streaming involved since there are no memory restriction in
    contrary to devices.
    """

    def __init__(self, args) -> None:
        setup_logging(args.verbose)
        self.path = args.app

    def __enter__(self) -> "NativeStreamer":
        self.proc = subprocess.Popen(self.path, stdin=subprocess.PIPE, stdout=subprocess.PIPE, preexec_fn=set_pdeath)
        return self

    def __exit__(self, type, value, traceback):
        self.proc.kill()

    @staticmethod
    def _readall(fp, size: int) -> bytes:
        data = b""
        while size > 0:
            chunk = fp.read(size)
            if not chunk:
                raise Exception("Server: Connection closed with client")
            data += chunk
            size -= len(chunk)
        return data

    def exchange(self, request: bytes) -> Optional[bytes]:
        rsize = len(request).to_bytes(4, "little")
        if self.proc.stdin is None:
            # fix mypy error: Item "None" of "Optional[IO[bytes]]" has no attribute "write"
            raise Exception("stdin is closed")
        self.proc.stdin.write(rsize)
        self.proc.stdin.write(request)
        self.proc.stdin.flush()

        data = NativeStreamer._readall(self.proc.stdout, 4)
        size = int.from_bytes(data, "little")
        data = NativeStreamer._readall(self.proc.stdout, size)

        return data
