# Contributing

1. Keep application logic independent of board pin numbers.
2. Put board and SDK release differences in `board-support/`.
3. Build with zero warnings where practical.
4. Run every validation item on both supported boards before release.
5. Do not commit generated binaries or local workspace metadata.
6. Record the MCUXpresso IDE and SDK release used for validation in the pull request.
