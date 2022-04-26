import logging
import os
import sys

from abc import ABC, abstractmethod
from collections import namedtuple
from typing import Union

from ledgerwallet.client import LedgerClient
from ledgerwallet.transport import enumerate_devices
from ledgerwallet.utils import serialize


Apdu = namedtuple("Apdu", "status data")
logger = logging.getLogger("comm")
CLA = 0x12


def import_ledgerwallet(use_speculos: bool) -> None:
    if use_speculos:
        os.environ["LEDGER_PROXY_ADDRESS"] = "127.0.0.1"
        os.environ["LEDGER_PROXY_PORT"] = "9999"

    if False:
        logger.setLevel(logging.DEBUG)


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

        return self

    def __exit__(self, type, value, traceback):
        self.client.close()
        self.client = None

    def _exchange(self, data: bytes) -> bytes:
        return self.client.raw_exchange(data)


class CommClientBLE(CommClient):
    def __init__(self):
        from ble import disconnect_ble_client, exchange_ble, get_client_ble
        self.disconnect_ble_client = disconnect_ble_client
        self.exchange_ble = exchange_ble
        self.get_client_ble = get_client_ble
        self.client = None

    def __enter__(self) -> "CommClientBLE":
        self.client = self.get_client_ble()
        return self

    def __exit__(self, type, value, traceback):
        self.disconnect_ble_client(self.client)
        self.client = None

    def _exchange(self, data: bytes) -> bytes:
        return self.exchange_ble(self.client, data)


client: Union[CommClientUSB, CommClientBLE] = None


def get_client(transport="usb") -> Union[CommClientUSB, CommClientBLE]:
    global client

    if client is None:
        if transport.lower() == "usb":
            client = CommClientUSB()
        elif transport.lower() == "ble":
            client = CommClientBLE()
        else:
            assert False

    return client
