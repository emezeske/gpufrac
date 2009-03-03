#ifndef DRAWING_UTILITIES_H
#define DRAWING_UTILITIES_H

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <string>
#include <map>

#include "geometry.h"

////////////////////////////////////////////////////////////////////////////////
// A simple abstract class to provide a consistent interface for drawable objects.
////////////////////////////////////////////////////////////////////////////////

class Drawable
{
    public:
        virtual ~Drawable() {};

        virtual void draw() = 0;
};

struct ColorRGBA
{
    ColorRGBA();
    ColorRGBA( const float r, const float g, const float b, const float a );

    void setColorGL() const;

    float r_, g_, b_, a_;
};

////////////////////////////////////////////////////////////////////////////////
// The TextureReference class provides the mechanism to load an OpenGL texture
// from almost any file format.  It also provides the interface for reference-
// counting textures, so that any give texture is only loaded once.
//
// TODO: Probably should add a flag to never delete textures unless explicitly
//       requested.
////////////////////////////////////////////////////////////////////////////////

class TextureReference
{
    public:
        static TextureReference &getTexture( const std::string filename );
        static void releaseTexture( const std::string filename );

        TextureReference();
        TextureReference( const std::string filename );
        ~TextureReference();

        void bind() const;
        int sizeX() const { return size_.x_; }
        int sizeY() const { return size_.y_; }
        unsigned incReferenceCount() { return ++referenceCount_; }
        unsigned decReferenceCount() { return --referenceCount_; }

        GLuint getTexture() const;

    private:
        typedef std::map<std::string, TextureReference> FileTextureMap;
        static FileTextureMap loaded_textures;

        unsigned referenceCount_;
        GLuint texture_;
        bool loaded_;
        Vector2Di size_;
};

////////////////////////////////////////////////////////////////////////////////
// The TextureMap class is a wrapper around the TextureReference class.  It
// enforces the reference-counting and provides some simple helper functions for
// binding and drawing textures.
////////////////////////////////////////////////////////////////////////////////

class TextureMap
{
    public:
        TextureMap( const std::string &filename );
        TextureMap( const TextureMap &rhs );
        TextureMap &operator = ( const TextureMap &rhs );
        ~TextureMap();

        void bind() const;
        void rasterize( const Vector2Di &position, const Vector2Di &offset ) const;

        void vertexLL() const { glTexCoord2i( 0, 1 ); }
        void vertexUL() const { glTexCoord2i( 0, 0 ); }
        void vertexUR() const { glTexCoord2i( 1, 0 ); }
        void vertexLR() const { glTexCoord2i( 1, 1 ); }

        const ColorRGBA &getColor() const;
        void setColor( const ColorRGBA &color );

        GLuint getTexture() const;

    private:
        typedef std::map<std::string, Vector2Di> OffsetMap;

        static const std::string 
            TEXTURE_DIRECTORY,
            OFFSETS_FILENAME;

        static OffsetMap offsets;

        static void loadOffsets();
        static Vector2Di getOffset( const std::string &filename );

        std::string filename_;
        TextureReference &tref_;
        Vector2Di offset_;
        ColorRGBA color_;
};

#endif // DRAWING_UTILITIES_H
