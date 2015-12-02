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
typedef point curve[ 4 ];
typedef curve bquad[ 4 ];

curve test_bezier = { {  48, 272 },
                      { 176, 400 },
                      { 336, 114 },
                      { 464, 272 } };

/* 0----1
 * |    |
 * |    |
 * 3----2
 */

bquad test_gradient_field = { { {  4 * 32, 12 * 32 },
                                {  7 * 32, 15 * 32 },
                                {  9 * 32,  9 * 32 },
                                { 12 * 32, 12 * 32 } },
                              
                              { { 12 * 32, 12 * 32 },
                                { 15 * 32,  8 * 32 },
                                { 15 * 32,  7 * 32 },
                                { 12 * 32,  4 * 32 } },
                              
                              { { 12 * 32,  4 * 32 },
                                {  9 * 32,  1 * 32 },
                                {  9 * 32,  4 * 32 },
                                {  4 * 32,  4 * 32 } },
                              
                              { {  4 * 32,  4 * 32 },
                                {  2 * 32,  4 * 32 },
                                {  5 * 32,  9 * 32 },
                                {  4 * 32, 12 * 32 } } };

unsigned char test_gradient_colors[ 4 ][ 3 ] = { { 0xFF, 0x00, 0x00 },
                                                 { 0x00, 0xFF, 0x00 },
                                                 { 0x00, 0x00, 0xFF },
                                                 { 0x00, 0x00, 0x00 } };

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

void getBezierPoint( float t, curve& c, point& p )
{
    float t_term_0 = ( 1 - t ) * ( 1 - t ) * ( 1 - t );
    float t_term_1 = 3 * ( 1 - t ) * ( 1 - t ) * t;
    float t_term_2 = 3 * ( 1 - t ) * t * t;
    float t_term_3 = t * t * t;
    
    for( int i = 0; i < DIMENSIONS; ++i )
        p[ i ] =   c[ 0 ][ i ] * t_term_0
                 + c[ 1 ][ i ] * t_term_1
                 + c[ 2 ][ i ] * t_term_2
                 + c[ 3 ][ i ] * t_term_3;
}

/* Takes a single point in real space and projects its normalized coordinates in
 * the arbitrary space defined by the closed bezier quad, optionally clamping
 * values that lie outside the quad between 0.0 and 1.0
 */
void projectBezierPoint( point& original,
                         bquad quad,
                         point& projected,
                         bool clamp )
{
    // Project coordinates onto the 4-bezier space /////////////////////////////
    
    projected[ 0 ] = original[ 0 ] /= VIEW_WIDTH;
    projected[ 1 ] = original[ 1 ] /= VIEW_HEIGHT;
    
    
    
    // Clamp/clip projected values /////////////////////////////////////////////
    
    if( clamp )
    {
        if( projected[ 0 ] > 1.0f )
            projected[ 0 ] = 1.0f;
        else if( projected[ 0 ] < 0.0f )
            projected[ 0 ] = 0.0f;
        
        if( projected[ 1 ] > 1.0f )
            projected[ 1 ] = 1.0f;
        else if( projected[ 1 ] < 0.0f )
            projected[ 1 ] = 0.0f;
    }
}

void initTexture()
{
    for( int i = 0; i < VIEW_WIDTH * VIEW_HEIGHT * 3; ++i )
        texture_data[ i ] = 0xFF;
}

void drawBezier( curve& c, int steps, unsigned char fill )
{
    for( int i = 0; i < steps; ++i )
        {
            point p;
            
            getBezierPoint( i / ( float )steps, c, p );
            
            int x = p[ 0 ];
            int y = p[ 1 ];
            
            // int pos = ( y * VIEW_WIDTH + x ) * 3;
            // int pos = ( x * VIEW_HEIGHT + y ) * 3;
            int pos = ( y * VIEW_WIDTH + ( VIEW_HEIGHT - 1 - x ) ) * 3;
            
            if( i % 10 )
            {
                texture_data[ pos + 0 ] = fill;
                texture_data[ pos + 1 ] = fill;
                texture_data[ pos + 2 ] = fill;
            }
            else
            {
                texture_data[ pos + 0 ] = ~fill;
                texture_data[ pos + 1 ] = ~fill;
                texture_data[ pos + 2 ] = ~fill;
            }
        }
}

void drawField( int dividers )
{
    // Draw field grid /////////////////////////////////////////////////////////
    
    float divisor = dividers + 1;
    
    for( int i = 1; i <= dividers; ++i )
    {
        float lerp = i / divisor;
        
        curve mn;
        
        getBezierPoint( lerp, test_gradient_field[ 3 ], mn[ 0 ] );
        getBezierPoint( 1 - lerp, test_gradient_field[ 1 ], mn[ 3 ] );
        
        mn[ 1 ][ 0 ] =         lerp   * test_gradient_field[ 0 ][ 1 ][ 0 ]
                       // -                test_gradient_field[ 0 ][ 0 ][ 0 ]
                       + ( 1 - lerp ) * test_gradient_field[ 2 ][ 2 ][ 0 ]
                       // -                test_gradient_field[ 2 ][ 3 ][ 0 ]
                       /*+ mn[ 0 ][ 0 ]*/;
        mn[ 1 ][ 1 ] =         lerp   * test_gradient_field[ 0 ][ 1 ][ 1 ]
                       // -                test_gradient_field[ 0 ][ 0 ][ 1 ]
                       + ( 1 - lerp ) * test_gradient_field[ 2 ][ 2 ][ 1 ]
                       // -                test_gradient_field[ 2 ][ 3 ][ 1 ]
                       /*+ mn[ 0 ][ 1 ]*/;
        
        mn[ 2 ][ 0 ] =         lerp   * test_gradient_field[ 0 ][ 2 ][ 0 ]
                       + ( 1 - lerp ) * test_gradient_field[ 2 ][ 1 ][ 0 ];
        mn[ 2 ][ 1 ] =         lerp   * test_gradient_field[ 0 ][ 2 ][ 1 ]
                       + ( 1 - lerp ) * test_gradient_field[ 2 ][ 1 ][ 1 ];
        
        drawBezier( mn, 256, 0x80 );
    }
    
    for( int i = 1; i <= dividers; ++i )
    {
        float lerp = i / divisor;
        
        curve mn;
        
        getBezierPoint( lerp, test_gradient_field[ 0 ], mn[ 0 ] );
        getBezierPoint( 1 - lerp, test_gradient_field[ 2 ], mn[ 3 ] );
        
        mn[ 1 ][ 0 ] =         lerp   * test_gradient_field[ 1 ][ 1 ][ 0 ]
                       + ( 1 - lerp ) * test_gradient_field[ 3 ][ 2 ][ 0 ];
        mn[ 1 ][ 1 ] =         lerp   * test_gradient_field[ 1 ][ 1 ][ 1 ]
                       + ( 1 - lerp ) * test_gradient_field[ 3 ][ 2 ][ 1 ];
        
        mn[ 2 ][ 0 ] =         lerp   * test_gradient_field[ 1 ][ 2 ][ 0 ]
                       + ( 1 - lerp ) * test_gradient_field[ 3 ][ 1 ][ 0 ];
        mn[ 2 ][ 1 ] =         lerp   * test_gradient_field[ 1 ][ 2 ][ 1 ]
                       + ( 1 - lerp ) * test_gradient_field[ 3 ][ 1 ][ 1 ];
        
        drawBezier( mn, 256, 0x80 );
    }
    
    // Draw field bounds ///////////////////////////////////////////////////////
    
    for( int i = 0; i < 4; ++i )
        drawBezier( test_gradient_field[ i ], 256, 0xFF * ( i % 2 ) );
        // drawBezier( test_gradient_field[ i ], 256, 0x00 );
}

void drawStuff()
{
    // Set up //////////////////////////////////////////////////////////////////
    
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
    
    // Compute gradient ////////////////////////////////////////////////////////
    
    for( int x = 0; x < VIEW_WIDTH; ++x )
        for( int y = 0; y < VIEW_HEIGHT; ++y )
        {
            point original;
            original[ 0 ] = x;
            original[ 1 ] = y;
            
            point projected;
            
            projectBezierPoint( original,
                                test_gradient_field,
                                projected,
                                false );
            
            int pos = ( y * VIEW_WIDTH + x ) * 3;
            
            unsigned char top_grad[ 3 ];
            unsigned char bot_grad[ 3 ];
            
            top_grad[ 0 ] =           projected[ 0 ]   * test_gradient_colors[ 0 ][ 0 ]
                            + ( 1.0 - projected[ 0 ] ) * test_gradient_colors[ 1 ][ 0 ];
            top_grad[ 1 ] =           projected[ 0 ]   * test_gradient_colors[ 0 ][ 1 ]
                            + ( 1.0 - projected[ 0 ] ) * test_gradient_colors[ 1 ][ 1 ];
            top_grad[ 2 ] =           projected[ 0 ]   * test_gradient_colors[ 0 ][ 2 ]
                            + ( 1.0 - projected[ 0 ] ) * test_gradient_colors[ 1 ][ 2 ];
            
            bot_grad[ 0 ] =           projected[ 0 ]   * test_gradient_colors[ 3 ][ 0 ]
                            + ( 1.0 - projected[ 0 ] ) * test_gradient_colors[ 2 ][ 0 ];
            bot_grad[ 1 ] =           projected[ 0 ]   * test_gradient_colors[ 3 ][ 1 ]
                            + ( 1.0 - projected[ 0 ] ) * test_gradient_colors[ 2 ][ 1 ];
            bot_grad[ 2 ] =           projected[ 0 ]   * test_gradient_colors[ 3 ][ 2 ]
                            + ( 1.0 - projected[ 0 ] ) * test_gradient_colors[ 2 ][ 2 ];
            
            texture_data[ pos + 0 ] =           projected[ 1 ]   * top_grad[ 0 ]
                                      + ( 1.0 - projected[ 1 ] ) * bot_grad[ 0 ];
            texture_data[ pos + 1 ] =           projected[ 1 ]   * top_grad[ 1 ]
                                      + ( 1.0 - projected[ 1 ] ) * bot_grad[ 1 ];
            texture_data[ pos + 2 ] =           projected[ 1 ]   * top_grad[ 2 ]
                                      + ( 1.0 - projected[ 1 ] ) * bot_grad[ 2 ];
        }
    
    std::cout << "Computed gradient\n";
    
    // Draw field curves ///////////////////////////////////////////////////////
    
    drawField( 20 );
    
    // Draw ////////////////////////////////////////////////////////////////////
    
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
    
    glBindTexture( GL_TEXTURE_2D, texture );
    
    glBegin( GL_QUADS );
    {
        glTexCoord2d( 0, 0 );
        glVertex2i( VIEW_WIDTH, VIEW_HEIGHT );
        
        glTexCoord2d( 0, 1.0f );
        glVertex2i( VIEW_WIDTH, 0 );
        
        glTexCoord2d( 1.0f, 1.0f );
        glVertex2i( 0, 0 );
        
        glTexCoord2d( 1.0f, 0 );
        glVertex2i( 0, VIEW_HEIGHT );
    }
    glEnd();
    
    glBindTexture( GL_TEXTURE_2D, 0x00 );
    
    glfwSwapBuffers( window );
    
    std::cout << "Waiting for quit...\n";
    
    // Wait for quit/exit //////////////////////////////////////////////////////
    
    glfwWaitEvents();
    
    while( !glfwWindowShouldClose( window ) )
        glfwWaitEvents();
    
    std::cout << "Exiting\n";
}

// Main ////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
    unsigned int seed = time( NULL );
    srand(seed);
    
    if( !setUpGLFW() )
        return 1;
    
    drawStuff();
    
    glfwTerminate();
    
    return 0;
}


