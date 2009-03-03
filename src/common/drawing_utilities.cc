#include <SDL/SDL_image.h> 
#include <fstream>

#include "exceptions.h"
#include "utilities.h"
#include "drawing_utilities.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
// Function definitions for ColorRGBA:
//////////////////////////////////////////////////////////////////////////////////

ColorRGBA::ColorRGBA() :
    r_( 1.0f ), g_( 1.0f ), b_( 1.0f ), a_( 1.0f ) 
{}

ColorRGBA::ColorRGBA( const float r, const float g, const float b, const float a ) :
    r_( r ), g_( g ), b_( b ), a_( a )
{}

void ColorRGBA::setColorGL() const
{
    glColor4f( r_, g_, b_, a_ );
}

//////////////////////////////////////////////////////////////////////////////////
// Static variable definitions for TextureReference:
//////////////////////////////////////////////////////////////////////////////////

TextureReference::FileTextureMap TextureReference::loaded_textures;

//////////////////////////////////////////////////////////////////////////////////
// Static function definitions for TextureReference:
//////////////////////////////////////////////////////////////////////////////////

TextureReference &TextureReference::getTexture( const string filename )
{
    FileTextureMap::iterator t = loaded_textures.find( filename );

    if ( t != loaded_textures.end() )
    {
        t->second.incReferenceCount();
    }
    else loaded_textures[filename] = TextureReference( filename );

    return loaded_textures[filename];
}

void TextureReference::releaseTexture( const string filename )
{
    FileTextureMap::iterator t = loaded_textures.find( filename );

    if ( t != loaded_textures.end() && t->second.decReferenceCount() == 0 )
    {
        loaded_textures.erase( t );
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Function definitions for TextureReference:
//////////////////////////////////////////////////////////////////////////////////

TextureReference::TextureReference() :
    referenceCount_( 0 ),
    texture_( 0 ),
    loaded_( false )
{}

TextureReference::TextureReference( const string filename ) :
    referenceCount_( 1 ),
    texture_( 0 ),
    loaded_( false )
{
    SDL_Surface *surface = IMG_Load( filename.c_str() );
    
    if ( surface )
    {
        // Ensure that the width and height of the image are powers of 2.
        if ( ( surface->w & ( surface->w - 1 ) ) == 0 ) 
        {
            if ( ( surface->h & ( surface->h - 1 ) ) == 0 )
            {
                GLint num_colors = surface->format->BytesPerPixel;
                GLenum texture_format = 0;

                size_.x_ = surface->w;
                size_.y_ = surface->h;
        
                if ( num_colors == 4 ) // Contains an alpha channel.
                {
                    texture_format = ( surface->format->Rmask == 0x000000ff ) ? GL_RGBA : GL_BGRA;
                } 
                else if ( num_colors == 3 ) // No alpha channel.
                {
                    texture_format = ( surface->format->Rmask == 0x000000ff ) ? GL_RGB : GL_BGR;
                } 
        
                if ( texture_format )
                {
                    glGenTextures( 1, &texture_ );
                    glBindTexture( GL_TEXTURE_2D, texture_ );
                    
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); // TODO Make this configurable.
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        
                    if ( SDL_LockSurface( surface ) != -1 )
                    {
                        VNOTIFY( VERBOSE, "Successfully loaded texture #%d (%s).", texture_, filename.c_str() );
                        glTexImage2D( GL_TEXTURE_2D, 0, num_colors, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels );
                        SDL_UnlockSurface( surface );
                        loaded_ = true;
                    }
                    else VNOTIFY( FAULT, "SDL_LockSurface() failed: %s.", SDL_GetError() );
                }
            }
            else VNOTIFY( FAULT, "Width of %d is not a power of 2.", surface->w );
        }
        else VNOTIFY( FAULT, "Height of %d is not a power of 2.", surface->h );
    
        SDL_FreeSurface( surface );
    }
    else VNOTIFY( FAULT, "IMG_Load() failed:: %s.", IMG_GetError() );

    if ( !loaded_ ) throw LoadingError( "Failed to load texture file '" + filename + "'" );
}

void TextureReference::bind() const
{
    if ( loaded_ )
    {
        glBindTexture( GL_TEXTURE_2D, texture_ );
    }
    else NOTIFY( FAULT, "Attempted to bind an unloaded texture." );
}

GLuint TextureReference::getTexture() const
{
    // TODO Probably should throw an exception.
    if ( !loaded_ ) NOTIFY( FAULT, "Attempted to get an unloaded texture." );
    return texture_;
}

TextureReference::~TextureReference()
{ 
    if ( loaded_ && referenceCount_ == 0 )
    {
        VNOTIFY( VERBOSE, "Deleting texture #%d.", texture_ );
        glDeleteTextures( 1, &texture_ );
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Static variable definitions for TextureMap:
//////////////////////////////////////////////////////////////////////////////////

TextureMap::OffsetMap TextureMap::offsets;

const string 
    TextureMap::TEXTURE_DIRECTORY = "gfx/",
    TextureMap::OFFSETS_FILENAME  = TEXTURE_DIRECTORY + "texture_offsets.txt";

//////////////////////////////////////////////////////////////////////////////////
// Static function definitions for TextureMap:
//////////////////////////////////////////////////////////////////////////////////

void TextureMap::loadOffsets()
{
    if ( offsets.empty() )
    {
        ifstream file( OFFSETS_FILENAME.c_str() );

        if ( file.is_open() )
        {
            string texture_name;
            int x, y;

            while ( file >> texture_name >> x >> y )
            {
                VNOTIFY( FAULT, "Loaded offsets for %s: (%d, %d)", texture_name.c_str(), x, y );
                offsets[texture_name] = Vector2Di( x, y );
            }

            if ( !file.eof() ) throw LoadingError( "Error parsing texture offsets file '" + OFFSETS_FILENAME + "'" );
        }
        else throw LoadingError( "Error opening texture offsets file '" + OFFSETS_FILENAME + "'" );
    }
}

Vector2Di TextureMap::getOffset( const string &filename )
{
    Vector2Di offset;

    loadOffsets();

    OffsetMap::const_iterator offset_it = offsets.find( filename );

    if ( offset_it != offsets.end() )
    {
        offset = offset_it->second;
    }
    else throw LoadingError( "Failed to load offsets for texture '" + filename + "'" );

    return offset;
}

//////////////////////////////////////////////////////////////////////////////////
// Function definitions for TextureMap:
//////////////////////////////////////////////////////////////////////////////////

TextureMap::TextureMap( const string &filename ) :
    filename_( TEXTURE_DIRECTORY + filename ),
    tref_( TextureReference::getTexture( filename_ ) ),
    offset_( getOffset( filename ) ),
    color_()
{
}

TextureMap::TextureMap( const TextureMap &rhs ) :
    filename_( rhs.filename_ ),
    tref_( TextureReference::getTexture( filename_ ) ),
    offset_( rhs.offset_ ),
    color_( rhs.color_ )
{
} 

TextureMap &TextureMap::operator = ( const TextureMap &rhs )
{
    filename_ = rhs.filename_;
    tref_ = TextureReference::getTexture( filename_ );
    offset_ = rhs.offset_;
    color_ = rhs.color_;
    return *this;
}

void TextureMap::rasterize( const Vector2Di &position, const Vector2Di &offset ) const
{
    glEnable( GL_TEXTURE_2D );
    tref_.bind();
    color_.setColorGL();
    
    glPushMatrix();
        glTranslatef( static_cast<float>( position.x_ + offset.x_ ), static_cast<float>( position.y_ + offset.y_ ), 0.0f );
        glBegin( GL_QUADS );
            vertexLL();
            glVertex2i( 0,             0             );
            vertexUL();
            glVertex2i( 0,             tref_.sizeY() );
            vertexUR();
            glVertex2i( tref_.sizeX(), tref_.sizeY() );
            vertexLR();
            glVertex2i( tref_.sizeX(), 0             );
        glEnd();
    glPopMatrix();
}

const ColorRGBA &TextureMap::getColor() const
{
    return color_;
}

GLuint TextureMap::getTexture() const
{
    return tref_.getTexture();
}

void TextureMap::setColor( const ColorRGBA &color )
{
    color_ = color;
}

TextureMap::~TextureMap()
{
    TextureReference::releaseTexture( filename_ );
}
