# Evidence classification

This publication deliberately distinguishes three kinds of evidence.

## Static target findings

P0468 HMI analysis established function identifiers, relevant service predicates
and field behavior. Firmware fingerprints for independent comparison:

- Original HMI JXE: SHA256
  `302c681717bbfa70432fe4254710fa58e64dfe913e84dd5076bdca4a39b80679`.
- Local analysis JAR: SHA256
  `298d5ad6253a20c093251440f467d1180331ea8071e30482d76e26e17c9cf067`.

These are fingerprints of analysis inputs, not downloadable files, official
firmware signatures, loaded-memory identities, or proof of compatibility with
another firmware. No proprietary input or reconstructed class is included.

The InfoStates/function-38 result was rechecked for this publication. The
larger NavSD findings are research summaries from the same target. Static
facts alone do not establish a changed cluster display.

## Vehicle observations

The project observed real CarPlay guidance and a later synthetic native-guidance
catalogue on one MST2_EU_VW_ZR_P0468T system. The rich catalogue covered 23
ordered states, including intersections, lanes, roundabouts, forks and clearing.

The source recordings and runtime captures remain private. This public branch
provides authored summaries and newly transcribed synthetic parameters, not
the original recordings or a verifiable raw vehicle-evidence package. Readers
cannot independently replay those physical observations from this repository.

Confirmed observations are narrow: specified lane selections were visible,
specified rich turn/fork forms were visible, and final clearing occurred.
Joining/recalculating text, distance accuracy, all roundabout branches and
reliable live route restart remain unresolved or partial.

## Reproducible offline results

The exported C tests construct frames directly from synthetic fields. The Java
test constructs its context and lane events in memory. They exercise parser
bounds, duplicate/narrowing rejection, field presence, lane selection,
generation matching, freshness, unsupported geometry and clear-on-failure.

No vehicle log, recorded road, photograph, proprietary class or generated
vehicle JAR is required to run them. Passing these tests does not validate a
QNX ABI, a live publisher, a video decoder, or physical display behavior.

## Unproved on our P0468

- suppression of the phone-navigation overlay through InfoStates;
- stable restart/recalculation and permanent native-guidance activation;
- received and displayed secondary CarPlay stream 111;
- a visible experimental video pattern on the cluster;
- secondary video decode/presentation and stability after reconnect;
- portability to P0480 or any other firmware/cluster.

The external P0480 video project reports its own results. Do not turn that
report, our host tests, or matching exported symbol names into P0468 video proof.
