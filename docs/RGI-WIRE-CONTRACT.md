# RGI wire subset implemented by the exported parser

This describes the exported original parser and conservative audit. It is not
a vendor protocol-document copy or a claim to support every iAP2 variant.

## Framing

The parser input starts with six bytes:

```text
offset 0: 0x40 0x40
offset 2: total parser-input length, unsigned 16-bit big-endian
offset 4: message identifier, unsigned 16-bit big-endian
offset 6: first TLV
```

Each TLV has a 16-bit big-endian **total TLV size including its four-byte
header**, then a 16-bit big-endian identifier, then payload. Sizes below four,
truncated headers, or a payload crossing the declared input length are rejected.
This input is not an entire iAP2 link frame; extracting it is a separate,
target-specific transport responsibility.

Recognized scalar payload widths are 1/2/4/8 bytes, big-endian. The strict
wrapper rejects values that narrow beyond the destination field. Component
identifier TLV 0 is exactly two bytes in this subset. Unknown TLVs are skipped
only after their bounds are checked.

## Route update 0x5201: top-level identifiers

- **0:** component identifier, u16.
- **1:** route state, stored u8.
- **2:** maneuver state, stored u8.
- **3 / 4:** current-road / destination text.
- **5 / 6:** ETA / remaining time, stored u64.
- **7 / 8 / 9:** remaining distance u32 / display string / unit u8.
- **10 / 11 / 12:** next-maneuver distance u32 / display string / unit u8.
- **13:** current maneuver-index list, consecutive u16 big-endian elements.
- **14:** maneuver count, stored u16.
- **15:** visibility, stored u8.
- **16 / 17 / 18:** selected lane-event index u16 / lane total u16 /
  lane-display gate u8.
- **19 / 20:** source name text / source-support flag u8.

The bridge's route-state model uses 0 stopped, 1 active guidance, 2 arrival,
3 calculation, 4 localization, 5 recalculation, 6 joining route. These are not
the BAP information-state numbers. Units/epoch of ETA must be established by
the consumer; do not apply a fixed timezone offset as a universal conversion.

## Maneuver definition 0x5202

- **0 / 1:** component / maneuver index, u16.
- **2 / 3:** description text / maneuver type u8.
- **4:** road after maneuver text.
- **5 / 6 / 7:** distance u32 / display string / unit u8.
- **8 / 9:** driving side u8 / junction type u8.
- **10:** one signed-16 junction angle per repeated TLV, exactly two bytes.
- **11:** selected exit angle, represented as signed-16 in this subset.
- **12:** optional linked lane-event index u16.
- **13:** exit-information text.

Useful type values in the tested bridge include 2 right turn, 11 start/joining
instruction, and 13/14 keep-left/right. A name such as keep-left does not by
itself justify a particular native two/three-branch fork descriptor.

## Lane event 0x5204

Top level: **0** component; **1** event index u16; repeated **2** nested lane
information; **3** event description text.

Within a nested lane block:

- **0:** position, one or two bytes; starts the lane representation.
- **1:** status/recommendation, stored u8.
- **2:** nonempty consecutive signed-16 angles in the strict subset.
- **3:** primary/highlighted angle, represented as signed-16.

The strict wrapper requires explicit position before the other recognized lane
fields, rejects repeated status/primary and odd angle-byte counts, and retains
whether each field was actually present. It does not promote an implicit
position or a default angle to received evidence.

The primary/highlighted direction is independent of the optional angles list.
The policy can accept a valid primary-only lane. If an explicit angles list is
present, conflicting sectors remain an error rather than being discarded.
Statuses 0/1/2 are preserved by the conservative policy. Catalogue validation
establishes that 2 marks the selected recommendation in its tested cases;
this publication does not assign a fully qualified visual meaning to 1.

## Bounds and field presence

- Parser text capacity: 96 bytes, including termination; text can be truncated.
- Stored maneuver-index list: at most 16.
- Stored junction angles: at most 16.
- Stored lanes: at most 8; stored angles per lane: at most 16.
- The separate bridge policy admits at most 6 lanes. These are different limits.

The wrapper records counts before truncation and exposes `junction_bounded`,
`current_list_bounded`, and `lane_list_bounded`. Check the corresponding
presence bits as well: a default zero count does not prove an explicit empty
field. The wrapper clears its entire output on rejection.

Presence and bounded storage still do not prove current selection, route
generation, coherence or freshness. Those belong to the producer/consumer
contract in [LIFECYCLE-AND-EVIDENCE.md](LIFECYCLE-AND-EVIDENCE.md).

The exported `test-b26-rgi-parser.c` and `test-c314-input.c` are runnable
synthetic examples of valid and invalid framing, including explicit zero,
negative angles, truncation, duplicate fields and malformed nested lanes.
