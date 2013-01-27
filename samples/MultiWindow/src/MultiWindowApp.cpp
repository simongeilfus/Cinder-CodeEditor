#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include "CodeEditor.h"

// VisualStudio does'nt seem to support initializer_list
// yet so let's use boost::assign::list_of instead
#if defined( CINDER_MSW )
#include "boost/assign/list_of.hpp"
using namespace boost::assign;
#endif


using namespace ci;
using namespace ci::app;
using namespace std;

class MultiWindowApp : public AppNative {
  public:
	void setup();
	void drawMain();
    void keyDown( KeyEvent event );
    
    gl::GlslProg    mShader;
    CodeEditorRef   mCodeEditor;
};

void MultiWindowApp::setup()
{
    // Create secondary window
    WindowRef editorWindow = createWindow( Window::Format().size( 500, 180 ) );
    editorWindow->getSignalDraw().connect( [this](){ gl::clear( ColorA::gray( 0.85f ) ); } );
    editorWindow->setPos( getWindowIndex(0)->getPos() - Vec2i( 510, 0 ) );
    
    // Connect main draw to main window
    getWindowIndex(0)->connectDraw( &MultiWindowApp::drawMain, this );
    
    // Create CodeEditor
#if defined( CINDER_MSW )
    mCodeEditor = CodeEditor::create( list_of<string>( "SphereShader.vert" )( "SphereShader.frag"), CodeEditor::Settings().lineNumbers().autoSave() );
#else
    mCodeEditor = CodeEditor::create( { "SphereShader.vert", "SphereShader.frag" }, CodeEditor::Settings().lineNumbers().autoSave() );
#endif
    
    mCodeEditor->registerCodeChanged( "SphereShader.vert", "SphereShader.frag", [this](const string& vert,const string& frag) {
        try {
            mShader = gl::GlslProg( vert.c_str(), frag.c_str() );
            mCodeEditor->clearErrors();
        }
        catch( gl::GlslProgCompileExc exc ) {
            mCodeEditor->setError( "Sphere: " + string( exc.what() ) );
        }
    } );
}

void MultiWindowApp::drawMain()
{
    gl::clear( Color( 1, 1, 1 ) );
    
    if( mShader ){
        gl::enableAlphaBlending();
        gl::enableWireframe();
        
        mShader.bind();
        gl::drawSphere( Vec3f( getWindowCenter().x, getWindowCenter().y, 0.0f ), 150.0f );
        mShader.unbind();
        
        gl::disableWireframe();
		gl::disableAlphaBlending();
    }
}


void MultiWindowApp::keyDown( KeyEvent event )
{
    if( event.isAccelDown() && event.getCode() == KeyEvent::KEY_RETURN ){
        mCodeEditor->blur();
        setFullScreen( !isFullScreen() );
    }
}

CINDER_APP_NATIVE( MultiWindowApp, RendererGl )
