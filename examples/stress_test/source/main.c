#include <stdio.h>

#include <nds.h>

#include "dsma/dsma.h"

// Animated models
#include "one_quad_dsm_bin.h"
#include "robot_dsm_bin.h"
#include "wiggle_dsm_bin.h"

// Animations
#include "one_quad_wiggle_dsa_bin.h"
#include "robot_bow_dsa_bin.h"
#include "robot_walk_dsa_bin.h"
#include "robot_wave_dsa_bin.h"
#include "wiggle_shake_dsa_bin.h"

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
    gluPerspective(90, 256.0 / 192.0, 0.1, 40);

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

#define NUM_ROBOTS  7
#define NUM_QUADS   5
#define NUM_WIGGLES 13

#define NUM_MODELS  (NUM_ROBOTS + NUM_QUADS + NUM_WIGGLES)

struct {
    int32_t x, y, z;
    uint32_t num_frames;
    uint32_t curr_frame_interp;
    uint32_t animation_speed;
    int animation_index;
} model[NUM_MODELS];

const void *robot_dsa_file[] = {
    robot_bow_dsa_bin,
    robot_walk_dsa_bin,
    robot_wave_dsa_bin
};
const size_t robot_dsa_num = sizeof(robot_dsa_file) / sizeof(robot_dsa_file[0]);

void initialize_models(void)
{
    for (int i = 0; i < NUM_MODELS; i++)
    {
        model[i].x = ((i % 5) * inttof32(5)) - inttof32(10);
        model[i].y = 0;
        model[i].z = ((i / 5) * inttof32(5)) - inttof32(10);

        model[i].curr_frame_interp = 0;
        model[i].animation_speed = (((i * 7) % 10) + 8) << 6;
    }

    int index = 0;

    for (int i = 0; i < NUM_ROBOTS; i++)
    {
        int anim_index = i % robot_dsa_num;
        model[index].animation_index = anim_index;
        model[index].num_frames = DSMA_GetNumFrames(robot_dsa_file[anim_index]);
        index++;
    }
    for (int i = 0; i < NUM_QUADS; i++)
    {
        model[index].animation_index = 0;
        model[index].num_frames = DSMA_GetNumFrames(one_quad_wiggle_dsa_bin);
        index++;
    }
    for (int i = 0; i < NUM_WIGGLES; i++)
    {
        model[index].animation_index = 0;
        model[index].num_frames = DSMA_GetNumFrames(wiggle_shake_dsa_bin);
        index++;
    }
}

void draw_and_update_models(void)
{
    int index = 0;

    for (int i = 0; i < NUM_ROBOTS; i++)
    {
        const void *dsm = robot_dsm_bin;
        const void *dsa = robot_dsa_file[model[index].animation_index];
        uint32_t frame = model[index].curr_frame_interp;

        glPushMatrix();

        glTranslatef32(model[index].x, model[index].y, model[index].z);
        DSMA_DrawModel(dsm, dsa, frame);

        glPopMatrix(1);

        index++;
    }
    for (int i = 0; i < NUM_QUADS; i++)
    {
        const void *dsm = one_quad_dsm_bin;
        const void *dsa = one_quad_wiggle_dsa_bin;
        uint32_t frame = model[index].curr_frame_interp;

        glPushMatrix();

        glTranslatef32(model[index].x, model[index].y, model[index].z);
        DSMA_DrawModel(dsm, dsa, frame);

        glPopMatrix(1);

        index++;
    }
    for (int i = 0; i < NUM_WIGGLES; i++)
    {
        const void *dsm = wiggle_dsm_bin;
        const void *dsa = wiggle_shake_dsa_bin;
        uint32_t frame = model[index].curr_frame_interp;

        glPushMatrix();

        glTranslatef32(model[index].x, model[index].y, model[index].z);
        DSMA_DrawModel(dsm, dsa, frame);

        glPopMatrix(1);

        index++;
    }

    for (int i = 0; i < NUM_MODELS; i++)
    {
        model[i].curr_frame_interp += model[i].animation_speed;
        if (model[i].curr_frame_interp >= inttof32(model[i].num_frames))
            model[i].curr_frame_interp -= inttof32(model[i].num_frames);
    }
}

int main(void)
{
    setup_video();

    int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(0, textureID);
    glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_128, TEXTURE_SIZE_128, 0,
                 TEXGEN_TEXCOORD, (u8*)texture128_bin);

    initialize_models();

    iprintf("\x1b[0;0HRotate: Left/Right");
    iprintf("\x1b[1;0HExit demo: START");

    iprintf("\x1b[19;0HTime      Ticks   us    CPU%%");

    float rotationY = 0.0f;

    while(1)
    {
        cpuStartTiming(0);

        glLoadIdentity();

        gluLookAt(10.0, 6.0, 10.0, // camera possition
                  0.0, -6.0, 0.0,   // look at
                  0.0, 1.0, 0.0);  // up

        glBindTexture(0, textureID);

        glRotateY(rotationY);

        draw_and_update_models();

        uint32_t end_time = cpuEndTiming();

        const float us_per_frame = 1000000.0 / 60.0;
        printf("\x1b[20;0H         %6lu  %4lu  %.3f%%  ",
               end_time, timerTicks2usec(end_time),
               100.0 * timerTicks2usec(end_time) / us_per_frame);

        // Wait for geometry engine operations to end
        while (GFX_STATUS & BIT(27));

        printf("\x1b[23;0HPolys: %4d      Vertices: %4d",
               GFX_POLYGON_RAM_USAGE, GFX_VERTEX_RAM_USAGE);

        scanKeys();
        u16 keys_held = keysHeld();

        if (keys_held & KEY_RIGHT)
            rotationY += 3;
        if (keys_held & KEY_LEFT)
            rotationY -= 3;

        glFlush(0);

        swiWaitForVBlank();

        if (keys_held & KEY_START)
            break;
    }

    return 0;
}
