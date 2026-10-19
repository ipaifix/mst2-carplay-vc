#!/usr/bin/env python3
"""Generate three fictitious parser-input frames; no capture or vehicle I/O."""
import struct


def tlv(identifier, payload):
    return struct.pack('>HH', len(payload) + 4, identifier) + payload


def frame(identifier, fields):
    payload = b''.join(fields)
    return b'@@' + struct.pack('>HH', len(payload) + 6, identifier) + payload


def u16(value):
    return struct.pack('>H', value)


def samples():
    route = frame(0x5201, [tlv(0, u16(16)), tlv(1, b'\x01'),
                          tlv(10, struct.pack('>I', 150)), tlv(13, u16(4)),
                          tlv(16, u16(7)), tlv(18, b'\x01')])
    maneuver = frame(0x5202, [tlv(0, u16(16)), tlv(1, u16(4)),
                             tlv(2, b'Demo turn'), tlv(3, b'\x02'), tlv(9, b'\x00'),
                             tlv(10, struct.pack('>h', -90)), tlv(10, struct.pack('>h', 0)),
                             tlv(11, struct.pack('>h', 90)), tlv(12, u16(7))])
    lanes = [tlv(0, u16(16)), tlv(1, u16(7))]
    for i in range(4):
        # Explicit position, status, and primary; deliberately no other-angle list.
        nested = tlv(0, u16(i)) + tlv(1, bytes([2 if i >= 2 else 0])) + tlv(3, struct.pack('>h', 90))
        lanes.append(tlv(2, nested))
    lanes.append(tlv(3, b'Demo lanes'))
    return route, maneuver, frame(0x5204, lanes)


if __name__ == '__main__':
    for packet in samples():
        print('SYNTHETIC data=' + packet.hex())
