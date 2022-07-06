import logging
import zipfile

from construct import Bytes, Int8ul, Int16ub, Int16ul, Int32ul, Struct
from typing import Any, Dict, Optional, Tuple

from comm import Cmd, CommClient, get_client
from comm import CommClient, get_client
from app import App
from hsm import hsm_sign_app
from manifest import Manifest
from merkletree import Entry, MerkleTree
from streamer import StreamerABC, setup_logging


logger = logging.getLogger()


class Page:
    def __init__(self, data: bytes, iv=0, read_only=False) -> None:
        self.read_only = False
        self.update(data, iv)
        self.read_only = read_only

    def update(self, data: bytes, iv: int) -> None:
        assert not self.read_only
        self.data = data
        self.iv = iv


class DeviceStream:
    PAGE_SIZE = 0x00000100
    PAGE_MASK = 0xffffff00
    PAGE_MASK_INVERT = (~PAGE_MASK & 0xffffffff)

    def __init__(self, app: App, client: CommClient) -> None:
        self.pages: Dict[int, Page] = {}
        self.merkletree = MerkleTree()
        self.manifest = app.manifest
        self.signature = app.manifest_hsm_signature

        m = Manifest(app.manifest)

        addr = m.data_start
        for data in app.data_pages:
            assert addr not in self.pages
            # The IV is set to 0. It allows the VM to tell which key should be
            # used for decryption.
            self._write_page(addr, data, 0)
            addr += DeviceStream.PAGE_SIZE

        addr = m.code_start
        for data in app.code_pages:
            assert addr not in self.pages
            self._write_page(addr, data, 0)
            addr += DeviceStream.PAGE_SIZE

        self.send_buffer = b""
        self.send_buffer_counter = 0
        self.recv_buffer = b""
        self.recv_buffer_counter = 0

        self.initialized = False
        self.client = client

    def _get_page(self, addr: int) -> Page:
        assert (addr & DeviceStream.PAGE_MASK_INVERT) == 0
        return self.pages[addr]

    def _write_page(self, addr: int, data: bytes, iv: int) -> None:
        assert (addr & DeviceStream.PAGE_MASK_INVERT) == 0
        assert len(data) == DeviceStream.PAGE_SIZE
        if addr not in self.pages:
            self.pages[addr] = Page(data, iv)
            self.merkletree.insert(Entry.from_values(addr, data, iv))
        else:
            self.pages[addr].update(data, iv)
            self.merkletree.update(Entry.from_values(addr, data, iv))

    @staticmethod
    def _parse_request(struct: Struct, data: bytes) -> Any:
        assert len(data) == struct.sizeof()
        request = struct.parse(data)

        return request

    def cmd_exchange(self, data: bytes = b"\0") -> Tuple[int, bytes]:
        assert len(data) != 0

        raw_data = self.client.raw_exchange(data)
        assert len(raw_data) >= 2

        cmd = Struct("cmd" / Int16ub).parse(raw_data[-2:]).cmd
        return cmd, raw_data[:-2]

    def init_app(self) -> Tuple[int, bytes]:
        assert not self.initialized

        apdu = self.client.exchange(0, data=self.signature)
        assert apdu.status == Cmd.REQUEST_MANIFEST, f"received: {hex(apdu.status)}, {apdu.data[:8].hex()}"
        assert len(apdu.data) == 0

        self.initialized = True

        return self.cmd_exchange(self.manifest)

    def handle_read_access(self, data: bytes) -> Tuple[int, bytes]:
        # 1. receive address of the requested page
        addr = DeviceStream._parse_request(Struct("addr" / Int32ul), data).addr
        logger.debug(f"read access: {addr:#x}")

        # 2. send encrypted page data
        page = self._get_page(addr)
        assert len(page.data) == DeviceStream.PAGE_SIZE
        data_struct = Struct("iv" / Int32ul, "page_data" / Bytes(DeviceStream.PAGE_SIZE))

        cmd, data = self.cmd_exchange(data_struct.build(dict(iv=page.iv, page_data=page.data)))

        assert cmd == Cmd.REQUEST_PROOF
        assert addr == DeviceStream._parse_request(Struct("addr" / Int32ul), data).addr

        # 3. send merkle proof
        entry, proof = self.merkletree.get_proof(addr)
        assert entry.addr == addr
        assert entry.counter == page.iv

        # TODO: handle larger proofs
        assert len(proof) <= 260, f"{len(proof)} > 260"

        return self.cmd_exchange(proof)

    def handle_write_access(self, data: bytes) -> Tuple[int, bytes]:
        # 1. receive contextual information
        request = DeviceStream._parse_request(Struct("addr" / Int32ul, "iv" / Int32ul), data)

        # 2. send merkle proof
        if self.merkletree.has_addr(request.addr):
            # proof of previous value
            old_page, proof = self.merkletree.get_proof(request.addr)
            assert old_page.addr == request.addr
            assert old_page.counter + 1 == request.iv
            update = True
        else:
            # proof of last entry
            _, proof = self.merkletree.get_proof_of_last_entry()
            update = False

        # TODO: handle larger proofs
        assert len(proof) <= 260
        cmd, data = self.cmd_exchange(proof)

        # 3. if it's an update, send the old page
        if update:
            page = self._get_page(request.addr)
            data_struct = Struct("iv" / Int32ul, "page_data" / Bytes(DeviceStream.PAGE_SIZE))
            return self.cmd_exchange(data_struct.build(dict(iv=page.iv, page_data=page.data)))
        else:
            return self.cmd_exchange()

        # 4. commit new page
        assert cmd == Cmd.COMMIT_PAGE
        assert len(data) == DeviceStream.PAGE_SIZE
        self._write_page(request.addr, data, request.iv)

    def handle_send_buffer(self, data: bytes) -> bool:
        logger.debug(f"got buffer {data!r}")

        request = DeviceStream._parse_request(Struct("data" / Bytes(253), "size" / Int8ul, "counter" / Int32ul), data)

        logger.debug(f"got buffer {request.data!r} (counter: {request.counter})")

        stop = (request.counter & 0x80000000) != 0
        assert self.send_buffer_counter == (request.counter & 0x7fffffff)
        self.send_buffer_counter += 1

        assert request.size <= 253
        self.send_buffer += request.data[:request.size]

        return stop

    def handle_recv_buffer(self, data: bytes) -> Tuple[int, bytes]:
        request = DeviceStream._parse_request(Struct("counter" / Int32ul, "maxsize" / Int16ul), data)

        assert self.recv_buffer_counter == request.counter
        self.recv_buffer_counter += 1

        buf = self.recv_buffer[:request.maxsize]
        self.recv_buffer = self.recv_buffer[request.maxsize:]
        logger.debug(f"buf: {buf!r}")
        logger.debug(f"buffer: {self.recv_buffer!r}")
        if len(self.recv_buffer) == 0:
            logger.debug(f"recv buffer last (size: {request.maxsize}, {len(buf)})")
            self.recv_buffer_counter = 0
            stop = 0x01
        else:
            stop = 0x00

        data_struct = Struct("stop" / Int8ul, "data" / Bytes(len(buf)))

        return self.cmd_exchange(data_struct.build(dict(stop=stop, data=buf)))

    def handle_exit(self, data: bytes) -> None:
        request = DeviceStream._parse_request(Struct("code" / Int32ul), data)
        logger.warn(f"app exited with code {request.code}")

    def handle_fatal(self, data: bytes) -> None:
        request = DeviceStream._parse_request(Struct("message" / Bytes(254)), data)
        message = request.message.rstrip(b"\x00")
        logger.warn(f"app encountered a fatal error: {message}")

    def exchange(self, recv_buffer: bytes, exit_app=False) -> Optional[bytes]:
        if not self.initialized:
            cmd, data = self.init_app()
        else:
            # resume execution after previous exchange call
            cmd, data = self.cmd_exchange()

        buffer_received = False
        while True:
            logger.debug(f"[<] {cmd:#06x} {data[:8].hex()}...")
            if cmd == Cmd.REQUEST_PAGE:
                cmd, data = self.handle_read_access(data)
            elif cmd == Cmd.COMMIT_INIT:
                cmd, data = self.handle_write_access(data)
            elif cmd == Cmd.SEND_BUFFER:
                stop = self.handle_send_buffer(data)
                if stop:
                    logger.info(f"received buffer: {self.send_buffer!r} (len: {len(self.send_buffer)})")
                    send_buffer = self.send_buffer
                    self.send_buffer = b""
                    self.send_buffer_counter = 0
                    return send_buffer
                else:
                    cmd, data = self.cmd_exchange()
            elif cmd == Cmd.RECV_BUFFER:
                if len(self.recv_buffer) == 0:
                    # The app mustn't call recv() twice without having called
                    # send() after the first call.
                    assert buffer_received is False
                    self.recv_buffer = recv_buffer
                    buffer_received = True
                cmd, data = self.handle_recv_buffer(data)
            elif cmd == Cmd.EXIT:
                self.handle_exit(data)
                break
            elif cmd == Cmd.FATAL:
                self.handle_fatal(data)
                break
            else:
                logger.error(f"unexpected cmd: {cmd:#06x}, {data[:8].hex()}...")
                assert False

        return None

    def exit_app(self):
        self.exchange(b"", exit_app=True)


class DeviceStreamer(StreamerABC):
    """Stream a RISC-V app to a device."""

    def __init__(self, args) -> None:
        global logger

        logger = setup_logging(args.verbose)

        if zipfile.is_zipfile(args.app):
            zip_path = args.app
            app = App.from_zip(zip_path)
        else:
            logger.warn("app is an ELF file... retrieving the HSM signature")
            zip_path = "/tmp/app.zip"
            app = hsm_sign_app(args.app)
            app.export_zip(zip_path)

        self.client = get_client(args.transport, args.speculos)
        self.app = app

    def __enter__(self) -> "DeviceStreamer":
        self.client.__enter__()
        self.stream = DeviceStream(self.app, self.client)
        return self

    def __exit__(self, type, value, traceback):
        self.stream.exit_app()
        self.client.__exit__(type, value, traceback)
        self.client = None

    def exchange(self, recv_buffer: bytes) -> Optional[bytes]:
        return self.stream.exchange(recv_buffer)
