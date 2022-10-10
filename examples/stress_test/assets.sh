#!/bin/sh

TOOLS=../../tools
QUAD=../../models/one_quad
ROBOT=../../models/robot
WIGGLE=../../models/wiggle

python3 $TOOLS/md5_to_dsma.py \
    --model $QUAD/Quad.md5mesh \
    --name one_quad \
    --output data \
    --texture 128 128 \
    --anim $QUAD/Wiggle.md5anim \
    --skip-frames 1 \
    --bin \
    --blender-fix

python3 $TOOLS/md5_to_dsma.py \
    --model $ROBOT/Robot.md5mesh \
    --name robot \
    --output data \
    --texture 128 128 \
    --anim $ROBOT/Bow.md5anim $ROBOT/Walk.md5anim $ROBOT/Wave.md5anim \
    --skip-frames 1 \
    --bin \
    --blender-fix

python3 $TOOLS/md5_to_dsma.py \
    --model $WIGGLE/Wiggle.md5mesh \
    --name wiggle \
    --output data \
    --texture 128 128 \
    --anim $WIGGLE/Shake.md5anim \
    --skip-frames 1 \
    --bin \
    --blender-fix
