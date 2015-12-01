#include <cstdlib>
#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>

/*
clang++ -I/usr/local/Cellar/glfw3/3.1.1/include -L/usr/local/Cellar/glfw3/3.1.1/lib -lglfw3 -framework OpenGL gradient-field.cpp -o gradient-field
*/

// Globals etc. ////////////////////////////////////////////////////////////////

#define VIEW_WIDTH  512
#define VIEW_HEIGHT VIEW_WIDTH

unsigned char texture_data[ VIEW_WIDTH * VIEW_HEIGHT ];

GLFWwindow* window = NULL;

GLuint texture = 0x00;

// Functions ///////////////////////////////////////////////////////////////////

bool setUpGLFW()
{
    if( !glfwInit() )
    {
        std::cout << "Could not initialize GLFW\n";
        return false;
    }
    
    if( !( window = glfwCreateWindow( VIEW_WIDTH,
                                      VIEW_HEIGHT,
                                      "gradient-field",
                                      NULL,
                                      NULL ) ) )
    {
        glfwTerminate();
        std::cout << "Could not create window\n";
        return false;
    }
    
    glfwMakeContextCurrent( window );
    
    glGenTextures( 1, &texture );
    if( texture == 0x00 )
    {
        std::cout << "Could not create OpenGL texture\n";
        return false;
    }
    
    return true;
}

// Main ////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
    if( !setUpGLFW() )
        return 1;
    
    // TODO: Stuff
    
    glfwTerminate();
    
    return 0;
}


