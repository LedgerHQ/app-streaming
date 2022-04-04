from construct import Bytes, Int32ul, Struct
from typing import Type


class Manifest:
    """
    The manifest embeds info required by the VM to execute an app.
    """

    MANIFEST_STRUCT = Struct(
        "name" / Bytes(32),
        "version" / Bytes(16),
        "app_hash" / Bytes(32),
        "entrypoint" / Int32ul,
        "bss" / Int32ul,
        "code_start" / Int32ul,
        "code_end" / Int32ul,
        "stack_start" / Int32ul,
        "stack_end" / Int32ul,
        "data_start" / Int32ul,
        "data_end" / Int32ul,
        "mt_root_hash" / Bytes(32),
        "mt_size" / Int32ul,
        "mt_last_entry" / Bytes(8),
    )

    @classmethod
    def from_binary(cls: Type[object], data: bytes) -> "Manifest":
        assert len(data) == Manifest.MANIFEST_STRUCT.sizeof()
        m = Manifest.MANIFEST_STRUCT.parse(data)

        manifest = cls.__new__(cls)
        manifest.name = m.name
        manifest.version = m.version
        manifest.app_hash = m.app_hash
        manifest.entrypoint = m.entrypoint
        manifest.bss = m.bss
        manifest.code_start = m.code_start
        manifest.code_end = m.code_end
        manifest.stack_start = m.stack_start
        manifest.stack_end = m.stack_end
        manifest.data_start = m.data_start
        manifest.data_end = m.data_end
        manifest.mt_root_hash = m.mt_root_hash
        manifest.mt_size = m.mt_size
        manifest.mt_last_entry = m.mt_last_entry

        return manifest

    def export_binary(self) -> bytes:
        data = Manifest.MANIFEST_STRUCT.build(dict({
            "name": self.name,
            "version": self.version,
            "app_hash": self.app_hash,
            "entrypoint": self.entrypoint,
            "bss": self.bss,
            "code_start": self.code_start,
            "code_end": self.code_end,
            "stack_start": self.stack_start,
            "stack_end": self.stack_end,
            "data_start": self.data_start,
            "data_end": self.data_end,
            "mt_root_hash": self.mt_root_hash,
            "mt_size": self.mt_size,
            "mt_last_entry": self.mt_last_entry,
        }))

        return data

    def __str__(self) -> str:
        return str(Manifest.MANIFEST_STRUCT.parse(self.export_binary()))
