#!/bin/bash
# Native-Linux Minecraft Console Edition — launcher
cd "$(dirname "$(readlink -f "$0")")"
export DISPLAY="${DISPLAY:-:0}"
exec ./minecraft_gl "$@"
