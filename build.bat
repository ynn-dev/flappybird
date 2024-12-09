@echo off
cmake --preset=default
cmake --build build
copy spritesheet.png build
copy sfx_die.wav build
copy sfx_hit.wav build
copy sfx_point.wav build
copy sfx_swooshing.wav build
copy sfx_wing.wav build