# Pac Man ING2
 A Pac Man game with a randomly generated maze

## Versioned binaries policy

Runtime SDL binaries are intentionally kept in version control (for example `SDL2.dll`, `SDL2_image.dll`, `SDL2_mixer.dll`, and `SDL2_ttf.dll`, including their vendor copies under `SDL2/`) so the Windows build can run out of the box without requiring a separate SDL runtime installation.

Generated project artifacts are **not** intentionally versioned and are now ignored (for example `Projectp.exe`, `button.o`, `Projectp_private.res`, and `*.layout`).

