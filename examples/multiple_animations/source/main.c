#include <stdio.h>

#include <nds.h>

#include "dsma/dsma.h"

// Animated models
#include "robot_dsm_bin.h"

// Animations
#include "robot_dsa_bin.h"
#include "robot_bow_dsa_bin.h"
#include "robot_walk_dsa_bin.h"
#include "robot_wave_dsa_bin.h"

// Textures
#include "texture128_bin.h"

// Static models
#include "ball_bin.h"

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
}

int main(void)
{
    float rotateY = 0.0;
    float rotateZ = 0.0;

    setup_video();

    int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(0, textureID);
    glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_128, TEXTURE_SIZE_128, 0,
                 TEXGEN_TEXCOORD, (u8*)texture128_bin);

    const void *animations[] = {
        robot_wave_dsa_bin,
        robot_bow_dsa_bin,
        robot_walk_dsa_bin,
        robot_dsa_bin,
    };
    const char *names[] = {
        "Wave     ",
        "Bow      ",
        "Walk     ",
        "Base Pose"
    };
    const size_t num_animations = sizeof(animations) / sizeof(animations[0]);
    int curr_animation = 0;

    const void *dsa_file = NULL;
    const void *dsm_file = robot_dsm_bin;

    int32_t frame = 0;

    printf("\x1b[0;0HSwitch animation: X/Y");
    printf("\x1b[1;0HAnimate model: L/R");
    printf("\x1b[2;0HRotate model: Direction pad");
    printf("\x1b[3;0HExit demo: START");

    while(1)
    {
        dsa_file = animations[curr_animation];
        uint32_t num_frames = DSMA_GetNumFrames(dsa_file);

        printf("\x1b[10;0HAnimation:     %s ", names[curr_animation]);
        printf("\x1b[11;0HCurrent frame: %.2f ", f32tofloat(frame));

        glLoadIdentity();

        gluLookAt(8.0, 3.0, 0.0,  // camera possition
                  0.0, 3.0, 0.0,  // look at
                  0.0, 1.0, 0.0); // up

        glPushMatrix();
        {
            glRotateY(rotateY);
            glRotateZ(rotateZ);

            glBindTexture(0, textureID);

            glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 |
                      POLY_FORMAT_LIGHT1 | POLY_FORMAT_LIGHT2 | POLY_FORMAT_LIGHT3);

            cpuStartTiming(0);

            DSMA_DrawModel(dsm_file, dsa_file, frame);

            uint32_t end_time = cpuEndTiming();

            const float us_per_frame = 1000000.0 / 60.0;
            printf("\x1b[19;0HTime       Ticks   us    CPU%%");
            printf("\x1b[20;0HTotal:   %6lu  %4lu  %.3f%%  ",
                   end_time, timerTicks2usec(end_time),
                   100.0 * timerTicks2usec(end_time) / us_per_frame);

            // Wait for geometry engine operations to end
            while (GFX_STATUS & BIT(27));

            printf("\x1b[23;0HPolys: %4d      Vertices: %4d",
                   GFX_POLYGON_RAM_USAGE, GFX_VERTEX_RAM_USAGE);

            // Draw axes of coordinates of the model

            glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

            glBindTexture(0, 0);

            glColor3f(1, 0, 0);
            glVertex3f(0, 0.2, 0);
            glVertex3f(0., -0.2, 0);
            glVertex3f(5, 0, 0);
            glVertex3f(0, 0, 0.2);
            glVertex3f(0., 0, -0.2);
            glVertex3f(5, 0, 0);

            glColor3f(0, 1, 0);
            glVertex3f(0, 0, 0.2);
            glVertex3f(0, 0, -0.2);
            glVertex3f(0, 5, 0);
            glVertex3f(0.2, 0, 0);
            glVertex3f(-0.2, 0, 0);
            glVertex3f(0, 5, 0);

            glColor3f(0, 0, 1);
            glVertex3f(0.2, 0, 0);
            glVertex3f(-0.2, 0, 0);
            glVertex3f(0, 0, 5);
            glVertex3f(0, 0.2, 0);
            glVertex3f(0, -0.2, 0);
            glVertex3f(0, 0, 5);
        }
        glPopMatrix(1);

        // Draw a ball to use as reference for the lights
        glPushMatrix();
        {
            glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 |
                      POLY_FORMAT_LIGHT1 | POLY_FORMAT_LIGHT2 | POLY_FORMAT_LIGHT3);

            glTranslatef(0, 3, 5);
            glCallList((uint32_t *)ball_bin);
        }
        glPopMatrix(1);

        scanKeys();
        u16 keys_held = keysHeld();
        u16 keys_down = keysDown();

        if (keys_down & KEY_X)
        {
            curr_animation++;
            if (curr_animation >= num_animations)
                curr_animation = 0;
            frame = 0;
        }
        if (keys_down & KEY_Y)
        {
            curr_animation--;
            if (curr_animation < 0)
                curr_animation = num_animations - 1;
            frame = 0;
        }

        if (keys_held & KEY_UP)
            rotateZ += 3;
        if (keys_held & KEY_DOWN)
            rotateZ -= 3;
        if (keys_held & KEY_LEFT)
            rotateY -= 3;
        if (keys_held & KEY_RIGHT)
            rotateY += 3;

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
