import os, glob

SetOption( 'num_jobs', 4 ) # Set this to the number of processors you have.  TODO: Automate this.

libgpufrac_source_dir = 'src/libgpufrac'
libgpufrac_sources = glob.glob( os.path.join( libgpufrac_source_dir, '*.cc' ) )
libgpufrac_headers = glob.glob( os.path.join( libgpufrac_source_dir, '*.h' ) )

env = Environment()
env['ENV'] = {'PATH':os.environ['PATH'], 'TERM':os.environ['TERM'], 'HOME':os.environ['HOME']} # Environment variables required by colorgcc.
env['LIBPATH'] = [ './', '/usr/local/lib' ]
env['CCFLAGS'] = [ '-g', '-Wall', '-W', '-Wshadow', '-Wpointer-arith', '-Wcast-qual', '-Wwrite-strings', '-Wconversion', '-Winline', '-Wredundant-decls', '-Wno-unused', '-Wno-deprecated' ]
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

env.SharedLibrary( source = libgpufrac_sources, target = 'gpufrac' )

env.Command( 'tags', libgpufrac_sources + libgpufrac_headers, 'ctags -o $TARGET $SOURCES' )
