#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include "CodeEditor.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MultipleFilesApp : public AppNative {
public:
	void setup();
	void draw();
    
    gl::GlslProg    mShader;
    CodeEditorRef   mCodeEditor;
};

void MultipleFilesApp::setup()
{
    
    // Create CodeEditor
    mCodeEditor = CodeEditor::create( { "SphereShader.vert", "SphereShader.frag" }, CodeEditor::Settings().lineNumbers().autoSave() );
    
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

void MultipleFilesApp::draw()
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

CINDER_APP_NATIVE( MultipleFilesApp, RendererGl )
