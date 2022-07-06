import logging
import os
import sys
import time

from abc import ABC, abstractmethod
from collections import namedtuple
from enum import IntEnum
from typing import Optional, Union

from ledgerwallet.client import LedgerClient
from ledgerwallet.transport import enumerate_devices
from ledgerwallet.utils import serialize


Apdu = namedtuple("Apdu", "status data")
logger = logging.getLogger("comm")
CLA = 0x12


class ApduCmd(IntEnum):
    REQUEST_PAGE = 0x6101
    REQUEST_HMAC = 0x6102
    REQUEST_PROOF = 0x6103
    COMMIT_PAGE = 0x6201
    COMMIT_HMAC = 0x6202
    SEND_BUFFER = 0x6301
    RECV_BUFFER = 0x6401
    EXIT = 0x6501
    FATAL = 0x6601
    REQUEST_MANIFEST = 0x6701
    REQUEST_APP_PAGE = 0x6801
    REQUEST_APP_HMAC = 0x6802


class CommClient(ABC):
    def exchange(self, ins: int, data=b"", p1=0, p2=0, cla=CLA) -> Apdu:
        apdu = bytes([cla, ins, p1, p2])
        apdu += serialize(data)

        response = self._exchange(apdu)

        status_word = int.from_bytes(response[-2:], "big")
        return Apdu(status_word, response[:-2])

    @abstractmethod
    def __enter__(self) -> "CommClient":
        pass

    @abstractmethod
    def __exit__(self, type, value, traceback):
        pass

    @abstractmethod
    def _exchange(self, data: bytes) -> bytes:
        pass


class CommClientUSB(CommClient):
    def __init__(self):
        self.client = None

    def __enter__(self) -> "CommClientUSB":
        devices = enumerate_devices()
        if len(devices) == 0:
            logger.error("No Ledger device has been found.")
            sys.exit(0)

        self.client = LedgerClient(devices[0], cla=CLA)
        self.number_exchanges = 0
        self.total_time = 0.0

        logger_ledgerwallet = logging.getLogger("ledgerwallet")
        logger_ledgerwallet.setLevel(logging.INFO)

        return self

    def __exit__(self, type, value, traceback):
        average = int((self.total_time * 1000) // self.number_exchanges)
        logger.info(f"{self.number_exchanges} exchanges in {self.total_time:.1f} s, ~{average} ms / exchange")

        self.client.close()
        self.client = None

    def _exchange(self, data: bytes) -> bytes:
        start = time.time()
        response = self.client.raw_exchange(data)
        end = time.time()

        self.number_exchanges += 1
        self.total_time += end - start

        return response


class CommClientBLE(CommClient):
    def __init__(self):
        from ble import disconnect_ble_client, exchange_ble, get_client_ble
        self.disconnect_ble_client = disconnect_ble_client
        self.exchange_ble = exchange_ble
        self.get_client_ble = get_client_ble
        self.client = None

    def __enter__(self) -> "CommClientBLE":
        self.client = self.get_client_ble()
        self.number_exchanges = 0
        self.total_time = 0.0
        return self

    def __exit__(self, type, value, traceback):
        average = int((self.total_time * 1000) // self.number_exchanges)
        logger.info(f"{self.number_exchanges} exchanges in {self.total_time:.1f} s, ~{average} ms / exchange")

        self.disconnect_ble_client(self.client)
        self.client = None

    def _exchange(self, data: bytes) -> bytes:
        start = time.time()
        response = self.exchange_ble(self.client, data)
        end = time.time()

        self.number_exchanges += 1
        self.total_time += end - start

        return response


client: Optional[Union[CommClientUSB, CommClientBLE]] = None


def get_client(transport="usb", use_speculos=False) -> Union[CommClientUSB, CommClientBLE]:
    global client

    if use_speculos and transport == "usb":
        os.environ["LEDGER_PROXY_ADDRESS"] = "127.0.0.1"
        os.environ["LEDGER_PROXY_PORT"] = "9999"

    if client is None:
        if transport.lower() == "usb":
            client = CommClientUSB()
        elif transport.lower() == "ble":
            client = CommClientBLE()
        else:
            assert False

    return client
