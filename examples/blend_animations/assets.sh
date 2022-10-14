#!/bin/sh

TOOLS=../../tools
ROBOT=../../models/robot

python3 $TOOLS/md5_to_dsma.py \
    --model $ROBOT/Robot.md5mesh \
    --name robot \
    --output data \
    --texture 128 128 \
    --anim $ROBOT/Walk.md5anim $ROBOT/Wave.md5anim \
    --skip-frames 1 \
    --bin \
    --blender-fix
