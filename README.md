# MST2 P0468 — public navigation research

Research notes, original offline code and synthetic tests for CarPlay route
guidance on **MST2_EU_VW_ZR_P0468T / MQB Virtual Cockpit**.

This branch shares the findings that are useful to other MST2 developers.
It contains no firmware, vendor classes, decompiled code, graphical resources,
vehicle captures, photographs, videos or installable vehicle packages.

## Start here

- [Detailed developer handoff and reading order](docs/DEVELOPER-HANDOFF.md).
- [Phone-navigation overlay: the InfoStates lead](docs/PHONE-NAVIGATION-OVERLAY.md).
- [Exact P0468 symbols and connected mode/service/InfoStates path](docs/P0468-SYMBOL-MAP.md).
- [Native arrows and lane guidance](docs/NATIVE-GUIDANCE.md).
- [RGI session negotiation](docs/RGI-SESSION-SETUP.md), [wire fields](docs/RGI-WIRE-CONTRACT.md),
  [BAP publication contracts](docs/BAP-NAVSD-CONTRACTS.md),
  and [lifecycle/evidence rules](docs/LIFECYCLE-AND-EVIDENCE.md).
- [All 23 synthetic catalogue inputs and their observation limits](docs/CATALOGUE-23.md).
- [What was observed, tested, or remains unproved](docs/EVIDENCE-AND-LIMITS.md).
- [Portable code and reproduction](docs/OFFLINE-CODE.md).
- [Publication boundaries and clean history](docs/PUBLICATION.md).

The connected static lead for the “navigation is running on the phone” message is
**BAP NavSD function 38, InfoStates, value 6**, selected by mobile route-guidance
activity in the inspected P0468 HMI. This is a static finding and a candidate
boundary to investigate; it is **not a vehicle-tested overlay fix**.

Native arrows/lanes and the secondary video map are separate paths. Our P0468
work has useful native-guidance observations; it has not demonstrated a
secondary CarPlay map on this target.

## Relation to other projects

This research grew alongside [xPaiiN/mst2-carplay-vc](https://github.com/xPaiiN/mst2-carplay-vc).
This publication is an independent research snapshot, not a replacement release
of that project's vehicle patch.

[1mikew/mst2-altscreen-carplay](https://github.com/1mikew/mst2-altscreen-carplay)
reports secondary video on a Skoda P0480 target. At the reviewed revision
`be141a330966adda05515ae03b2423a4b067f179`, its priority blocker is the phone-navigation
overlay; native arrows and lane guidance remain separate roadmap items.
Our P0468 findings are leads for comparison, not proof of P0480 compatibility.
No source from that GPL project or other reference repositories is included here.

## Reproduce offline

Requirements: Python 3, a C99 compiler with ASan/UBSan, and a JDK providing
`java` and `javac` (or set `JAVA_HOME`). No OEM SDK or firmware is required.

```sh
python3 tools/run_tests.py
python3 tools/audit_public.py --history
```

The tests only exercise host code. They do not open a vehicle connection,
publish BAP messages, load QNX code or write an SD card.

## Status

- Strict RGI input audit: host normal/sanitizer coverage, synthetic frames.
- Pure rich-guidance policy and publication-plan adapter: host assertions, no HMI dependencies.
- Native rich arrows and lane catalogue: observed on one P0468 vehicle;
  observations are summarized without redistributing recordings.
- Route restart/recalculation: unresolved; no permanent activation recommended.
- Phone-overlay suppression: candidate investigation only.
- Secondary stream 111 / cluster video on our P0468: not demonstrated.

MIT covers the original source, tests and authored notes in this branch.
See [LICENSE](LICENSE) and [PROVENANCE.json](PROVENANCE.json).

Research on your own equipment, while stationary. Preserve stock behavior and
require an independent recovery path before any future vehicle experiment.
