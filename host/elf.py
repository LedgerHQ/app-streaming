import logging
import sys

from elftools.elf.constants import P_FLAGS
from elftools.elf.elffile import ELFFile
from elftools.elf.segments import Segment as ElfSegment
from typing import List, Tuple


logger = logging.getLogger("elf")


class Segment:
    PAGE_SIZE = 0x00000100
    PAGE_MASK = 0xffffff00
    PAGE_MASK_INVERT = (~PAGE_MASK & 0xffffffff)

    def __init__(self, segment: ElfSegment) -> None:
        assert (segment["p_align"] % Segment.PAGE_SIZE) == 0

        data = segment.data()
        size = segment["p_filesz"]
        start = segment["p_vaddr"]

        mask = Segment.PAGE_MASK_INVERT
        gap_start = start & mask
        gap_end = (Segment.PAGE_SIZE - ((start + size) & mask)) & mask
        data = (b"\x00" * gap_start) + data + (b"\x00" * gap_end)
        assert (len(data) % Segment.PAGE_SIZE) == 0

        self.data = data
        self.start = start & Segment.PAGE_MASK
        self.end = start + size + gap_end
        self.size = size + gap_start + gap_end


class Elf:
    HEAP_SIZE = 0x10000
    STACK_SIZE = 0x10000

    def __init__(self, path: str) -> None:
        with open(path, "rb") as fp:
            elf = ELFFile(fp)
            assert elf.get_machine_arch() == "RISC-V"
            self.app_infos = Elf._get_app_infos(elf)
            self.segments = Elf._parse_segments(elf)
            self.entrypoint = elf.header["e_entry"]

    @staticmethod
    def _get_app_infos(elf: ELFFile) -> dict:
        """
        Retrieve the sections .app_name and .app_version.
        """
        infos = {"name": "", "version": ""}
        for name in infos.keys():
            section_name = f".app_{name}"
            section = elf.get_section_by_name(section_name)
            if section is None:
                logger.critical(f"failed to get section {section_name}")
                sys.exit(1)
            infos[name] = section.data()

        assert len(infos["name"]) == 32
        assert len(infos["version"]) == 16

        return infos

    @staticmethod
    def _parse_segments(elf: ELFFile) -> List[Segment]:
        segments = list(filter(lambda segment: segment["p_type"] == "PT_LOAD", elf.iter_segments()))
        segments = sorted(segments, key=lambda segment: segment["p_flags"])

        # print([segment.header for segment in segments])

        # ensure there's one code and one data section
        assert len(segments) == 2
        flags = [segment["p_flags"] for segment in segments]

        ro = P_FLAGS.PF_R
        rx = P_FLAGS.PF_R | P_FLAGS.PF_X
        rw = P_FLAGS.PF_R | P_FLAGS.PF_W
        assert sorted(flags) in [[rx, rw], [ro, rx]]

        # ensure that the code segment is the first one
        if segments[0]["p_flags"] & P_FLAGS.PF_X:
            code, data = (segments[0], segments[1])
        else:
            code, data = (segments[1], segments[0])

        return [Segment(code), Segment(data)]

    def get_segment(self, name: str) -> Segment:
        keys = {"code": 0, "data": 1}
        assert name in keys.keys()
        return self.segments[keys[name]]

    def get_section_range(self, name: str) -> Tuple[int, int]:
        segment = self.get_segment(name)
        return segment.start, segment.end
