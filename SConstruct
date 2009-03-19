import os, glob

SetOption( 'num_jobs', 4 ) # Set this to the number of processors you have.  TODO: Automate this.

libafe_source_dirs = [ 'src/backend', 'src/backend/generators' ]
libafe_sources = [ glob.glob( os.path.join( dir, '*.cc' ) ) for dir in libafe_source_dirs ]
libafe_headers = [ glob.glob( os.path.join( dir, '*.h' ) ) for dir in libafe_source_dirs ]

env = Environment()
env['ENV'] = {'PATH':os.environ['PATH'], 'TERM':os.environ['TERM'], 'HOME':os.environ['HOME']} # Environment varialbes required by colorgcc.
env['LIBPATH'] = [ './', '/usr/local/lib' ]
env['CCFLAGS'] = [ '-g', '-Wall', '-W', '-Wshadow', '-Wpointer-arith', '-Wcast-qual', '-Wwrite-strings', '-Wconversion', '-Winline', '-Wredundant-decls', '-Wno-unused', '-Wno-deprecated', '-msse2', '-mfpmath=sse' ] # '-O3'
env['CPPPATH'] = [ './src' ]
env['LIBS'] = [ 'GLEW', 'm', 'ctemplate', 'boost_python', 'boost_thread' ]
env['SHLIBPREFIX'] = ""

# For people compiling boost themselves:
#env['CPPPATH'] = [ './src', '/usr/local/include/boost-1_36' ]
#env['LIBS'] = [ 'GLEW', 'm', 'ctemplate', 'boost_python-gcc41-mt', 'boost_thread-gcc41-mt' ]

env.ParseConfig( 'python-config --includes' )
env.ParseConfig( 'python-config --ldflags' )

# Old python versions:
#env.ParseConfig( 'python2.4-config --includes' )
#env.ParseConfig( 'python2.4-config --ldflags' )

env.SharedLibrary( source = libafe_sources, target = 'afepy' )

env.Command( 'tags', libafe_sources + libafe_headers, 'ctags -o $TARGET $SOURCES' )
