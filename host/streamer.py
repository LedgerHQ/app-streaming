import logging

from abc import ABC, abstractmethod
from typing import Optional


def setup_logging(verbose: bool) -> logging.Logger:
    log_level = logging.INFO
    if verbose:
        log_level = logging.DEBUG

    logging.basicConfig(level=log_level, format="%(asctime)s.%(msecs)03d:%(name)s: %(message)s", datefmt="%H:%M:%S")

    return logging.getLogger("stream")


class StreamerABC(ABC):
    @abstractmethod
    def __init__(self, args) -> None:
        pass

    @abstractmethod
    def exchange(self, recv_buffer: bytes) -> Optional[bytes]:
        pass
