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

#define BEZIER_STEPS 128

unsigned char texture_data[ VIEW_WIDTH * VIEW_HEIGHT * 3 ];

GLFWwindow* window = NULL;

GLuint texture = 0x00;

// Points //////////////////////////////////////////////////////////////////////

typedef double point[ DIMENSIONS ];
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
// bquad test_gradient_field = { { {  4 * 32, 12 * 32 },
//                                 {  4 * 32, 12 * 32 },
//                                 { 12 * 32, 10 * 32 },
//                                 { 12 * 32, 10 * 32 } },
                              
//                               { { 12 * 32, 10 * 32 },
//                                 { 12 * 32, 10 * 32 },
//                                 { 12 * 32,  4 * 32 },
//                                 { 12 * 32,  4 * 32 } },
                              
//                               { { 12 * 32,  4 * 32 },
//                                 { 12 * 32,  4 * 32 },
//                                 {  4 * 32,  4 * 32 },
//                                 {  4 * 32,  4 * 32 } },
                              
//                               { {  4 * 32,  4 * 32 },
//                                 {  4 * 32,  4 * 32 },
//                                 {  4 * 32, 12 * 32 },
//                                 {  4 * 32, 12 * 32 } } };

typedef unsigned char pixel[ 3 ];

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

void initTexture()
{
    for( int i = 0; i < VIEW_WIDTH * VIEW_HEIGHT * 3; ++i )
        texture_data[ i ] = 0xFF;
}

/* Takes a single point in real space and projects its normalized coordinates in
 * the arbitrary space defined by the closed bezier quad, optionally clamping
 * values that lie outside the quad between 0.0 and 1.0
 */
void projectBezierPoint( const point& original,
                         const bquad quad,
                         point& projected,
                         bool clamp )
{
    // Project coordinates onto the 4-bezier space /////////////////////////////
    
    projected[ 0 ] = original[ 0 ];
    projected[ 1 ] = original[ 1 ];
    
    // TODO: Actually implement
    
    // if( projected[ 0 ] > 12 * 32 )
    //     projected[ 0 ] = 12 * 32;
    // else if( projected[ 0 ] < 4 * 32 )
    //     projected[ 0 ] = 4 * 32;
    
    // if( projected[ 1 ] > 12 * 32 )
    //     projected[ 1 ] = 12 * 32;
    // else if( projected[ 1 ] < 4 * 32 )
    //     projected[ 1 ] = 4 * 32;
    
    projected[ 0 ] /= VIEW_WIDTH;
    projected[ 1 ] /= VIEW_HEIGHT;
    
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

void getBezierPoint( double t, const curve& c, point& p )
{
    double t_term_0 = ( 1 - t ) * ( 1 - t ) * ( 1 - t );
    double t_term_1 = 3 * ( 1 - t ) * ( 1 - t ) * t;
    double t_term_2 = 3 * ( 1 - t ) * t * t;
    double t_term_3 = t * t * t;
    
    for( int i = 0; i < DIMENSIONS; ++i )
        p[ i ] =   c[ 0 ][ i ] * t_term_0
                 + c[ 1 ][ i ] * t_term_1
                 + c[ 2 ][ i ] * t_term_2
                 + c[ 3 ][ i ] * t_term_3;
}

void drawBezier( const curve& c,
                 int steps,
                 unsigned char fill,
                 bool handles )
{
    double div = 0xFF;
    
    glColor4f( fill / div, fill / div, fill / div, 1.0f );
    
    glBegin( GL_LINE_STRIP );
    {
        for( int i = 0; i <= steps; ++i )
        {
            point p;
            
            getBezierPoint( i / ( double )steps, c, p );
            
            glColor4f( fill / div, fill / div, fill / div, 1.0f );
            
            glVertex2i( p[ 0 ], VIEW_HEIGHT - p[ 1 ] );
            
            fill = ~fill & 0xFF;
        }
    }
    glEnd();
    
    if( handles )
    {
        glColor4f( 0.0f, 0.0f, 1.0f, 1.0f );
        
        glBegin( GL_LINES );
        {
            glVertex2i( c[ 0 ][ 0 ], VIEW_HEIGHT - c[ 0 ][ 1 ] );
            glVertex2i( c[ 1 ][ 0 ], VIEW_HEIGHT - c[ 1 ][ 1 ] );
            
            glVertex2i( c[ 2 ][ 0 ], VIEW_HEIGHT - c[ 2 ][ 1 ] );
            glVertex2i( c[ 3 ][ 0 ], VIEW_HEIGHT - c[ 3 ][ 1 ] );
        }
        glEnd();
    }
    
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
}

/* Bezier interpolation method 1
 * Simple linear interpolation of handles
 */
void interpolateBezier1( double v,
                         const curve& begin,
                         const curve& end,
                         const curve& from,
                         const curve& to,
                         curve& final )
{
    getBezierPoint(     v, begin, final[ 0 ] );
    getBezierPoint( 1 - v,   end, final[ 3 ] );
    
    final[ 1 ][ 0 ] =         v   * from[ 1 ][ 0 ]
                      + ( 1 - v ) *   to[ 2 ][ 0 ];
    final[ 1 ][ 1 ] =         v   * from[ 1 ][ 1 ]
                      + ( 1 - v ) *   to[ 2 ][ 1 ];
    
    final[ 2 ][ 0 ] =         v   * from[ 2 ][ 0 ]
                      + ( 1 - v ) *   to[ 1 ][ 0 ];
    final[ 2 ][ 1 ] =         v   * from[ 2 ][ 1 ]
                      + ( 1 - v ) *   to[ 1 ][ 1 ];
}
/* Bezier interpolation method 2
 * Linear interpolation of handles, offset to the point they attach to
 */
void interpolateBezier2( double v,
                         const curve& begin,
                         const curve& end,
                         const curve& from,
                         const curve& to,
                         curve& final )
{
    getBezierPoint(     v, begin, final[ 0 ] );
    getBezierPoint( 1 - v,   end, final[ 3 ] );
    
    final[ 1 ][ 0 ] =         v   * (   from[ 1 ][ 0 ]
                                      - from[ 0 ][ 0 ] )
                      + ( 1 - v ) * (     to[ 2 ][ 0 ]
                                      -   to[ 3 ][ 0 ] )
                      + final[ 0 ][ 0 ];
    final[ 1 ][ 1 ] =         v   * (   from[ 1 ][ 1 ]
                                      - from[ 0 ][ 1 ] )
                      + ( 1 - v ) * (     to[ 2 ][ 1 ]
                                      -   to[ 3 ][ 1 ] )
                      + final[ 0 ][ 1 ];
    
    final[ 2 ][ 0 ] =         v   * (   from[ 2 ][ 0 ]
                                      - from[ 3 ][ 0 ] )
                      + ( 1 - v ) * (     to[ 1 ][ 0 ]
                                      -   to[ 0 ][ 0 ] )
                      + final[ 3 ][ 0 ];
    final[ 2 ][ 1 ] =         v   * (   from[ 2 ][ 1 ]
                                      - from[ 3 ][ 1 ] )
                      + ( 1 - v ) * (     to[ 1 ][ 1 ]
                                      -   to[ 0 ][ 1 ] )
                      + final[ 3 ][ 1 ];
}
/* Bezier interpolation method 3
 * 1D bezier interpolation between handles
 */
/* Bezier interpolation method 4
 * 1D bezier interpolation between handles, offset to attached point
 */

void drawField( int dividers )
{
    // Draw field grid /////////////////////////////////////////////////////////
    
    double divisor = dividers + 1;
    
    for( int i = 1; i <= dividers; ++i )
    {
        double lerp = i / divisor;
        
        curve mn;
        
        // interpolateBezier1( lerp,
        //                     test_gradient_field[ 3 ],
        //                     test_gradient_field[ 1 ],
        //                     test_gradient_field[ 0 ],
        //                     test_gradient_field[ 2 ],
        //                     mn );
        // drawBezier( mn, BEZIER_STEPS, 0x00, false );
        
        interpolateBezier2( lerp,
                            test_gradient_field[ 3 ],
                            test_gradient_field[ 1 ],
                            test_gradient_field[ 0 ],
                            test_gradient_field[ 2 ],
                            mn );
        drawBezier( mn, BEZIER_STEPS, 0x80, false );
    }
    
    for( int i = 1; i <= dividers; ++i )
    {
        double lerp = i / divisor;
        
        curve mn;
        
        // interpolateBezier1( lerp,
        //                     test_gradient_field[ 0 ],
        //                     test_gradient_field[ 2 ],
        //                     test_gradient_field[ 1 ],
        //                     test_gradient_field[ 3 ],
        //                     mn );
        // drawBezier( mn, BEZIER_STEPS, 0x00, false );
        
        interpolateBezier2( lerp,
                            test_gradient_field[ 0 ],
                            test_gradient_field[ 2 ],
                            test_gradient_field[ 1 ],
                            test_gradient_field[ 3 ],
                            mn );
        drawBezier( mn, BEZIER_STEPS, 0x80, false );
    }
    
    // Draw field bounds ///////////////////////////////////////////////////////
    
    for( int i = 0; i < 4; ++i )
        // drawBezier( test_gradient_field[ i ], BEZIER_STEPS, 0xFF * ( i % 2 ), true );
        drawBezier( test_gradient_field[ i ], BEZIER_STEPS, 0xFF, false );
}

void computeGradient()
{
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
    
    computeGradient();
    
    std::cout << "Computed gradient\n";
    
    // Draw gradient ///////////////////////////////////////////////////////////
    
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
    
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    
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
    
    // Draw field curves ///////////////////////////////////////////////////////
    
    glLineWidth( 1.0f );
    
    drawField( 20 );
    
    // Wait for quit/exit //////////////////////////////////////////////////////
    
    glfwSwapBuffers( window );
    
    std::cout << "Waiting for quit...\n";
    
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


