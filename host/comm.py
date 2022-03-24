import logging
import os
import sys

from collections import namedtuple

from ledgerwallet.client import LedgerClient
from ledgerwallet.transport import enumerate_devices
from ledgerwallet.utils import serialize


Apdu = namedtuple("Apdu", "status data")
logger = logging.getLogger("comm")


def import_ledgerwallet(use_speculos: bool) -> None:
    if use_speculos:
        os.environ["LEDGER_PROXY_ADDRESS"] = "127.0.0.1"
        os.environ["LEDGER_PROXY_PORT"] = "9999"

    if False:
        logger.setLevel(logging.DEBUG)


def get_client() -> LedgerClient:
    CLA = 0x12
    devices = enumerate_devices()
    if len(devices) == 0:
        logger.error("No Ledger device has been found.")
        sys.exit(0)

    return LedgerClient(devices[0], cla=CLA)


def exchange(client: LedgerClient, ins: int, data=b"", p1=0, p2=0, cla=0) -> Apdu:
    if cla == 0:
        cla = client.cla
    apdu = bytes([cla, ins, p1, p2])
    apdu += serialize(data)
    response = client.raw_exchange(apdu)
    status_word = int.from_bytes(response[-2:], "big")
    return Apdu(status_word, response[:-2])
