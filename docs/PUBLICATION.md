# Public snapshot boundaries

This branch was assembled from an explicit selection into a new Git object
database. Its initial commit has no parent from the private research history.
Deleting a file in a later commit would not have removed an older blob.

## Included

- Original authored interoperability findings and limits.
- Original parser/input-audit code and synthetic test constructors.
- Original pure guidance policy, relocated to an independent Java namespace.
- Original pure adapter and custom evidence-envelope codec, with synthetic tests.
- Synthetic observation parameters, source provenance, and publication checks.

## Excluded entirely

- Firmware, dumps, IFS/JXE/JAR/CLASS/ELF/native objects and prebuilt packages.
- Decompiled or reconstructed vendor classes, vendor headers and shadow classes.
- OEM glyphs, skins, atlases, fonts and extracted graphical resources.
- All photographs, videos, screenshots and audio, including embedded metadata.
- Raw traces, vehicle snapshots, route/device/location data and private reports.
- Third-party reference source trees, including GPL and PolyForm references.
- Vehicle installers, deployment scripts, authentication material and lab state.

There are no media files to strip or accidentally retain metadata from. Facts
about an interface and independently authored summaries are distinguished from
the vendor files used privately to investigate it. No redistribution right is
asserted over excluded inputs.

## Checks and scope

The manifest lists the exact approved files and hashes. The audit rejects extra
tracked files, symlinks, non-UTF-8/binary contents, suspicious identifiers,
credentials, personal filesystem paths and prohibited extensions. It can scan
every commit/blob reachable from the branch tip and inspect commit identities.
The same approved content policy applies to historical blobs.

These checks supplement manual source/provenance review. They are not a claim
that a pattern scanner proves all intellectual-property or privacy questions.

Only the public branch is covered. Existing branches/tags already present on
the destination fork are not erased or rewritten by pushing this new branch.
Merging it into a history containing old captures would make those ancestors
reachable again. Keep the research snapshot independent.

To publish this prepared branch, an operator can push its explicit ref to the
existing fork after review. No repository-wide force push, tag rewrite, pull
request or message to another developer is part of preparation.

## Attribution

The original xPaiiN project is credited as project context; its distributed
patches and OEM-adjacent binaries are not included. External alt-screen
repositories are linked for comparison and retain their own licenses.
The original implementation files selected here are listed in PROVENANCE.json.
