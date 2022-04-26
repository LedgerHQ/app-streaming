#!/usr/bin/env python3

"""
Script to run streamed apps over BLE. This is a quick and dirty PoC since it
serves no other purposes.

The script should be ran along bluetoothctl and the device must be disconnected:

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

# 13d63400-2c97-0004-0001-4c6564676572
HANDLE_CHAR_ENABLE_NOTIF = 13

# 13d63400-2c97-0004-0002-4c6564676572
HANDLE_CHAR_WRITE = 16


queue: asyncio.Queue = asyncio.Queue()


async def list_services(client):
    print("Services:")

    for service in client.services:
        print(f"[Service] {service.uuid}: {service.description}")
        for char in service.characteristics:
            if "read" in char.properties:
                try:
                    value = bytes(await client.read_gatt_char(char.uuid))
                except Exception as e:
                    value = str(e).encode()
            else:
                value = None

            properties = ",".join(char.properties)
            print(f"  [Characteristic] {char.uuid}: (Handle: {char.handle}) ({properties}) | "
                  f"Name: {char.description}, Value: {value}")

            for desc in char.descriptors:
                value = await client.read_gatt_descriptor(desc.handle)
                print(f"    [Descriptor] {desc.uuid}: (Handle: {desc.handle}) | Value: {value}")


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


def asyncio_run(coro):
    # asyncio.run doesn't seem to exist on Python 3.6.9 / Bleak 0.14.2
    loop = asyncio.get_event_loop()
    return loop.run_until_complete(coro)


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
        await client.write_gatt_char(HANDLE_CHAR_WRITE, header + chunk)

    response = await wait_for_apdu()

    return response


def exchange_ble(client: BleakClient, data: bytes) -> bytes:
    return asyncio_run(async_exchange_ble(client, data))


async def async_get_client_ble(ble_address: str) -> BleakClient:
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

    # register notification callback
    await client.start_notify(HANDLE_CHAR_ENABLE_NOTIF, callback)

    # enable notifications
    await client.write_gatt_char(HANDLE_CHAR_WRITE, bytes.fromhex("0001"))
    assert await wait_for_response() == b"\x00\x00\x00\x00\x00"

    # confirm that the MTU is 0x99
    await client.write_gatt_char(HANDLE_CHAR_WRITE, bytes.fromhex("0800000000"))
    assert await wait_for_response() == b"\x08\x00\x00\x00\x01\x99"

    return client


def get_client_ble(ble_address=ADDRESS):
    return asyncio_run(async_get_client_ble(ble_address))


def disconnect_ble_client(client: BleakClient):
    asyncio_run(client.disconnect())


if __name__ == "__main__":
    client = get_client_ble(ADDRESS)

    print(exchange_ble(client, b"\xE0\x01\x00\x00\x00"))

    disconnect_ble_client(client)
