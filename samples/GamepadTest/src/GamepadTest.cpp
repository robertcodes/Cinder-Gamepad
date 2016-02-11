#include "cinder/app/App.h"
#include "cinder/Log.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Text.h"

// Gamepad block is a C library, so must use extern "C" to include from C++
extern "C" {
#include "Gamepad.h"
}

using namespace ci;
using namespace ci::app;
using namespace std;

static bool verbose = false;
static const bool PREMULT = false;

// We'll create a new Cinder Application by deriving from the AppBasic class
class BasicApp : public App
{
  public:
    static void prepare( Settings *settings );

    void setup();
    void shutdown();
    void keyDown( KeyEvent event );
    void update();
    void draw();

    void onButtonPress(unsigned int button, double timestamp);

  private:
    gl::Texture2dRef mTexture;
    std::string statusMsg;
};

void BasicApp::prepare( Settings *settings )
{
    settings->setTitle( "GamepadTest" );
}


// begin wrapper button functions
void onButtonDown(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context)
{
    if (verbose)
    {
        console() << "Button " << buttonID << " down on device " << device->deviceID << " at " << timestamp << " with context " << context << std::endl;
    }
    BasicApp* app = (BasicApp*) ci::app::App::get();
    app->onButtonPress(buttonID, timestamp);
}


void onButtonUp(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context)
{
    if (verbose)
    {
        console() << "Button " << buttonID << " up on device " << device->deviceID << " at " << timestamp << " with context " << context << std::endl;
    }
}

void onAxisMoved(struct Gamepad_device * device, unsigned int axisID, float value, float lastValue, double timestamp, void * context)
{
    if (verbose && (value < 0.2f || value > 0.2)) // reduce the output noise by making a dead zone
    {
        console() << "Axis " << axisID << " moved from " << lastValue <<" to " << value << " on device " << device->deviceID << " at " << timestamp << " with context " << context << std::endl;
    }
}

void onDeviceAttached(struct Gamepad_device * device, void * context)
{
    if (verbose)
    {
        console() << "Device ID " << device->deviceID << " attached (vendor = " << device->vendorID <<"; product = " << device->productID << ") with context" << context << std::endl;
    }
}

void onDeviceRemoved(struct Gamepad_device * device, void * context)
{
    if (verbose)
    {
        console() << "Device ID " << device->deviceID << " removed with context " << context << std::endl;
    }
}
// end wrapper button functions

void BasicApp::setup()
{
    Gamepad_deviceAttachFunc(onDeviceAttached, (void *) 0x1);
    Gamepad_deviceRemoveFunc(onDeviceRemoved, (void *) 0x2);
    Gamepad_buttonDownFunc(onButtonDown, (void *) 0x3);
    Gamepad_buttonUpFunc(onButtonUp, (void *) 0x4);
    // Disabling axis logging since we only have buttons
    //Gamepad_axisMoveFunc(onAxisMoved, (void *) 0x5);
    Gamepad_init();
    statusMsg = "ping";
}


void BasicApp::onButtonPress(unsigned int button, double timestamp)
{
    CI_LOG_D( "onButtonPress " << button << " " << timestamp );
    statusMsg = "Button " + std::to_string(button) + " " + std::to_string(timestamp);
}

void BasicApp::shutdown()
{
    Gamepad_shutdown();
}

void BasicApp::keyDown( KeyEvent event )
{
    if (event.getCode() == KeyEvent::KEY_ESCAPE)
    {
        quit();
    }
    statusMsg = "pong";
}

#define POLL_ITERATION_INTERVAL 30

void BasicApp::update()
{
	static unsigned int iterationsToNextPoll = POLL_ITERATION_INTERVAL;

    iterationsToNextPoll--;
    if (iterationsToNextPoll == 0) {
        Gamepad_detectDevices();
        iterationsToNextPoll = POLL_ITERATION_INTERVAL;
    }

    Gamepad_processEvents();

    TextLayout layout;
    layout.clear(ColorA(0.1f,0.1f,0.1f,0.7f));
    layout.setColor( Color( 0.9f, 0.9f, 0.9f ) );
    layout.setFont( Font( "Arial Black", 30 ) );
    layout.addCenteredLine(statusMsg);
    Surface8u rendered = layout.render( true, false );
    mTexture = gl::Texture2d::create( rendered );
}

void BasicApp::draw()
{

    // this pair of lines is the standard way to clear the screen in OpenGL
    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );
    gl::setMatricesWindow( getWindowSize() );

    gl::enableAlphaBlending( PREMULT );

    gl::color( Color::white() );
    gl::draw( mTexture, vec2( 10, getWindowHeight() - mTexture->getHeight() - 5 ) );
}

// This line tells Cinder to actually create the application
CINDER_APP( BasicApp, RendererGl( RendererGl::Options().msaa( 0 ) ), BasicApp::prepare )
