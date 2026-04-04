# Pac Man ING2
A Pac Man game with a randomly generated maze.

## Versioning
This repository uses a canonical `VERSION` file at the repository root with semantic version format: `MAJOR.MINOR.PATCH`.

Build metadata is synchronized from `VERSION` via:
- `Makefile.win` compile definition (`APP_VERSION`) and generated header `include/version.h`.
- `Projectp.dev` `[VersionInfo]` metadata and `VersionSyncFile=VERSION` note.

The game exposes the version:
- In startup logs (`Pac Man ING2 version X.Y.Z`).
- In the menu footer (`vX.Y.Z`).

## Release process
1. Bump `VERSION` (for example from `1.0.0` to `1.1.0`).
2. Regenerate/rebuild so `include/version.h` and project metadata are aligned.
3. Generate or update `CHANGELOG.md` with notable changes.
4. Commit release changes.
5. Create and push a Git tag named `vX.Y.Z` (matching `VERSION`).
6. Publish release notes from the changelog.
