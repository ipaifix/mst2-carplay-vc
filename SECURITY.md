# Experimental research

This branch is for offline research and does not contain a vehicle installer.
Its code does not authorize an experiment on another firmware or cluster.

Any later vehicle implementation must preserve stock navigation, audio and
CarPlay, provide explicit bounded activation and independent recovery, and
qualify disconnect/reconnect, cleanup and restart behavior. Host assertions,
static analysis and a successful API return are separate from physical proof.

Perform vehicle work only while stationary. Do not force-reset or terminate
stock services to satisfy an experiment's preconditions. Keep raw diagnostics
and media private; submit a minimal synthetic reproducer instead.
