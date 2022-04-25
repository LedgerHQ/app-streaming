#!/usr/bin/env python3

"""
Script to run streamed apps over BLE. This is a quick and dirty PoC since it
serves no other purposes.

The script should be run along bluetoothctl and the device must disconnected:

  $ bluetoothctl --agent
  remove DE:F1:03:1E:30:7B
  scan on
  scan off
  devices
  agent on
  trust DE:F1:03:1E:30:7B
  pair DE:F1:03:1E:30:7B
  connect DE:F1:03:1E:30:7B
  disconnect DE:F1:03:1E:30:7B

Note that the connection to the device isn't closed automatically in
bluetoothctl after exit.

To improve performances drastically:

  echo 6 | sudo tee /sys/kernel/debug/bluetooth/hci0/conn_min_interval
  echo 7 | sudo tee /sys/kernel/debug/bluetooth/hci0/conn_max_interval
"""

import asyncio

from bleak import BleakClient, BleakScanner
from bleak.exc import BleakError
from typing import List

ADDRESS = "DE:F1:03:1E:30:7B"


queue: asyncio.Queue = asyncio.Queue()


def callback(sender: int, data: bytes):
    #print(f"{sender}: {data}")

    response = bytes(data)
    queue.put_nowait(response)


async def wait_for_response():
    ret = await queue.get()
    return ret


async def wait_for_apdu():
    response = await wait_for_response()

    assert len(response) >= 5
    assert response[0] == 0x05
    assert response[1:3] == b"\x00\x00"
    total_size = int.from_bytes(response[3:5], "big")

    apdu = response[5:]
    i = 1
    if len(apdu) < total_size:
        assert total_size > len(response) - 5

        response = await wait_for_response()

        assert len(response) >= 3
        assert response[0] == 0x05
        assert int.from_bytes(response[1:3], "big") == i
        i += 1
        apdu += response[3:]

    assert len(apdu) == total_size

    return apdu


async def list_services(client):
    print("Services:")
    for service in client.services:
        print("[Service] {0}: {1}".format(service.uuid, service.description))
        for char in service.characteristics:
            if "read" in char.properties:
                try:
                    value = bytes(await client.read_gatt_char(char.uuid))
                except Exception as e:
                    value = str(e).encode()
            else:
                value = None
            print(
                "\t[Characteristic] {0}: (Handle: {1}) ({2}) | Name: {3}, Value: {4} ".format(
                    char.uuid,
                    char.handle,
                    ",".join(char.properties),
                    char.description,
                    value,
                )
            )
            for descriptor in char.descriptors:
                value = await client.read_gatt_descriptor(descriptor.handle)
                print(
                    "\t\t[Descriptor] {0}: (Handle: {1}) | Value: {2} ".format(
                        descriptor.uuid, descriptor.handle, bytes(value)
                    )
                )


async def main(ble_address: str):
    device = await BleakScanner.find_device_by_address(ble_address, timeout=1.0)
    if not device:
        raise BleakError(f"A device with address {ble_address} could not be found.")

    # Connect to the device explicitly instead of using the following line which
    # prevents the client from being returner:
    #   with BleakClient(device) as client:
    client = BleakClient(device)
    await client.connect()

    if False:
        await list_services(client)

    # enable notifications
    print("starting notification handler")
    await client.start_notify("13d63400-2c97-0004-0001-4c6564676572", callback)

    print("enabling notifications")
    await asyncio.sleep(0.1)

    print("writing 1 to handle 16")
    await client.write_gatt_char("13d63400-2c97-0004-0002-4c6564676572", bytes.fromhex("0001"))
    await asyncio.sleep(0.1)

    assert await wait_for_response() == b"\x00\x00\x00\x00\x00"

    print("get MTU")
    await client.write_gatt_char("13d63400-2c97-0004-0002-4c6564676572", bytes.fromhex("0800000000"))
    await asyncio.sleep(0.1)

    assert await wait_for_response() == b"\x08\x00\x00\x00\x01\x99"

    return client


async def ble_wait_response():
    return await wait_for_apdu()


async def async_get_client_ble(ble_address: str) -> BleakClient:
    device = await BleakScanner.find_device_by_address(ble_address, timeout=1.0)
    if not device:
        raise BleakError(f"A device with address {ble_address} could not be found.")

    client = BleakClient(device)
    await client.connect()

    # register notification callback
    await client.start_notify("13d63400-2c97-0004-0001-4c6564676572", callback)

    # enable notifications
    await client.write_gatt_char("13d63400-2c97-0004-0002-4c6564676572", bytes.fromhex("0001"))
    assert await wait_for_response() == b"\x00\x00\x00\x00\x00"

    return client


def asyncio_run(coro):
    # asyncio.run doesn't seem to exist on Python 3.6.9 / Bleak 0.14.2
    loop = asyncio.get_event_loop()
    return loop.run_until_complete(coro)


def get_client_ble():
    return asyncio_run(async_get_client_ble(ADDRESS))


async def async_exchange_ble(client: BleakClient, apdu: bytes) -> bytes:
    # https://github.com/LedgerHQ/ledgerjs/blob/9639f96a970e1f46e9e39d0c2c361ce2289923be/packages/devices/src/ble/sendAPDU.ts#L22

    TAG_ID = b"\x05"

    mtu = 0x99
    chunks: List[bytes] = []
    buffer = apdu
    while buffer:
        if not chunks:
            size = 5
        else:
            size = 3
        size = mtu - size
        chunks.append(buffer[:size])
        buffer = buffer[size:]

    for i, chunk in enumerate(chunks):
        header = TAG_ID
        header += i.to_bytes(2, "big")
        if i == 0:
            header += len(apdu).to_bytes(2, "big")
        await client.write_gatt_char("13d63400-2c97-0004-0002-4c6564676572", header + chunk)

    response = await ble_wait_response()

    return response


def exchange_ble(client: BleakClient, data: bytes) -> bytes:
    return asyncio_run(async_exchange_ble(client, data))


if __name__ == "__main__":
    client = asyncio_run(main(ADDRESS))

    print(exchange_ble(client, b"\xE0\x01\x00\x00\x00"))

    print("disconnecting")
    client = asyncio_run(client.disconnect())
