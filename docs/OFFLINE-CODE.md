# Original offline code

## C parser and strict input audit

`native/p0468-rgi-parser` contains an independently implemented bounded TLV
parser and a host decoder. `native/p0468-c314-input` adds a stricter audit before
the legacy parser: duplicate recognized fields, scalar narrowing, nested lane
structure and pre-truncation element counts are checked.

Use `p0468_c314_parse` and check presence/completeness flags for new work. The
legacy parser alone is retained for compatibility research; its bounded storage
can truncate represented lists. A `bounded` flag is not proof that the sender
described the full physical road topology.

The host decoder hides road/destination text by default. Any use with private
captures must remain local; the command-line option to show text is a debugging
choice, not permission to publish the output.

## Pure Java policy

`java/src/org/mst2research/guidance/C314RichPolicy.java` is the original pure
policy relocated out of the vendor namespace. It uses only standard Java
language/runtime features; no OEM class is supplied or loaded.

It returns decisions and clear instructions. It has no network, file access,
stock class replacement, BAP serializer, publisher, installation or boot hook.
The three arrow templates and conservative lane rules are intentionally narrow.

The test relocation changes only the package declaration. Rebuilding with a
desktop JDK proves host behavior, not class compatibility with the vehicle JVM.
The old private build's vehicle core-JCL dependency is not redistributed.

`C314GuidanceAdapter.java` adds the original pure publication-plan layer. It
distinguishes REFUSED, STATUS, BASELINE and RICH, maintains separate status versus
instruction permission, and always replaces old side/lane layers. Its dedicated
tests exercise transitions and preservation of raw code units including zero.
It still does not publish BAP or decode an OEM class.

`GuidanceDemo` constructs a fully synthetic example and emits JSON for a rich
right turn with two recommended lanes, recalculation, STOP and an invalid-frame
refusal. The test runner validates those plans. The example's true proof flags
describe its synthetic fixture; they must never be copied as transport evidence.

## Original evidence envelope

`native/p0468-c314-envelope` contains the original PRGX/v1 codec. Its byte
layout and limitations are documented in LIFECYCLE-AND-EVIDENCE.md. The synthetic
test builds only the minimum nested payload accepted by that codec, not a full
route or a vendor packet. Normal/sanitizer checks cover output clearing,
checksum, identity, count and evidence-flag constraints.

## Run

```sh
python3 tools/run_tests.py
```

To see the actual synthetic frames, decoder output and four JSON publication
plans as well as the checks:

```sh
python3 tools/run_tests.py --examples
```

`examples/synthetic_rgi.py` constructs a route update, a right-turn definition
and a four-lane event with two recommendations. It can also be run on its own
to generate a small parser-input text fixture. The test runner compiles the
host decoder and verifies that it decodes all three frames while hiding text.
It separately runs `GuidanceDemo`; there is no implicit C-to-Java state bridge
in this example. The lifecycle rules explain the evidence needed to build one.

`CC` can select the C compiler. `JAVA_HOME` can select a JDK. The script fails
if a required compiler/runtime is missing; it does not silently skip a language.
Tests run normally and with ASan/UBSan for C. Outputs go under ignored `build/`.

No model runtime, DGX, QNX installation, firmware file or vehicle connection is
part of this workflow.
