// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2022-2024

#include <stdio.h>

#include <nds.h>

#include "dsma/dsma.h"

// Animated models
#include "robot_dsm_bin.h"

// Animations
#include "robot_walk_dsa_bin.h"

// Textures
#include "texture128_bin.h"

void setup_video(void)
{
    videoSetMode(MODE_0_3D);

    vramSetBankA(VRAM_A_TEXTURE);

    consoleDemoInit();

    glInit();

    glEnable(GL_ANTIALIAS);
    glEnable(GL_TEXTURE_2D);

    // Setup the rear plane
    glClearColor(2, 2, 2, 31); // BG must be opaque for AA to work
    glClearPolyID(63); // BG must have a unique polygon ID for AA to work
    glClearDepth(0x7FFF);

    glViewport(0, 0, 255, 191);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70, 256.0 / 192.0, 0.1, 40);

    glLight(0, RGB15(0, 31, 0),                 0,  floattov10(0.8), floattov10(-0.2));
    glLight(1, RGB15(31, 0, 0),                 0, floattov10(-0.8), floattov10(-0.2));
    glLight(2, RGB15(31, 31, 0),  floattov10(0.7),                0, floattov10(-0.7));
    glLight(3, RGB15(0, 0, 31),  floattov10(-0.7),                0, floattov10(-0.7));

    glMaterialf(GL_AMBIENT, RGB15(0, 0, 0));
    glMaterialf(GL_DIFFUSE, RGB15(31, 31, 31));
    glMaterialf(GL_SPECULAR, BIT(15) | RGB15(0, 0, 0));
    glMaterialf(GL_EMISSION, RGB15(0, 0, 0));
    glMaterialShinyness();

    glMatrixMode(GL_MODELVIEW);
    //glMatrixMode(GL_POSITION); // This rotates lights with the model

    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 |
              POLY_FORMAT_LIGHT1 | POLY_FORMAT_LIGHT2 | POLY_FORMAT_LIGHT3);
}

int main(void)
{
    // Basic 3D setup, mostly from libnds examples
    setup_video();

    // Load texture
    int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(0, textureID);
    if (glTexImage2D(0, 0, GL_RGBA, TEXTURE_SIZE_128, TEXTURE_SIZE_128, 0,
                     TEXGEN_TEXCOORD, (u8 *)texture128_bin) == 0)
    {
        printf("Failed to load texture");
        while (1)
            swiWaitForVBlank();
    }

    // Get pointers to the animated model files
    const void *dsa_file = robot_walk_dsa_bin;
    const void *dsm_file = robot_dsm_bin;

    // Obtain number of frames in the animation
    const uint32_t num_frames = DSMA_GetNumFrames(dsa_file);

    printf("\x1b[1;0HAnimate model: L/R");
    printf("\x1b[2;0HRotate model: Direction pad");
    printf("\x1b[3;0HExit demo: START");

    int32_t frame = 0;
    float rotateY = 0.0;
    float rotateZ = 0.0;

    while(1)
    {
        glLoadIdentity();

        gluLookAt(8.0, 3.0, 0.0,  // camera possition
                  0.0, 3.0, 0.0,  // look at
                  0.0, 1.0, 0.0); // up

        // Draw animated model
        glPushMatrix();
        {
            glRotateY(rotateY);
            glRotateZ(rotateZ);

            glBindTexture(0, textureID);

            DSMA_DrawModel(dsm_file, dsa_file, frame);
        }
        glPopMatrix(1);

        scanKeys();
        u16 keys_held = keysHeld();

        if (keys_held & KEY_UP)
            rotateZ += 3;
        if (keys_held & KEY_DOWN)
            rotateZ -= 3;
        if (keys_held & KEY_LEFT)
            rotateY -= 3;
        if (keys_held & KEY_RIGHT)
            rotateY += 3;

        printf("\x1b[11;0HCurrent frame:     %.2f ", f32tofloat(frame));

        if (keys_held & KEY_R)
        {
            frame += 1 << 9;
            if (frame >= (num_frames << 12))
                frame -= num_frames << 12;
        }
        if (keys_held & KEY_L)
        {
            frame -= 1 << 9;
            if (frame < 0)
                frame += num_frames << 12;
        }

        glFlush(0);

        swiWaitForVBlank();

        if (keys_held & KEY_START)
            break;
    }

    return 0;
}
