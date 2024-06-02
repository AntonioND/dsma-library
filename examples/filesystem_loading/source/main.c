// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2022-2024

#include <stdbool.h>
#include <stdio.h>

#include <filesystem.h>
#include <nds.h>

#include "dsma/dsma.h"

int file_load(const char *filename, void **buffer, size_t *size_)
{
    FILE *f = fopen(filename, "rb");
    size_t size;

    *buffer = NULL;
    if (size_)
        *size_ = 0;

    if (f == NULL)
    {
        iprintf("%s couldn't be opened!\n", filename);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    if (size_)
        *size_ = size;

    if (size == 0)
    {
        iprintf("Size of %s is 0!\n", filename);
        fclose(f);
        return -1;
    }

    rewind(f);
    *buffer = malloc(size);
    if (*buffer == NULL)
    {
        iprintf("Not enought memory to load %s!\n", filename);
        fclose(f);
        return -1;
    }

    if (fread(*buffer, size, 1, f) != 1)
    {
        iprintf("Error while reading: %s\n", filename);
        fclose(f);
        free(*buffer);
        return -1;
    }

    fclose(f);

    return 0;
}

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

    if (!nitroFSInit(NULL))
    {
        iprintf("nitroFSInit failed.\nPress START to exit");
        while(1)
        {
            swiWaitForVBlank();
            scanKeys();
            if (keysHeld()&KEY_START)
                return 0;
        }
    }

    // Load files from the filesystem
    void *texture128, *dsa_file, *dsm_file;
    size_t texture128_size, dsa_file_size, dsm_file_size;

    int ret = 0;
    ret |= file_load("robot.dsm", &dsm_file, &dsm_file_size);
    ret |= file_load("robot_walk.dsa", &dsa_file, &dsa_file_size);
    ret |= file_load("texture128.bin", &texture128, &texture128_size);
    if (ret)
    {
        iprintf("Press START to exit");
        while(1)
        {
            swiWaitForVBlank();
            scanKeys();
            if (keysHeld()&KEY_START)
                return 0;
        }
    }

    printf("\x1b[20;0HLoaded files:");
    printf("\x1b[21;0HDSM:     %8zu bytes", dsm_file_size);
    printf("\x1b[22;0HDSA:     %8zu bytes", dsa_file_size);
    printf("\x1b[23;0HTexture: %8zu bytes", texture128_size);

    // Load texture
    int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(0, textureID);
    if (glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_128, TEXTURE_SIZE_128, 0,
                     TEXGEN_TEXCOORD, texture128) == 0)
    {
        printf("Failed to load texture");
        while (1)
            swiWaitForVBlank();
    }

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
