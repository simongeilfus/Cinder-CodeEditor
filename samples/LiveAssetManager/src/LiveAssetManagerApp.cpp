#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include "CodeEditor.h"
#include "LiveAssetManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LiveAssetManagerApp : public AppNative {
public:
	void setup();
	void draw();
    
    gl::GlslProg    mShader;
    CodeEditorRef   mCodeEditor;
};

void LiveAssetManagerApp::setup()
{
    
    // Create CodeEditor
    mCodeEditor = CodeEditor::create( "shaders/simple.frag", CodeEditor::Settings().autoSave() );
    
    // Load and watch shader asset
    //
    // For some reason even with the autoSave feature enabled,
    // you might need to manually save your file to get it reloaded.
    LiveAssetManager::load( "shaders/simple.frag", [this](DataSourceRef frag) {
        try {
            mShader = gl::GlslProg( NULL, frag );
            mCodeEditor->clearErrors();
        }
        catch( gl::GlslProgCompileExc exc ) {
            mCodeEditor->setError( "Simple: " + string( exc.what() ) );
        }
    } );
    
}


void LiveAssetManagerApp::draw()
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

CINDER_APP_NATIVE( LiveAssetManagerApp, RendererGl )
