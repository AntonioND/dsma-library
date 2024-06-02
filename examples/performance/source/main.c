// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2022-2024

#include <stdio.h>

#include <nds.h>

#include "dsma/dsma.h"

// Animated models
#include "one_quad_dsm_bin.h"
#include "robot_dsm_bin.h"

// Animations
#include "one_quad_wiggle_dsa_bin.h"
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

    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 |
              POLY_FORMAT_LIGHT1 | POLY_FORMAT_LIGHT2 | POLY_FORMAT_LIGHT3);
}

int main(void)
{
    setup_video();

    int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(0, textureID);
    if (glTexImage2D(0, 0, GL_RGBA, 128, 128, 0, TEXGEN_TEXCOORD, texture128_bin) == 0)
    {
        printf("Failed to load texture");
        while (1)
            swiWaitForVBlank();
    }

    const void *robot_dsa_file = robot_walk_dsa_bin;
    const void *robot_dsm_file = robot_dsm_bin;
    const uint32_t robot_num_frames = DSMA_GetNumFrames(robot_dsa_file);
    int32_t robot_frame = 0;

    const void *quad_dsa_file = one_quad_wiggle_dsa_bin;
    const void *quad_dsm_file = one_quad_dsm_bin;
    const uint32_t quad_num_frames = DSMA_GetNumFrames(quad_dsa_file);
    int32_t quad_frame = 0;

    printf("\x1b[0;0HExit demo: START");

    printf("\x1b[4;0H"
           "This sample shows two models\n"
           "side by side.\n"
           "\n"
           "The robot has aprox. 550 polys\n"
           "and 16 bones. It's an example\n"
           "of a regular model\n."
           "\n"
           "The quad has 2 polys and 7\n"
           "bones. It's an example of how\n"
           "much CPU time the skeleton\n"
           "calculations take.\n");

    printf("\x1b[19;0HTime       Ticks   us    CPU%%");

    while(1)
    {
        glLoadIdentity();

        gluLookAt(8.0, 3.0, 0.0,  // camera possition
                  0.0, 3.0, 0.0,  // look at
                  0.0, 1.0, 0.0); // up

        glPushMatrix();
        {
            glTranslatef(0, 0, -3);

            glBindTexture(0, textureID);

            cpuStartTiming(0);

            DSMA_DrawModel(robot_dsm_file, robot_dsa_file, robot_frame);

            uint32_t end_time = cpuEndTiming();

            const float us_per_frame = 1000000.0 / 60.0;
            printf("\x1b[20;0HRobot:   %6lu  %4lu  %.3f%%  ",
                   end_time, timerTicks2usec(end_time),
                   100.0 * timerTicks2usec(end_time) / us_per_frame);
        }
        glPopMatrix(1);

        glPushMatrix();
        {
            glTranslatef(0, 0, 3);
            glRotateY(-90);

            glBindTexture(0, 0);

            cpuStartTiming(0);

            DSMA_DrawModel(quad_dsm_file, quad_dsa_file, quad_frame);

            uint32_t end_time = cpuEndTiming();

            const float us_per_frame = 1000000.0 / 60.0;
            printf("\x1b[21;0HQuad:    %6lu  %4lu  %.3f%%  ",
                   end_time, timerTicks2usec(end_time),
                   100.0 * timerTicks2usec(end_time) / us_per_frame);
        }
        glPopMatrix(1);


            // Wait for geometry engine operations to end
            while (GFX_STATUS & BIT(27));

            printf("\x1b[23;0HPolys: %4d      Vertices: %4d",
                   GFX_POLYGON_RAM_USAGE, GFX_VERTEX_RAM_USAGE);

        scanKeys();
        u16 keys_held = keysHeld();

        robot_frame += 1 << 9;
        if (robot_frame >= inttof32(robot_num_frames))
            robot_frame -= inttof32(robot_num_frames);

        quad_frame += 1 << 9;
        if (quad_frame >= inttof32(quad_num_frames))
            quad_frame -= inttof32(quad_num_frames);

        glFlush(0);

        swiWaitForVBlank();

        if (keys_held & KEY_START)
            break;
    }

    return 0;
}
