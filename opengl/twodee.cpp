#include <GL/gl.h>
#include <GL/freeglut.h>
#include <iostream>
#include <stdint.h>
#include "engine.h"
#include "scene_builder.h"
#include <cstring>
#include <unistd.h>

using std::cout;
using std::endl;

#define NLOG(A) cout << __FUNCTION__ << ' ' << A << ' ' << __FILE__ << ":" << __LINE__ << endl

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

GLuint texID;

void render()
{
    glClear( GL_COLOR_BUFFER_BIT);
    GLfloat x = (SCREEN_WIDTH - SCREEN_WIDTH) / 2;
    GLfloat y = (SCREEN_HEIGHT - SCREEN_HEIGHT) / 2;
    glLoadIdentity();
    //glTranslatef( x, y, 0);
    glBindTexture(GL_TEXTURE_2D, texID);
    NLOG("Binding " << texID);
        glBegin( GL_QUADS );
            glTexCoord2f( 0.f, 0.f ); glVertex2f(           0.f,            0.f );
            glTexCoord2f( 1.f, 0.f ); glVertex2f( SCREEN_WIDTH,            0.f );
            glTexCoord2f( 1.f, 1.f ); glVertex2f( SCREEN_WIDTH, SCREEN_HEIGHT );
            glTexCoord2f( 0.f, 1.f ); glVertex2f(           0.f, SCREEN_HEIGHT );
        glEnd();
    glutSwapBuffers();
}
int main(int argc, char** argv)
{
	char path[2048];

	readlink("/proc/self/exe",path, sizeof(path));
	char* sl = strrchr(path, '/');
	sl[1] = 0;
    glutInit(&argc, argv);
    glutInitContextVersion(2, 1);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow(" 2 D ");

    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, 1.0, -1.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glClearColor(0, 0, 0, 1.0);
    glEnable(GL_TEXTURE_2D);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        NLOG(gluErrorString(error));
        return 1;
    }
	GraphicsEngine* engine = init_base_engine();
	Scene scene;
	SceneBuilder builder;
    if (argc > 1)
    {
        try {
            builder.parse_containers(path, argv[1], engine);
        } catch (char const * ex) {
            cout << "Exception " << ex << endl;
            return 1;
        } catch (const std::string& ex) {
            cout << "Exception " << ex << endl;
            return 1;
        }
        scene.containers = builder.nc;
        scene.assets = builder.na;
    }
    else
    {
        scene.containers.push_back(Container());
        scene.containers.push_back(Container({{ 0,0,1280,720 },1,0,0,0xFF00FF00}));
        scene.assets.push_back(Asset());
    }
    draw_scene(scene, engine);
	GraphicsBuffer* screenBuffer = engine->getScreenBuffer();
	uint8_t* screen; uint32_t pitch;
	screenBuffer->lock(screen, pitch);
#if 0
    uint32_t* screen = new uint32_t[SCREEN_WIDTH*SCREEN_HEIGHT];
    for (unsigned y=0; y<SCREEN_HEIGHT; ++y)
        for (unsigned x=0; x<SCREEN_WIDTH; ++x)
        {
            screen[y*SCREEN_WIDTH+x] = ((x ^ y) & 1) ? 0xFF000000 : 0xFFFFFFFF;
        }
#endif
    glGenTextures(1, &texID);
    NLOG("Generated " << texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    NLOG("Binding " << texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen);
    NLOG("TEXiMAGE");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    error = glGetError();
    if( error != GL_NO_ERROR )
    {
        NLOG("Error loading texture from " <<screen <<" pixels! " << gluErrorString( error ));
        return false;
    }

    glutDisplayFunc(render);
    glutMainLoop();

    return 0;
}
