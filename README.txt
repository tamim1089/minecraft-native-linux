Minecraft Console Edition — native Linux build (4J Studios engine, ported)
==========================================================================

RUN IT:
    ./play.sh
  (or:  DISPLAY=:0 ./minecraft_gl )

CONTROLS:
    Play Game / Options / Quit  - click on the title screen
    WASD            - move          Ctrl  - sprint
    Space           - jump / swim   Shift - down (when flying)
    Mouse           - look around   F     - toggle fly / walk
    1-9 or scroll   - pick block    LMB   - break    RMB - place
    Esc             - back to menu

WHAT'S HERE:
    minecraft_gl     the game (self-contained; uses system GL/X11/png libs)
    Common/res/      textures it loads at runtime (terrain, font, gui, mob)
    play.sh          launcher (sets DISPLAY, runs from this folder)

NOTES:
    - The world is deterministic (same seed each launch).
    - Must be run with the working directory = this folder (play.sh handles it).
