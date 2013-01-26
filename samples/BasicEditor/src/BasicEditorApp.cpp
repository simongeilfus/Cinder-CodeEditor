#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include "CodeEditor.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicEditorApp : public AppNative {
  public:
	void setup();
	void draw();
    
    gl::GlslProg    mShader;
    CodeEditorRef   mCodeEditor;
};

void BasicEditorApp::setup()
{
    
    // Create CodeEditor
    mCodeEditor = CodeEditor::create( "shaders/simple.frag" );
    
    mCodeEditor->registerCodeChanged( "shaders/simple.frag", [this](const string& frag) {
        try {
            mShader = gl::GlslProg( NULL, frag.c_str() );
            mCodeEditor->clearErrors();
        }
        catch( gl::GlslProgCompileExc exc ) {
            mCodeEditor->setError( "Simple: " + string( exc.what() ) );
        }
    } );
    
}


void BasicEditorApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    if( mShader ){
        gl::enableAlphaBlending();
        mShader.bind();
        gl::drawSolidRect( getWindowBounds() );
        mShader.unbind();
		gl::disableAlphaBlending();
    }
}

CINDER_APP_NATIVE( BasicEditorApp, RendererGl )
