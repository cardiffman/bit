#include <GL/gl.h>
#include <GL/freeglut.h>
#include <iostream>
#include <stdint.h>

using std::cout;
using std::endl;
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitContextVersion(2, 1);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow(" 2 D ");

    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glClearColor(0, 0, 0, 1.0);
    glEnable(GL_TEXTURE_2D);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        cout << gluErrorString(error) << endl;
        return 1;
    }
    uint32_t* screen = new uint32_t[SCREEN_WIDTH*SCREEN_HEIGHT];
    for (unsigned y=0; y<SCREEN_HEIGHT; ++y)
        for (unsigned x=0; x<SCREEN_WIDTH; ++x)
        {
            screen[y*SCREEN_WIDTH+x] = ((x & y) ^ 1) ? 0xFF000000 : 0xFFFFFFFF;
        }
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    error = glGetError();
    if( error != GL_NO_ERROR )
    {
        cout << "Error loading texture from " <<screen <<" pixels! " << gluErrorString( error ) << endl;
        return false;
    }    
    return 0;
}