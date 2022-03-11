import logging
import sys

from elftools.elf.elffile import ELFFile
from merkletree import Entry, MerkleTree
from encryption import Encryption


logger = logging.getLogger("elf")


class Segment:
    PAGE_SIZE = 0x00000100
    PAGE_MASK = 0xffffff00
    PAGE_MASK_INVERT = (~PAGE_MASK & 0xffffffff)

    def __init__(self, segment):
        if segment["p_flags"] == 5:
            self.name = "data"
        elif segment["p_flags"] == 6:
            self.name = "code"
        else:
            assert False

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

    def __init__(self, path):
        self.enc = Encryption()
        with open(path, "rb") as fp:
            elf = ELFFile(fp)
            assert elf.get_machine_arch() == "RISC-V"
            self.app_infos = Elf._get_app_infos(elf)
            self.segments = Elf._parse_segments(elf)
            self.entrypoint = elf.header["e_entry"]

    @staticmethod
    def _get_app_infos(elf):
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
    def _parse_segments(elf):
        segments = list(filter(lambda segment: segment["p_type"] == "PT_LOAD", elf.iter_segments()))
        segments = sorted(segments, key=lambda segment: segment["p_flags"])

        #print([segment.header for segment in segments])

        assert len(segments) == 2
        assert segments[0]["p_flags"] != segments[1]["p_flags"]
        flags = [segment["p_flags"] for segment in segments]
        assert flags == [5, 6]

        return [Segment(segment) for segment in segments]

    def _get_segment(self, name):
        keys = {"code": 0, "data": 1}
        assert name in keys.keys()
        return self.segments[keys[name]]

    def get_encrypted_pages(self, name, iv=0):
        segment = self._get_segment(name)
        pages = self.enc.encrypt_segment(segment, Segment.PAGE_SIZE, iv)
        # ensure pages are sorted to compute the merkletree deterministically
        for addr in sorted(pages.keys()):
            (data, digest) = pages[addr]
            yield addr, (data, digest)

    def get_section_range(self, name):
        segment = self._get_segment(name)
        return segment.start, segment.end

    def _compute_initial_merkle_tree(self):
        pages = {}
        merkletree = MerkleTree()
        iv = 0

        for addr, (data, mac) in self.get_encrypted_pages("data"):
            assert addr not in pages
            merkletree.insert(Entry.from_values(addr, iv))

        return merkletree

    def get_manifest(self):
        entrypoint = self.entrypoint

        sdata_start, sdata_end = self.get_section_range("data")
        scode_start, scode_end = self.get_section_range("code")
        app_name = self.app_infos["name"]
        assert len(app_name) == 32

        stack_end = 0x80000000
        stack_start = stack_end - Elf.STACK_SIZE
        bss = sdata_end

        addresses = [
            entrypoint,
            bss,
            scode_start,
            scode_end,
            stack_start,
            stack_end,
            sdata_start,
            sdata_end + Elf.HEAP_SIZE,
        ]

        logger.debug(f"bss:   {bss:#x}")
        logger.debug(f"code:  {scode_start:#x} - {scode_end:#x}")
        logger.debug(f"data:  {sdata_start:#x} - {sdata_end + Elf.HEAP_SIZE:#x}")
        logger.debug(f"stack: {stack_start:#x} - {stack_end:#x}")

        merkletree = self._compute_initial_merkle_tree()

        data = b""
        data += app_name
        data += self.enc.hmac_key
        data += self.enc.encryption_key
        data += b"".join([addr.to_bytes(4, "little") for addr in addresses])
        data += merkletree.root_hash()
        data += len(merkletree.entries).to_bytes(4, "little")
        data += bytes(merkletree.entries[-1])

        return data
