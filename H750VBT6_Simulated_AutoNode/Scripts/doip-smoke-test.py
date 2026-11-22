#!/usr/bin/env python3

import argparse
import socket
import struct
import sys

DOIP_VERSION = 0x02
DOIP_INVERSE = 0xFD
DOIP_PORT = 13400
DOIP_TESTER_ADDRESS = 0x0E00
DOIP_ENTITY_ADDRESS = 0x0100


def doip_frame(payload_type, payload=b""):
    return struct.pack(">BBHI", DOIP_VERSION, DOIP_INVERSE, payload_type, len(payload)) + payload


def parse_header(data):
    if len(data) != 8:
        raise RuntimeError("short DoIP header")
    version, inverse, payload_type, length = struct.unpack(">BBHI", data)
    if version != DOIP_VERSION or inverse != DOIP_INVERSE:
        raise RuntimeError(f"invalid DoIP header: {version:02X}{inverse:02X}")
    return payload_type, length


def recv_exact(sock, length):
    result = bytearray()
    while len(result) < length:
        chunk = sock.recv(length - len(result))
        if not chunk:
            raise RuntimeError("socket closed")
        result.extend(chunk)
    return bytes(result)


def recv_frame(sock):
    payload_type, length = parse_header(recv_exact(sock, 8))
    return payload_type, recv_exact(sock, length)


def discover(host, timeout):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    try:
        sock.sendto(doip_frame(0x0001), (host, DOIP_PORT))
        data, _ = sock.recvfrom(2048)
        payload_type, length = parse_header(data[:8])
        if payload_type != 0x0004:
            raise RuntimeError(f"unexpected UDP response type 0x{payload_type:04X}")
        payload = data[8:8 + length]
        if len(payload) < 19:
            raise RuntimeError("short vehicle identification response")
        vin = payload[:17].decode("ascii", errors="replace")
        address = struct.unpack(">H", payload[17:19])[0]
        print(f"UDP discovery OK: VIN={vin}, entity=0x{address:04X}")
    finally:
        sock.close()


def activate(sock):
    payload = struct.pack(">HB4s4s",
                          DOIP_TESTER_ADDRESS,
                          0x00,
                          b"\x00" * 4,
                          b"\x00" * 4)
    sock.sendall(doip_frame(0x0005, payload))
    payload_type, response = recv_frame(sock)
    if payload_type != 0x0006 or len(response) < 5:
        raise RuntimeError(f"routing activation response 0x{payload_type:04X}")
    tester, entity, code = struct.unpack(">HHB", response[:5])
    print(f"Routing activation: tester=0x{tester:04X}, entity=0x{entity:04X}, code=0x{code:02X}")
    if code != 0x10:
        raise RuntimeError("routing activation rejected")


def diagnostic(sock, uds, timeout):
    payload = struct.pack(">HH", DOIP_TESTER_ADDRESS, DOIP_ENTITY_ADDRESS) + uds
    sock.sendall(doip_frame(0x8001, payload))

    deadline = timeout
    sock.settimeout(deadline)
    while True:
        payload_type, response = recv_frame(sock)
        if payload_type == 0x8002 and len(response) >= 5:
            print(f"Diagnostic ACK: {response.hex(' ').upper()}")
            continue
        if payload_type == 0x8003 and len(response) >= 5:
            print(f"Diagnostic NACK: {response.hex(' ').upper()}")
            return
        if payload_type == 0x8001 and len(response) >= 4:
            source, target = struct.unpack(">HH", response[:4])
            print(f"Diagnostic response: source=0x{source:04X}, target=0x{target:04X}, "
                  f"UDS={response[4:].hex(' ').upper()}")
            return
        print(f"Unexpected TCP frame: type=0x{payload_type:04X}, payload={response.hex(' ').upper()}")
        return


def main():
    parser = argparse.ArgumentParser(description="H750 DoIP discovery and routing smoke test")
    parser.add_argument("--host", default="192.168.100.30")
    parser.add_argument("--uds", default="3E00", help="UDS request bytes in hex")
    parser.add_argument("--timeout", type=float, default=3.0)
    args = parser.parse_args()

    uds = bytes.fromhex(args.uds.replace(" ", ""))
    discover(args.host, args.timeout)

    sock = socket.create_connection((args.host, DOIP_PORT), timeout=args.timeout)
    try:
        activate(sock)
        diagnostic(sock, uds, args.timeout)
    finally:
        sock.close()

    print("DoIP smoke test completed.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)
