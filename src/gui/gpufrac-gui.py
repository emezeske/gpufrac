#!/usr/bin/python

import Image, wx, sys, time, numpy, random, glob, re, os
from wx import xrc
from wx import glcanvas
from OpenGL.GL import *

sys.path.append( './' )
import gpufrac

COLORING_MODES = {
    'Continuous'     : gpufrac.ColoringMode.CM_CONTINUOUS,
    'Stepped'        : gpufrac.ColoringMode.CM_ITERATIVE,
    'Radius Squared' : gpufrac.ColoringMode.CM_RADIUS_SQUARED,
    'Angle'          : gpufrac.ColoringMode.CM_ANGLE
}

ESCAPE_CONDITIONS = {
    'Circle' : gpufrac.EscapeCondition.EC_CIRCLE,
    'Square' : gpufrac.EscapeCondition.EC_BOX
}

MULTISAMPLING_MODES = {
    'None' : gpufrac.MultisamplingMode.MS_NONE,
    '4X'   : gpufrac.MultisamplingMode.MS_4X,
    '8X'   : gpufrac.MultisamplingMode.MS_8X
}

UPDATE_INTERVAL_MS = 10

SCREENSHOT_DIR = 'screenshots'

def get_image_for_opengl( filename, use_alpha ):
    # The efficiency of this function has become a bit of a problem with large (1024x1024+) orbit traps.
    # Figure out a way to optimise it in Python, or just move it into libgpufrac.
    image = Image.open( filename )
    data = gpufrac.ByteVector()
    if use_alpha and not 'A' in image.getbands():
        raise RuntimeError( 'Image file does not have an alpha channel' )
    for pixel in image.getdata():
        data.append( chr( pixel[0] ) )
        data.append( chr( pixel[1] ) )
        data.append( chr( pixel[2] ) )
        if use_alpha:
            data.append( chr( pixel[3] ) )
    return ( data, image.size[0], image.size[1] )

def slider_to_scaling_factor( slider_value, neutral = 50, max_scaling = 25.0, min_scaling = 25.0 ):
    """ 
        Returns a value v such that ( 1 / min_scaling ) < v < max_scaling.
        The neutral position is the slider value that corresponds with 1.0.
    """
    scaling = float( slider_value )
    if scaling < neutral:
        scaling = 1.0 / ( ( ( neutral - scaling ) / neutral ) * min_scaling )
    else:
        scaling = 1.0 + ( ( scaling - neutral ) / neutral ) * max_scaling
    return scaling

class GeneralSettings( object ):
    def __init__( self, parent, generator ):
        self.generator = generator
        self.seed_real_text_ctrl = xrc.XRCCTRL( parent, 'seed_real' )
        self.seed_imag_text_ctrl = xrc.XRCCTRL( parent, 'seed_imag' )
        seed = self.generator.get_seed()
        self.set_seed_text_ctrl( seed.x, seed.y )

        parent.Bind( wx.EVT_TEXT_ENTER, self.on_seed_real, id=xrc.XRCID( 'seed_real' ) )
        parent.Bind( wx.EVT_TEXT_ENTER, self.on_seed_imag, id=xrc.XRCID( 'seed_imag' ) )
        parent.Bind( wx.EVT_CHOICE, self.on_coloring_mode, id=xrc.XRCID( 'coloring_mode' ) )
        parent.Bind( wx.EVT_CHOICE, self.on_escape_condition, id=xrc.XRCID( 'escape_condition' ) )
        parent.Bind( wx.EVT_CHOICE, self.on_multisampling, id=xrc.XRCID( 'multisampling_mode' ) )
        parent.Bind( wx.EVT_CHECKBOX, self.on_mandelbrot_mode, id=xrc.XRCID( 'mandelbrot_mode' ) )
        parent.Bind( wx.EVT_CHECKBOX, self.on_normal_mapping, id=xrc.XRCID( 'normal_mapping' ) )
        parent.Bind( wx.EVT_SLIDER, self.on_height_scale, id=xrc.XRCID( 'height_scale' ) )
        parent.Bind( wx.EVT_SLIDER, self.on_max_iterations, id=xrc.XRCID( 'max_iterations' ) )
        parent.Bind( wx.EVT_CHECKBOX, self.on_enable_arbitrary_exponent, id=xrc.XRCID( 'enable_arbitrary_exponent' ) )
        parent.Bind( wx.EVT_SLIDER, self.on_julia_exponent, id=xrc.XRCID( 'julia_exponent' ) )

    def set_seed_text_ctrl( self, real, imag ):
        self.seed_real_text_ctrl.SetValue( '%.4g' % real )
        self.seed_imag_text_ctrl.SetValue( '%.4g' % imag )

    def on_seed_real( self, event ):
        seed = self.generator.get_seed()
        try:
            seed.x = float( event.GetString() )
        except ValueError: pass
        else: self.generator.set_seed( seed )

    def on_seed_imag( self, event ):
        seed = self.generator.get_seed()
        try:
            seed.y = float( event.GetString() )
        except ValueError: pass
        else: self.generator.set_seed( seed )

    def on_coloring_mode( self, event ):
        self.generator.set_coloring_mode( COLORING_MODES.get( event.GetString() ) )

    def on_escape_condition( self, event ):
        self.generator.set_escape_condition( ESCAPE_CONDITIONS.get( event.GetString() ) )

    def on_multisampling( self, event ):
        self.generator.set_multisampling_mode( MULTISAMPLING_MODES.get( event.GetString() ) )

    def on_mandelbrot_mode( self, event ):
        self.generator.set_mandelbrot_mode_enabled( event.IsChecked() )

    def on_normal_mapping( self, event ):
        self.generator.set_normal_mapping_enabled( event.IsChecked() )

    def on_height_scale( self, event ):
        self.generator.set_height_scale( slider_to_scaling_factor( event.GetInt(), min_scaling = 100.0, max_scaling = 10.0 ) )

    def on_max_iterations( self, event ):
        self.generator.set_max_iterations( event.GetInt() )

    def on_enable_arbitrary_exponent( self, event ):
        self.generator.set_arbitrary_exponent_enabled( event.IsChecked() )

    def on_julia_exponent( self, event ):
        self.generator.set_julia_exponent( round( event.GetInt() / 10.0 ) )

class TrigPaletteSlider( object ):
    def __init__( self, parent, color, setting, generator ):
        self.color, self.setting, self.generator = color, setting, generator
        self.id = self.color + '_' + self.setting
        self.setter = getattr( self.generator, 'set_' + self.id ) 
        self.slider = xrc.XRCCTRL( parent, self.id )
        self.slider.Bind( wx.EVT_SLIDER, self.on_change )

    def on_change( self, event ):
        self.setter( self.scale( event.GetInt() ) )

    def randomize( self ):
        if self.setting == 'phase':
            self.slider.SetValue( random.randint( -314, 314 ) )
        elif self.setting in ( 'amplitude', 'frequency' ):
            self.slider.SetValue( random.randint( 0, 100 ) )
        self.setter( self.scale( self.slider.GetValue() ) )

    def scale( self, slider_value ):
        if self.setting in ( 'phase', 'amplitude' ):
            return slider_value / 100.0
        elif self.setting == 'frequency':
            return slider_to_scaling_factor( slider_value )
    
class TrigPaletteSettings( object ):
    def __init__( self, parent, generator ):
        parent.Bind( wx.EVT_BUTTON, self.on_randomize, id=xrc.XRCID( 'trig_randomize' ) )
        self.sliders = []
        for color in ( 'red', 'green', 'blue' ):
            for setting in ( 'phase', 'amplitude', 'frequency' ):
                self.sliders.append( TrigPaletteSlider( parent, color, setting, generator ) )

    def on_randomize( self, event ):
        for slider in self.sliders:
            slider.randomize()

class PaletteSettings( object ):
    def __init__( self, parent, generator, prepare_gl ):
        self.parent = parent
        self.generator = generator
        self.prepare_gl = prepare_gl
        self.cycle_speed = 0.0
        parent.Bind( wx.EVT_SLIDER, self.on_palette_cycle_speed, id=xrc.XRCID( 'palette_cycle_speed' ) )
        parent.Bind( wx.EVT_SLIDER, self.on_palette_stretch, id=xrc.XRCID( 'palette_stretch' ) )
        parent.Bind( wx.EVT_RADIOBUTTON, self.on_palette_mode_texture, id=xrc.XRCID( 'palette_mode_texture' ) )
        parent.Bind( wx.EVT_FILEPICKER_CHANGED, self.on_palette_image_file_picker, id=xrc.XRCID( 'palette_image_file_picker' ) )
        parent.Bind( wx.EVT_RADIOBUTTON, self.on_palette_mode_orbit_trap, id=xrc.XRCID( 'palette_mode_orbit_trap' ) )
        parent.Bind( wx.EVT_FILEPICKER_CHANGED, self.on_palette_orbit_trap_file_picker, id=xrc.XRCID( 'orbit_trap_image_file_picker' ) )
        parent.Bind( wx.EVT_RADIOBUTTON, self.on_palette_mode_trig, id=xrc.XRCID( 'palette_mode_trig' ) )
        self.trig_settings = TrigPaletteSettings( parent, self.generator )

    def on_palette_cycle_speed( self, event ):
        self.cycle_speed = event.GetInt()

    def on_palette_stretch( self, event ):
        self.generator.set_palette_stretch( slider_to_scaling_factor( event.GetInt() ) )

    def on_palette_mode_texture( self, event ):
        self.generator.set_palette_mode( gpufrac.PaletteMode.PM_TEXTURE )

    def on_palette_image_file_picker( self, event ):
        self.prepare_gl()
        data, width, height = get_image_for_opengl( event.GetPath(), False )
        self.generator.set_palette_texture( data, width, height )

    def on_palette_mode_orbit_trap( self, event ):
        self.generator.set_palette_mode( gpufrac.PaletteMode.PM_ORBIT_TRAP )

    def on_palette_orbit_trap_file_picker( self, event ):
        self.prepare_gl()
        data, width, height = get_image_for_opengl( event.GetPath(), True )
        self.generator.set_orbit_trap_texture( data, width, height )

    def on_palette_mode_trig( self, event ):
        self.generator.set_palette_mode( gpufrac.PaletteMode.PM_TRIG )
    
class FractalShaderGenerator( object ):
    def __init__( self, fractal_frame, xml_resource ):
        self.fractal_frame = fractal_frame
        self.generator = gpufrac.FractalShader()
        self.frame = xml_resource.LoadFrame( self.fractal_frame, 'julia_shader_settings_frame' )
        self.general_settings = GeneralSettings( self.frame, self.generator )
        self.palette_settings = PaletteSettings( self.frame, self.generator, self.fractal_frame.canvas.SetCurrent )
        self.frame.Show()

    def do_one_step( self, elapsed_time ):
        self.generator.set_palette_offset( self.generator.get_palette_offset() + self.palette_settings.cycle_speed * elapsed_time )

    def draw_fractal( self, width, height, viewport ):
        self.generator.draw( gpufrac.Vector2Di( width, height ), viewport.position(), viewport.size() )

    def set_seed( self, real, imag ):
        self.general_settings.set_seed_text_ctrl( real, imag )
        self.generator.set_seed( gpufrac.Vector2Df( real, imag ) )
       
class FractalFrame( wx.Frame ):
    def __init__( self, pos, size ):
        super( FractalFrame, self ).__init__( None, -1, 'gpufrac', pos, size, wx.DEFAULT_FRAME_STYLE, 'gpufrac' )
        self.xml_resource = xrc.XmlResource( 'src/gui/gpufrac-gui.xrc' )
        attributes = ( glcanvas.WX_GL_RGBA, glcanvas.WX_GL_DOUBLEBUFFER )
        self.canvas = glcanvas.GLCanvas( self, attribList=attributes )
        self.gl_initialized = False
        self.viewport = gpufrac.Viewport( gpufrac.Vector2Df( -1.0, -1.0 ), gpufrac.Vector2Df( 2.0, 2.0 ) )
        self.generator_frame = None
        self.left_drag_begin = None
        self.previous_window_size = None
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
        self.handle_depressed_keys()
        self.viewport.do_one_step( step_time )
        if self.generator_frame:
            self.generator_frame.do_one_step( step_time )
        self.canvas.Refresh()

    def on_size( self, event ):
        self.canvas.Show()
        self.canvas.SetCurrent()
        self.set_viewport()
        self.canvas.Refresh( False )

    def on_paint( self, event ):
        self.canvas.SetCurrent()
        if not self.gl_initialized:
            self.init_gl()
            self.gl_initialized = True
        if self.generator_frame:
            self.generator_frame.draw_fractal( self.width(), self.height(), self.viewport )
        self.canvas.SwapBuffers()

    def on_left_down( self, event ):
        self.left_drag_begin = ( event.GetX(), event.GetY() )
        event.Skip()

    def on_motion( self, event ):
        if event.Dragging:
            if event.LeftIsDown():
                assert( self.left_drag_begin )
                begin_real, begin_imag = self.screen_to_complex( self.left_drag_begin[0], self.left_drag_begin[1] )
                end_real, end_imag = self.screen_to_complex( event.GetX(), event.GetY() )
                self.viewport.pan( gpufrac.Vector2Df( begin_real - end_real, begin_imag - end_imag ) )
                self.left_drag_begin = ( event.GetX(), event.GetY() )
            if event.RightIsDown() and self.generator_frame:
                real = 2.0 * ( float( event.GetX() ) / self.width() - 0.5 )
                imag = 2.0 * ( float( self.height() - event.GetY() ) / self.height() - 0.5 )
                self.generator_frame.set_seed( real, imag )
                self.right_drag_begin = ( event.GetX(), event.GetY() )
            event.Skip()

    def on_mousewheel( self, event ):
        ZOOM_STEP = 0.05
        rotation = float( event.GetWheelRotation() ) / event.GetWheelDelta()
        zoom_factor = rotation * ZOOM_STEP
        locus = self.screen_to_complex( event.GetX(), event.GetY() )
        self.viewport.zoom( zoom_factor, gpufrac.Vector2Df( locus[0], locus[1] ) )
        event.Skip()

    def on_key_down( self, event ):
        if event.GetKeyCode() == wx.WXK_ESCAPE:
            sys.exit( 0 )
        elif event.GetKeyCode() == wx.WXK_TAB:
            self.ShowFullScreen( not self.IsFullScreen() )
            event.Skip()
        elif event.GetKeyCode() == wx.WXK_SPACE:
            self.take_screenshot()
            event.Skip()

    def handle_depressed_keys( self ):
        # EVT_KEY_DOWN and EVT_KEY_UP cannot be used to detect keys that are intended to be held down, because
        # the system's key repeat setting may result in them being called repeatedly while the key is depressed.
        pan_velocity = gpufrac.Vector2Df( 0.0, 0.0 )
        if wx.GetKeyState( ord( 'w' ) ): pan_velocity.y += 1.0
        if wx.GetKeyState( ord( 's' ) ): pan_velocity.y -= 1.0
        if wx.GetKeyState( ord( 'd' ) ): pan_velocity.x += 1.0
        if wx.GetKeyState( ord( 'a' ) ): pan_velocity.x -= 1.0
        self.viewport.set_desired_pan_velocity( pan_velocity )

        zoom_velocity = 0.0
        if wx.GetKeyState( ord( 'e' ) ): zoom_velocity += 1.0
        if wx.GetKeyState( ord( 'q' ) ): zoom_velocity -= 1.0
        self.viewport.set_desired_zoom_velocity( zoom_velocity )

    def take_screenshot( self ):
        context = wx.ClientDC( self )
        memory = wx.MemoryDC()
        x, y = self.GetClientSizeTuple()
        bitmap = wx.EmptyBitmap( x, y, -1 )
        memory.SelectObject( bitmap )
        memory.Blit( 0, 0, x, y, context, 0, 0 )
        memory.SelectObject( wx.NullBitmap )

        if os.path.exists( SCREENSHOT_DIR ):
            if not os.path.isdir( SCREENSHOT_DIR ):
                raise OSError( "Can't save screenshot to '%s': something is in the way." % SCREENSHOT_DIR )
        else:
            os.mkdir( SCREENSHOT_DIR )

        filename_id = 0
        existing_files = glob.glob( SCREENSHOT_DIR + '/gpufrac-*.png' )
        if existing_files: 
            matches = [ re.search( '([0-9]+)', file ) for file in existing_files ]
            ids = [ int( match.group(1) ) for match in matches if match ]
            if ids: filename_id = max( ids ) + 1

        filename = SCREENSHOT_DIR + '/gpufrac-%d.png' % filename_id
        print 'Saving to', filename
        bitmap.SaveFile( filename, wx.BITMAP_TYPE_PNG )

    def remember_window_size( self ):
        self.previous_window_size = ( self.width(), self.height() )

    def init_gl( self ):
        glClearColor( 0.0, 0.0, 0.0, 1.0 )
        self.set_viewport()
        gpufrac.Shader.init()
        self.generator_frame = FractalShaderGenerator( self, self.xml_resource )

    def set_viewport( self ):
        glViewport( 0, 0, self.width(), self.height() )
        glMatrixMode( GL_PROJECTION )
        glLoadIdentity()
        glOrtho( 0, self.width(), 0, self.height(), 0, 1.0 )
        if self.previous_window_size is not None:
            xscale = float( self.width() ) / self.previous_window_size[0]
            yscale = float( self.height() ) / self.previous_window_size[1]
            self.viewport.scale_extents( gpufrac.Vector2Df( xscale, yscale ) )
        self.remember_window_size()

    def screen_to_complex( self, screen_x, screen_y ):
        real = self.viewport.position().x + self.viewport.size().x * ( float( screen_x ) / self.width() )
        imag = self.viewport.position().y + self.viewport.size().y * ( float( self.height() - screen_y ) / self.height() )
        return real, imag

def exception_hook( type, value, traceback ):
    sys.__excepthook__( type, value, traceback )
    sys.exit( 1 )

sys.excepthook = exception_hook

app = wx.App()
frame = FractalFrame( wx.DefaultPosition, ( 512, 512 ) )
frame.Show()

app.MainLoop()
app.Destroy()
