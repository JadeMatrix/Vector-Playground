#include <cstdlib>
#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>

/*
clang++ -I/usr/local/Cellar/glfw3/3.1.1/include -L/usr/local/Cellar/glfw3/3.1.1/lib -lglfw3 -framework OpenGL gradient-field.cpp -o gradient-field
*/

// Globals etc. ////////////////////////////////////////////////////////////////

#define DIMENSIONS  2

#define VIEW_WIDTH  512
#define VIEW_HEIGHT VIEW_WIDTH

unsigned char texture_data[ VIEW_WIDTH * VIEW_HEIGHT * 3 ];

GLFWwindow* window = NULL;

GLuint texture = 0x00;

// Points //////////////////////////////////////////////////////////////////////

typedef float point[ DIMENSIONS ];

point test_bezier[] = { {  48, 272 },
                        { 176, 400 },
                        { 336, 114 },
                        { 464, 272 } };

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

void getBezierPoint( float t, point curve[ 4 ], point& p )
{
    float t_term_0 = ( 1 - t ) * ( 1 - t ) * ( 1 - t );
    float t_term_1 = 3 * ( 1 - t ) * ( 1 - t ) * t;
    float t_term_2 = 3 * ( 1 - t ) * t * t;
    float t_term_3 = t * t * t;
    
    for( int i = 0; i < DIMENSIONS; ++i )
        p[ i ] =   curve[ 0 ][ i ] * t_term_0
                 + curve[ 1 ][ i ] * t_term_1
                 + curve[ 2 ][ i ] * t_term_2
                 + curve[ 3 ][ i ] * t_term_3;
}

void initTexture()
{
    for( int i = 0; i < VIEW_WIDTH * VIEW_HEIGHT * 3; ++i )
        texture_data[ i ] = 0xFF;
}

void drawBezier()
{
    initTexture();
    
    std::cout << "Texture initialized\n";
    
    glfwMakeContextCurrent( window );
    
    glViewport( 0, 0, VIEW_WIDTH * 2, VIEW_HEIGHT * 2 );
    glLoadIdentity();
    glOrtho( 0.0, VIEW_WIDTH, VIEW_HEIGHT, 0.0, 1.0, -1.0 );
    
    glEnable( GL_TEXTURE_2D );
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );
    
    glColor4f( 1.0, 1.0f, 1.0f, 1.0f );
    
    std::cout << "Initial setup done\n";
    
    point p;
    
    for( int i = 0; i < 256; ++i )
    {
        getBezierPoint( i / 256.0f, test_bezier, p );
        
        int x = p[ 0 ];
        int y = p[ 1 ];
        
        // std::cout << "Got point "
        //           << p[ 0 ]
        //           << ","
        //           << p[ 1 ]
        //           << " (on texture: "
        //           << x
        //           << ","
        //           << y
        //           << ")\n";
        
        int pos = ( y * VIEW_WIDTH + x ) * 3;
        
        texture_data[ pos + 0 ] = 0x00;
        texture_data[ pos + 1 ] = 0x00;
        texture_data[ pos + 2 ] = 0x00;
    }
    
    std::cout << "Wrote curve\n";
    
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D,
                  0,
                  GL_RGB,
                  VIEW_WIDTH,
                  VIEW_HEIGHT,
                  0,
                  GL_RGB,
                  GL_UNSIGNED_BYTE,
                  ( void* )texture_data );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glBindTexture( GL_TEXTURE_2D, 0x00 );
    
    GLenum gl_error = glGetError();
    
    if( gl_error != GL_NO_ERROR )
        std::cout << "Could not create OpenGL texture\n";
    
    std::cout << "Created OpenGL texture\n";
    
    glBindTexture( GL_TEXTURE_2D, texture );
    
    glBegin( GL_QUADS );
    {
        glTexCoord2d( 0, 0 );
        glVertex2i( 0, 0 );
        
        glTexCoord2d( 0, 1.0f );
        glVertex2i( 0, VIEW_HEIGHT );
        
        glTexCoord2d( 1.0f, 1.0f );
        glVertex2i( VIEW_WIDTH, VIEW_HEIGHT );
        
        glTexCoord2d( 1.0f, 0 );
        glVertex2i( VIEW_WIDTH, 0 );
    }
    glEnd();
    
    glBindTexture( GL_TEXTURE_2D, 0x00 );
    
    // DEBUG:
    {
        glColor4f( 1.0f, 0.0f, 0.0f, 0.5f );
        
        glBegin( GL_QUADS );
        {
            glVertex2i( 0, 0 );
            
            glVertex2i( 0, VIEW_HEIGHT );
            
            glVertex2i( VIEW_WIDTH, VIEW_HEIGHT );
            
            glVertex2i( VIEW_WIDTH, 0 );
        }
        glEnd();
        
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    }
    
    std::cout << "Drew texture\n";
    
    glfwSwapBuffers( window );
    
    std::cout << "Swapped buffers\n";
    
    glfwWaitEvents();
    
    std::cout << "Wating...\n";
    
    while( !glfwWindowShouldClose( window ) )
        glfwWaitEvents();
    
    std::cout << "Exiting\n";
}

// Main ////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
    if( !setUpGLFW() )
        return 1;
    
    drawBezier();
    
    glfwTerminate();
    
    return 0;
}


