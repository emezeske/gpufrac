import os

env = Environment( ENV = { 
    'PATH' : os.environ['PATH'], # These environment variables are required by colorgcc.
    'TERM' : os.environ['TERM'],
    'HOME' : os.environ['HOME']
} )

SetOption( 'num_jobs', 4 ) # Set this to the number of processors you have.  TODO: Automate this.

binary = 'advanced-fractal-explorer'
libraries = [ 'SDL', 'SDL_image', 'GL', 'GLU', 'GLEW', 'm', 'CEGUIBase', 'CEGUIOpenGLRenderer', 'ctemplate' ]
compiler_warnings = '-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings -Wconversion -Winline -Wredundant-decls -Wno-unused'
compiler_flags = '-g ' + compiler_warnings + ' -DNO_SDL_GLEXT'
include_paths = [ 'src', '/usr/include/CEGUI' ]

source_dirs = [ 'src/common', 'src/afe', 'src/afe/generators' ]
sources = [ Glob( os.path.join( dir, '*.cc' ) ) for dir in source_dirs ]
headers = [ Glob( os.path.join( dir, '*.h' ) ) for dir in source_dirs ]

env.Command( 'tags', sources + headers, 'ctags -o $TARGET $SOURCES' )

env.Program( source = sources, target = binary, LIBS = libraries, CCFLAGS = compiler_flags, CPPPATH = include_paths )
