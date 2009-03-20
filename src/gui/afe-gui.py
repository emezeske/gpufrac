#!/usr/bin/python

import Image, wx, sys, time, numpy
from wx import xrc
from wx import glcanvas
from OpenGL.GLU import *
from OpenGL.GL import *

sys.path.append( './' )
import afepy

COLORING_METHODS = {
    'Continuous'     : afepy.ColoringMethod.CM_CONTINUOUS,
    'Stepped'        : afepy.ColoringMethod.CM_ITERATIVE,
    'Radius Squared' : afepy.ColoringMethod.CM_RADIUS_SQUARED,
    'Angle'          : afepy.ColoringMethod.CM_ANGLE
}

ESCAPE_CONDITIONS = {
    'Circle' : afepy.EscapeCondition.EC_CIRCLE,
    'Square' : afepy.EscapeCondition.EC_BOX
}

UPDATE_INTERVAL_MS = 10


def get_image_for_opengl( filename ):
    image = Image.open( filename )
    data = afepy.ByteVector()
    # This is not exactly "efficient", but it works.  Maybe just move this into the C++ code and pass in a filename.
    for pixel in image.getdata():
        data.append( chr( pixel[0] ) )
        data.append( chr( pixel[1] ) )
        data.append( chr( pixel[2] ) )
    return ( data, image.size[0], image.size[1] )

def calculate_phase( slider_position ):
    return slider_position / 100.0

def calculate_amplitude( slider_position ):
    return slider_position / 100.0

def calculate_frequency( slider_position ):
    # TODO This is very much the same as the stretch factor calculation below.  Factor out the commonality?
    SLIDER_NEUTRAL = 50.0
    MAX_FACTOR = 25.0
    frequency = slider_position
    if frequency < SLIDER_NEUTRAL:
        frequency = 1.0 / ( ( ( SLIDER_NEUTRAL - frequency ) / SLIDER_NEUTRAL ) * MAX_FACTOR )
    else:
        frequency = 1.0 + ( ( frequency - SLIDER_NEUTRAL ) / SLIDER_NEUTRAL ) * MAX_FACTOR
    return frequency


class JuliaCpuGenerator( object ):

    def __init__( self, fractal_frame, xml_resource ):
        self.fractal_frame = fractal_frame
        self.generator = afepy.JuliaCpu()
        self.frame = xml_resource.LoadFrame( self.fractal_frame, 'julia_cpu_settings_frame' )
        self.frame.Bind( wx.EVT_SLIDER, self.on_max_iterations, id=xrc.XRCID( 'max_iterations' ) )
        self.frame.Show()

    def do_one_step( self, elapsed_time ):
        return

    def draw_fractal( self, width, height, viewport ):
        self.generator.draw( afepy.Vector2Di( width, height ), viewport.position(), viewport.size() )

    def on_max_iterations( self, event ):
        self.generator.set_max_iterations( event.GetInt() )

    def on_fractal_left_down( self, event ):
        return

    def on_fractal_motion( self, event ):
        if event.Dragging():
            x = 2.0 * ( float( event.GetX() ) / self.fractal_frame.width() - 0.5 )
            y = 2.0 * ( float( self.fractal_frame.height() - event.GetY() ) / self.fractal_frame.height() - 0.5 )
            self.generator.set_seed( afepy.Vector2Df( x, y ) )
            event.Skip()

    def on_fractal_key_down( self, event ):
        return

    def on_fractal_key_up( self, event ):
        return


class JuliaShaderGenerator( object ):

    def __init__( self, fractal_frame, xml_resource ):
        self.fractal_frame = fractal_frame
        self.generator = afepy.JuliaShader()
        self.palette_cycle_speed = 0.0
        self.frame = xml_resource.LoadFrame( self.fractal_frame, 'julia_shader_settings_frame' )

        self.seed_real_text_ctrl = xrc.XRCCTRL( self.frame, 'seed_real' )
        self.seed_imag_text_ctrl = xrc.XRCCTRL( self.frame, 'seed_imag' )
        seed = self.generator.get_seed()
        self.set_seed_text_ctrl( seed.x, seed.y )

        self.multisampling_checkbox = xrc.XRCCTRL( self.frame, 'multisampling' )

        self.frame.Bind( wx.EVT_TEXT_ENTER, self.on_seed_real, id=xrc.XRCID( 'seed_real' ) )
        self.frame.Bind( wx.EVT_TEXT_ENTER, self.on_seed_imag, id=xrc.XRCID( 'seed_imag' ) )
        self.frame.Bind( wx.EVT_CHECKBOX, self.on_multisampling, id=xrc.XRCID( 'multisampling' ) )
        self.frame.Bind( wx.EVT_CHECKBOX, self.on_normal_mapping, id=xrc.XRCID( 'normal_mapping' ) )
        self.frame.Bind( wx.EVT_CHOICE, self.on_coloring_method, id=xrc.XRCID( 'coloring_method' ) )
        self.frame.Bind( wx.EVT_CHOICE, self.on_escape_condition, id=xrc.XRCID( 'escape_condition' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_max_iterations, id=xrc.XRCID( 'max_iterations' ) )
        self.frame.Bind( wx.EVT_CHECKBOX, self.on_enable_arbitrary_exponent, id=xrc.XRCID( 'enable_arbitrary_exponent' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_julia_exponent, id=xrc.XRCID( 'julia_exponent' ) )

        self.frame.Bind( wx.EVT_SLIDER, self.on_palette_cycle_speed, id=xrc.XRCID( 'palette_cycle_speed' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_palette_stretch, id=xrc.XRCID( 'palette_stretch' ) )
        self.frame.Bind( wx.EVT_RADIOBUTTON, self.on_palette_mode_texture, id=xrc.XRCID( 'palette_mode_texture' ) )
        self.frame.Bind( wx.EVT_FILEPICKER_CHANGED, self.on_palette_image_file_picker, id=xrc.XRCID( 'palette_image_file_picker' ) )
        self.frame.Bind( wx.EVT_RADIOBUTTON, self.on_palette_mode_magnitude, id=xrc.XRCID( 'palette_mode_magnitude' ) )
        self.frame.Bind( wx.EVT_RADIOBUTTON, self.on_palette_mode_trig, id=xrc.XRCID( 'palette_mode_trig' ) )

        # TODO: There's gotta be some magical Pythonic way to do this without so much repitition:
        self.frame.Bind( wx.EVT_SLIDER, self.on_red_phase, id=xrc.XRCID( 'red_phase' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_green_phase, id=xrc.XRCID( 'green_phase' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_blue_phase, id=xrc.XRCID( 'blue_phase' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_red_amplitude, id=xrc.XRCID( 'red_amplitude' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_green_amplitude, id=xrc.XRCID( 'green_amplitude' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_blue_amplitude, id=xrc.XRCID( 'blue_amplitude' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_red_frequency, id=xrc.XRCID( 'red_frequency' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_green_frequency, id=xrc.XRCID( 'green_frequency' ) )
        self.frame.Bind( wx.EVT_SLIDER, self.on_blue_frequency, id=xrc.XRCID( 'blue_frequency' ) )

        self.frame.Show()

    def do_one_step( self, elapsed_time ):
        self.generator.set_palette_offset( self.generator.get_palette_offset() + self.palette_cycle_speed * elapsed_time )

    def draw_fractal( self, width, height, viewport ):
        self.generator.draw( afepy.Vector2Di( width, height ), viewport.position(), viewport.size() )

    def set_seed_text_ctrl( self, real, imag ):
        self.seed_real_text_ctrl.SetValue( '%.4g' % real )
        self.seed_imag_text_ctrl.SetValue( '%.4g' % imag )

    def on_seed_real( self, event ):
        seed = self.generator.get_seed()
        try:
            seed.x = float( event.GetString() )
        except: return
        self.generator.set_seed( seed )

    def on_seed_imag( self, event ):
        seed = self.generator.get_seed()
        try:
            seed.y = float( event.GetString() )
        except: return
        self.generator.set_seed( seed )

    def on_multisampling( self, event ):
        self.generator.set_multisampling_enabled( event.IsChecked() )

    def on_normal_mapping( self, event ):
        if event.IsChecked():
            # Multisampling is required for normal mapping to work.
            self.multisampling_checkbox.SetValue( True )
            self.generator.set_multisampling_enabled( True )
        self.generator.set_normal_mapping_enabled( event.IsChecked() )

    def on_coloring_method( self, event ):
        self.generator.set_coloring_method( COLORING_METHODS.get( event.GetString() ) )

    def on_escape_condition( self, event ):
        self.generator.set_escape_condition( ESCAPE_CONDITIONS.get( event.GetString() ) )

    def on_max_iterations( self, event ):
        self.generator.set_max_iterations( event.GetInt() )

    def on_enable_arbitrary_exponent( self, event ):
        self.generator.set_arbitrary_exponent_enabled( event.IsChecked() )

    def on_julia_exponent( self, event ):
        self.generator.set_julia_exponent( round( event.GetInt() / 10.0 ) )

    def on_palette_cycle_speed( self, event ):
        self.palette_cycle_speed = event.GetInt()

    def on_palette_stretch( self, event ):
        SLIDER_NEUTRAL = 50.0
        MAX_STRETCH_FACTOR = 25.0
        stretch = event.GetInt()
        if stretch < SLIDER_NEUTRAL:
            stretch = 1.0 / ( ( ( SLIDER_NEUTRAL - stretch ) / SLIDER_NEUTRAL ) * MAX_STRETCH_FACTOR )
        else:
            stretch = 1.0 + ( ( stretch - SLIDER_NEUTRAL ) / SLIDER_NEUTRAL ) * MAX_STRETCH_FACTOR
        self.generator.set_palette_stretch( stretch )

    def on_palette_mode_texture( self, event ):
        self.generator.set_palette_mode( afepy.PaletteMode.PM_TEXTURE )

    def on_palette_image_file_picker( self, event ):
        self.fractal_frame.canvas.SetCurrent()
        data, width, height = get_image_for_opengl( event.GetPath() )
        self.generator.set_palette_texture( data, width, height )

    def on_palette_mode_magnitude( self, event ):
        self.generator.set_palette_mode( afepy.PaletteMode.PM_MAGNITUDE )

    def on_palette_mode_trig( self, event ):
        self.generator.set_palette_mode( afepy.PaletteMode.PM_TRIG )

    def on_red_phase( self, event ):
        self.generator.set_red_phase( calculate_phase( event.GetInt() ) )

    def on_green_phase( self, event ):
        self.generator.set_green_phase( calculate_phase( event.GetInt() ) )

    def on_blue_phase( self, event ):
        self.generator.set_blue_phase( calculate_phase( event.GetInt() ) )

    def on_red_amplitude( self, event ): 
        self.generator.set_red_amplitude( calculate_amplitude( event.GetInt() ) )

    def on_green_amplitude( self, event ):
        self.generator.set_green_amplitude( calculate_amplitude( event.GetInt() ) )

    def on_blue_amplitude( self, event ):
        self.generator.set_blue_amplitude( calculate_amplitude( event.GetInt() ) )

    def on_red_frequency( self, event ): 
        self.generator.set_red_frequency( calculate_frequency( event.GetInt() ) )

    def on_green_frequency( self, event ):
        self.generator.set_green_frequency( calculate_frequency( event.GetInt() ) )

    def on_blue_frequency( self, event ):
        self.generator.set_blue_frequency( calculate_frequency( event.GetInt() ) )

    def on_fractal_left_down( self, event ):
        return

    def on_fractal_motion( self, event ):
        if event.Dragging():
            real = 2.0 * ( float( event.GetX() ) / self.fractal_frame.width() - 0.5 )
            imag = 2.0 * ( float( self.fractal_frame.height() - event.GetY() ) / self.fractal_frame.height() - 0.5 )
            self.set_seed_text_ctrl( real, imag )
            self.generator.set_seed( afepy.Vector2Df( real, imag ) )
            event.Skip()

    def on_fractal_key_down( self, event ):
        return

    def on_fractal_key_up( self, event ):
        return

       
class AfeGlFrame( wx.Frame ):

    def __init__( self, pos, size ):
        super( AfeGlFrame, self ).__init__( None, -1, 'AFE', pos, size, wx.DEFAULT_FRAME_STYLE, 'AFE' )
        self.xml_resource = xrc.XmlResource( 'src/gui/afe-gui.xrc' )
        attributes = ( glcanvas.WX_GL_RGBA, glcanvas.WX_GL_DOUBLEBUFFER )
        self.canvas = glcanvas.GLCanvas( self, attribList=attributes )
        self.gl_initialized = False
        self.viewport = afepy.Viewport( afepy.Vector2Df( -1.0, -1.0 ), afepy.Vector2Df( 2.0, 2.0 ) )
        self.generator_frame = None
        self.depressed_keys = {}
        self.last_timer_event = time.time()
        self.timer = wx.Timer( self )
        self.timer.Start( UPDATE_INTERVAL_MS )
        self.Bind( wx.EVT_TIMER, self.on_timer )
        self.canvas.Bind( wx.EVT_SIZE, self.on_size )
        self.canvas.Bind( wx.EVT_PAINT, self.on_paint )
        self.canvas.Bind( wx.EVT_LEFT_DOWN, self.on_left_down )
        self.canvas.Bind( wx.EVT_MOTION, self.on_motion )
        self.canvas.Bind( wx.EVT_MOUSEWHEEL, self.on_mousewheel )
        self.canvas.Bind( wx.EVT_KEY_DOWN, self.on_key_down )
        self.canvas.Bind( wx.EVT_KEY_UP, self.on_key_up )
        self.Show()
        self.canvas.SetFocus()

    def width( self ):
        return self.canvas.GetClientSize().width

    def height( self ): 
        return self.canvas.GetClientSize().height

    def on_timer( self, event ):
        now = time.time()
        step_time = now - self.last_timer_event
        self.last_timer_event = now
        self.viewport.do_one_step( step_time )
        if self.generator_frame: self.generator_frame.do_one_step( step_time )
        self.canvas.Refresh()

    def on_size( self, event ):
        self.canvas.Show()
        self.canvas.SetCurrent()
        self.init_viewport()
        self.canvas.Refresh( False )
        event.Skip()

    def on_paint( self, event ):
        self.canvas.SetCurrent()
        if not self.gl_initialized:
            self.init_gl()
            self.gl_initialized = True
        if self.generator_frame: self.generator_frame.draw_fractal( self.width(), self.height(), self.viewport )
        self.canvas.SwapBuffers()
        event.Skip()

    def on_left_down( self, event ):
        if self.generator_frame: self.generator_frame.on_fractal_motion( event )

    def on_motion( self, event ):
        if self.generator_frame: self.generator_frame.on_fractal_motion( event )

    def on_mousewheel( self, event ):
        rotation = event.GetWheelRotation()

    def on_key_down( self, event ):
        if event.GetKeyCode() == wx.WXK_ESCAPE:
            sys.exit( 0 )
        elif event.GetKeyCode() in range(0, 256):
            char = chr( event.GetKeyCode() )
            self.depressed_keys[char] = True
            self.update_pan_velocity()
            self.update_zoom_velocity()
        #elif self.generator_frame: self.generator_frame.on_fractal_key_down( event )

    def on_key_up( self, event ):
        if event.GetKeyCode() in range(0, 256):
            char = chr( event.GetKeyCode() )
            self.depressed_keys[char] = False
            self.update_pan_velocity()
            self.update_zoom_velocity()
        #if self.generator_frame: self.generator_frame.on_fractal_key_up( event )

    def update_pan_velocity( self ):
        pan_velocity = afepy.Vector2Df( 0.0, 0.0 )
        if self.depressed_keys.get( 'D' ): pan_velocity.x += 1.0
        if self.depressed_keys.get( 'A' ): pan_velocity.x -= 1.0
        if self.depressed_keys.get( 'W' ): pan_velocity.y += 1.0
        if self.depressed_keys.get( 'S' ): pan_velocity.y -= 1.0
        self.viewport.set_desired_pan_velocity( pan_velocity )

    def update_zoom_velocity( self ):
        zoom_velocity = 0.0
        if self.depressed_keys.get( 'E' ): zoom_velocity += 1.0
        if self.depressed_keys.get( 'Q' ): zoom_velocity -= 1.0
        self.viewport.set_desired_zoom_velocity( zoom_velocity )

    def init_gl( self ):
        glClearColor( 0.0, 0.0, 0.0, 1.0 )
        self.init_viewport()
        afepy.Shader.init()
        self.generator_frame = JuliaShaderGenerator( self, self.xml_resource )
        #self.generator_frame = JuliaCpuGenerator( self, self.xml_resource )

    def init_viewport( self ):
        glViewport( 0, 0, self.width(), self.height() )
        glMatrixMode( GL_PROJECTION )
        glLoadIdentity()
        glOrtho( 0, self.width(), 0, self.height(), 0, 1.0 )



def exception_hook( type, value, traceback ):
    sys.__excepthook__( type, value, traceback )
    sys.exit( 1 )

sys.excepthook = exception_hook

app = wx.App()
frame = AfeGlFrame( wx.DefaultPosition, (512, 512) )
frame.Show()

app.MainLoop()
app.Destroy()
