//
//  CodeEditor.cpp
//  AwesomiumTest
//
//  Created by Simon Geilfus on 25/01/13.
//
//

#include "CodeEditor.h"
#include "CinderAwesomium.h"
#include "cinder/Timeline.h"

using namespace std;
using namespace ci;


CodeEditorRef CodeEditor::create( const ci::fs::path& filePath, CodeEditor::Settings settings )
{
    return CodeEditorRef( new CodeEditor( filePath, settings ) );
}

#if defined( CINDER_MAC )
CodeEditorRef CodeEditor::create( std::initializer_list<ci::fs::path> filePaths, CodeEditor::Settings settings )
{
    return CodeEditorRef( new CodeEditor( filePaths, settings ) );
}
#else
CodeEditorRef CodeEditor::create( std::vector<ci::fs::path> filePaths, CodeEditor::Settings settings )
{
    return CodeEditorRef( new CodeEditor( filePaths, settings ) );
}
#endif


CodeEditor::CodeEditor( const ci::fs::path& filePath, Settings settings )
: mSettings( settings ), mVisible( true ), mTabsReady( 0 )
{
    fs::path p = app::getAssetPath("") / filePath;
    
    if( fs::exists( p ) && fs::is_directory( p ) ){
        fs::directory_iterator end;
        for( fs::directory_iterator it( p ) ; it != end ; ++it ){
            fs::path ext = it->path().extension();
            if( ext == ".frag" || ext == ".vert" || ext == ".glsl" ){
                TabRef tab = TabRef( new Tab( this ) );
                tab->mFileName = it->path().filename();
                tab->mFilePath = it->path();
                mTabs.push_back( tab );
            }
        }
        
        if( mTabs.size() )
            mCurrentTab = mTabs[0];
    }
    else if( fs::exists( p ) ){
        TabRef tab = TabRef( new Tab( this ) );
        tab->mFileName = filePath;
        tab->mFilePath = p;
        mTabs.push_back( tab );
        
        mCurrentTab = tab;
    }
    else if( !fs::exists( p ) ){
        if( !fs::exists( p.parent_path() ) )
            fs::create_directory( p.parent_path() );
        std::ofstream oStream( ( p ).c_str() );
        oStream.close();
    }
    
    initAwesomium();
    if( mSettings.getWindow() )
        connectWindow( mSettings.getWindow() );
}


#if defined( CINDER_MAC )
CodeEditor::CodeEditor( std::initializer_list<ci::fs::path> filePaths, Settings settings )
: mSettings( settings ), mVisible( true ), mTabsReady( 0 )
{
    for_each( filePaths.begin(), filePaths.end(), [this](fs::path path) {
        fs::path p = app::getAssetPath("") / path;
        
        if( !fs::exists( p ) ){
            if( !fs::exists( p.parent_path() ) )
                fs::create_directory( p.parent_path() );
            std::ofstream oStream( ( p ).c_str() );
            oStream.close();
        }
        
        TabRef tab = TabRef( new Tab( this ) );
        tab->mFileName = path;
        tab->mFilePath = p;
        mTabs.push_back( tab );
        
    } );
    
    if( mTabs.size() )
        mCurrentTab = mTabs[0];
    
    initAwesomium();
    if( mSettings.getWindow() )
        connectWindow( mSettings.getWindow() );
}
#else
CodeEditor::CodeEditor( std::vector<ci::fs::path> filePaths, Settings settings )
: mSettings( settings ), mVisible( true ), mTabsReady( 0 )
{
    for_each( filePaths.begin(), filePaths.end(), [this](fs::path path) {
        fs::path p = app::getAssetPath("") / path;
        
        if( !fs::exists( p ) ){
            if( !fs::exists( p.parent_path() ) )
                fs::create_directory( p.parent_path() );
            std::ofstream oStream( ( p ).c_str() );
            oStream.close();
        }
        
        TabRef tab = TabRef( new Tab( this ) );
        tab->mFileName = path;
        tab->mFilePath = p;
        mTabs.push_back( tab );
        
    } );
    
    if( mTabs.size() )
        mCurrentTab = mTabs[0];
    
    initAwesomium();
    if( mSettings.getWindow() )
        connectWindow( mSettings.getWindow() );
}
#endif


void CodeEditor::setup()
{    
    if( mSettings.isLineWrappingEnabled() ) enableLineWrapping();
    if( mSettings.isLineNumbersEnabled() ) enableLineNumbers();
    setFontSize( mSettings.getFontSize() );
    setTheme( mSettings.getTheme() );
    setOpacity( mSettings.getOpacity() );
    setHeight( mSettings.getHeight() );
}
void CodeEditor::connectWindow( app::WindowRef window )
{
    if( mSettings.isUpdateConnectionEnabled() )
        app::App::get()->getSignalUpdate().connect( std::bind( &CodeEditor::update, this ) );

    app::App::get()->getSignalShutdown().connect( std::bind( &CodeEditor::shutdown, this ) );
    
    if( mSettings.isPostDrawConnectionEnabled() )
        window->getSignalPostDraw().connect( std::bind( &CodeEditor::draw, this ) );
    
    window->connectKeyDown( &CodeEditor::keyDown, this );
    window->connectKeyUp( &CodeEditor::keyUp, this );
    window->connectMouseMove( &CodeEditor::mouseMove, this );
    window->connectMouseDown( &CodeEditor::mouseDown, this );
    window->connectMouseDrag( &CodeEditor::mouseDrag, this );
    window->connectMouseUp( &CodeEditor::mouseUp, this );
    window->connectMouseWheel( &CodeEditor::mouseWheel, this );
    window->connectFileDrop( &CodeEditor::fileDrop, this );
    window->connectResize( &CodeEditor::resize, this );
}
void CodeEditor::initTabs()
{
    
}
void CodeEditor::initAwesomium()
{
    
	// set Awesomium logging to verbose
	Awesomium::WebConfig cnf;
	cnf.log_level = Awesomium::kLogLevel_Verbose;
    
#if defined( CINDER_MAC )
	std::string frameworkPath = ( app::getAppPath() / "Contents" / "MacOS" ).string();
	cnf.package_path = Awesomium::WebString::CreateFromUTF8( frameworkPath.c_str(), frameworkPath.size() );
	cnf.log_path = Awesomium::WebString::CreateFromUTF8( frameworkPath.c_str(), frameworkPath.size() );
    
    std::string editorPath = ( app::getAppPath() / "Contents/MacOS/CodeEditor/editor.html" ).string();
#else
    std::string editorPath = ( app::getAppPath() / "editor_msw.html" ).string();    
#endif
    
	// initialize the Awesomium web engine
	mWebCorePtr = Awesomium::WebCore::Initialize( cnf );
    
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        
        // create a webview
        WebViewRef webView = WebViewRef( mWebCorePtr->CreateWebView( app::getWindowWidth(), app::getWindowHeight() ), mem_fn( &Awesomium::WebView::Destroy ) );
        
        webView->LoadURL( Awesomium::WebURL( Awesomium::WSLit( ( "file://" + editorPath ).c_str() ) ) );
        
        webView->SetTransparent( true );
        //webView->Focus();
        
        // set load listener to execute javascript
        webView->set_load_listener( tab.get() );
        webView->set_js_method_handler( tab.get() );
        
        tab->mWebView = webView;
    }
}

void CodeEditor::deleteTab( TabRef tab )
{
    for( std::vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        if( *it == tab ){ mTabs.erase( it ); return; }
    }
}

CodeEditor::TabRef CodeEditor::getTab( ci::fs::path fileName )
{
    for( std::vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        if( (*it)->mFileName == fileName )
            return *it;
    }
    return TabRef();
}

void CodeEditor::update()
{
    if( !mVisible ) return;
    
	mWebCorePtr->Update();
    
    if( mCurrentTab && mCurrentTab->mWebView ){
        if( ! mCurrentTab->mWebView->IsLoading() && ph::awesomium::isDirty( mCurrentTab->mWebView.get() ) )
        {
            try {
                gl::Texture::Format fmt;
                fmt.setMagFilter( GL_LINEAR );
                mWebTexture = ph::awesomium::toTexture( mCurrentTab->mWebView.get(), fmt );
            }
            catch( const std::exception &e ) { cout << e.what() << std::endl; }
        }
    }
    
}
void CodeEditor::draw()
{
    if( !mVisible ) return;
    
    if( mWebTexture )
	{
		gl::color( ColorA::white() );
        gl::enableAlphaBlending( true );
		gl::draw( mWebTexture );
		gl::disableAlphaBlending();
	}
}

void CodeEditor::shutdown()
{
    mTabs.clear();
	if( mCurrentTab ) {
        mCurrentTab.reset();        
    }
	Awesomium::WebCore::Shutdown();
}
void CodeEditor::resize()
{
	if( mCurrentTab )
		mCurrentTab->mWebView->Resize( app::getWindowWidth(), app::getWindowHeight() );
}

void CodeEditor::mouseMove( app::MouseEvent event )
{
    if( !mVisible || !mCurrentTab ) return;
    
	ph::awesomium::handleMouseMove( mCurrentTab->mWebView.get(), event );
}

void CodeEditor::mouseDown( app::MouseEvent event )
{
    if( !mVisible || !mCurrentTab ) return;
    
	ph::awesomium::handleMouseDown( mCurrentTab->mWebView.get(), event );
}

void CodeEditor::mouseDrag( app::MouseEvent event )
{
    if( !mVisible || !mCurrentTab ) return;
    
	ph::awesomium::handleMouseDrag( mCurrentTab->mWebView.get(), event );
}

void CodeEditor::mouseUp( app::MouseEvent event )
{
    if( !mVisible || !mCurrentTab ) return;
    
	ph::awesomium::handleMouseUp( mCurrentTab->mWebView.get(), event );
}

void CodeEditor::mouseWheel( app::MouseEvent event )
{
    if( !mVisible || !mCurrentTab ) return;
    
	ph::awesomium::handleMouseWheel( mCurrentTab->mWebView.get(), event );
}

void CodeEditor::keyDown( ci::app::KeyEvent event )
{
    // Toggle Visibility
    
    if( event.getChar() == '~' ){
        setVisible( !isVisible() );
        return;
    }
    
    if( !mVisible || !mCurrentTab ) return;
    
    
    // Save file
    
    if( event.isAccelDown() && event.getCode() == ci::app::KeyEvent::KEY_s ){
        mCurrentTab->write( mCurrentTab->mFilePath );
    }
    
    // Autocomplete
    
    else if( mSettings.isCodeCompletionEnabled() && event.isControlDown() && event.getCode() == ci::app::KeyEvent::KEY_SPACE ){
        autoComplete();
    }
    
    // Switch Tab
    
    else if( event.isShiftDown() && event.getCode() == ci::app::KeyEvent::KEY_TAB ){
        vector<TabRef>::iterator it;
        for( it = mTabs.begin(); it != mTabs.end(); ++it ){
            TabRef tab = *it;
            if( tab == mCurrentTab ){
                ++it;
                break;
            }
        }
        if( it == mTabs.end() ) it = mTabs.begin();
        mCurrentTab = *it;
        mCurrentTab->mWebView->Focus();
        ( (Awesomium::BitmapSurface*) mCurrentTab->mWebView->surface() )->set_is_dirty( true );
        update();
                
        Awesomium::JSArray args;
        args.Push( Awesomium::JSValue( Awesomium::WSLit( mCurrentTab->mFileName.string().c_str() ) ) );
        
        Awesomium::JSObject& window = mCurrentTab->mJSWindow;
        window.Invoke( Awesomium::WSLit("setTitleDialog"), args );
        window.Invoke( Awesomium::WSLit("setFocus"), Awesomium::JSArray() );
        app::timeline().add( [window]() mutable {
                window.Invoke( Awesomium::WSLit("clearTitleDialog"), Awesomium::JSArray() );
        }, app::timeline().getCurrentTime() + 1.0f );
    }
    
    // Switch Theme
    
    else if( event.isAccelDown() && event.getCode() == ci::app::KeyEvent::KEY_t ){
        if( mSettings.getTheme() == "light" )
            setTheme( "dark" );
        else setTheme( "light" );
    }
    // Other keys
    else {
        ph::awesomium::handleKeyDown( mCurrentTab->mWebView.get(), event );
    }
}
void CodeEditor::keyUp( ci::app::KeyEvent event )
{
    if( !mVisible || !mCurrentTab ) return;
    
	ph::awesomium::handleKeyUp( mCurrentTab->mWebView.get(), event );
}

void CodeEditor::fileDrop( ci::app::FileDropEvent event )
{
    if( !mVisible || !mCurrentTab ) return;
    
    fs::path ext = event.getFile(0).extension();
    if( ext == ".frag" || ext == ".vert" || ext == ".glsl" ){
        read( event.getFile(0) );
    }
}


void CodeEditor::write( const ci::fs::path& filePath )
{
    if( !mCurrentTab ) return;
    
    mCurrentTab->write( filePath );
}
void CodeEditor::read( const ci::fs::path& filePath )
{
    if( !mCurrentTab ) return;
    
    mCurrentTab->read( filePath );
}

void CodeEditor::setValue( const std::string &value )
{
    if( !mCurrentTab ) return;
    
    mCurrentTab->setValue( value );
}
std::string CodeEditor::getValue()
{
    if( !mCurrentTab ) return "";
    
    return mCurrentTab->getValue();
}
void CodeEditor::Tab::write( const ci::fs::path& filePath )
{
    string value = getValue();
    if( !value.empty() ){
        std::ofstream oStream( filePath.c_str() );
        oStream << value;
        oStream.close();
    }
}
void CodeEditor::Tab::read( const ci::fs::path& filePath )
{
    string value = ci::loadString( loadFile( filePath ) );
    if( !value.empty() ){
        setValue( value );
    }    
}

void CodeEditor::Tab::setValue( const std::string &value )
{
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( Awesomium::WSLit( value.c_str() ) ) );
    mJSWindow.Invoke( Awesomium::WSLit("setValue"), args );
    
}
std::string CodeEditor::Tab::getValue()
{
    Awesomium::JSValue value = mJSWindow.Invoke( Awesomium::WSLit("getValue"), Awesomium::JSArray() );
    if( value.IsString() ){
        return Awesomium::ToString( value.ToString() );
    }
    return "";
}

void CodeEditor::Tab::setMode( const std::string &value )
{
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( Awesomium::WSLit( value.c_str() ) ) );
    mJSWindow.Invoke( Awesomium::WSLit("setMode"), args );
}
void CodeEditor::autoComplete()
{
    if( mCurrentTab )
        mCurrentTab->mJSWindow.Invoke( Awesomium::WSLit("autoComplete"), Awesomium::JSArray() );
    
}

void CodeEditor::enableLineWrapping( bool enabled )
{
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( enabled ) );
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        tab->mJSWindow.Invoke( Awesomium::WSLit("enableLineWrapping"), args );
    }
}
void CodeEditor::enableLineNumbers( bool enabled )
{
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( enabled ) );
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        tab->mJSWindow.Invoke( Awesomium::WSLit("enableLineNumbers"), args );
    }
}

void CodeEditor::setOpacity( float alpha )
{
    mSettings.setOpacity( alpha );
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( alpha ) );
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        tab->mJSWindow.Invoke( Awesomium::WSLit("setOpacity"), args );
    }
}
void CodeEditor::setFontSize( int size )
{
    mSettings.setFontSize( size );
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( size ) );
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        tab->mJSWindow.Invoke( Awesomium::WSLit("setFontSize"), args );
    }
}
void CodeEditor::setTheme( const std::string& name )
{
    mSettings.setTheme( name );
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( Awesomium::WSLit( name.c_str() ) ) );
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        tab->mJSWindow.Invoke( Awesomium::WSLit("setTheme"), args );
    }
    setOpacity( mSettings.getOpacity() );
}
void CodeEditor::setHeight( float height )
{
    mSettings.setHeight( height );
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( height ) );
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        tab->mJSWindow.Invoke( Awesomium::WSLit("setHeight"), args );
    }
}

bool CodeEditor::hasFocus()
{
    return false;
}
void CodeEditor::blur()
{
    if( !mCurrentTab ) return;
    
    mCurrentTab->mJSWindow.Invoke( Awesomium::WSLit("blur"), Awesomium::JSArray() );
}


void CodeEditor::setError( /*uint16_t line, */const std::string &message )
{
    clearErrors();
    Awesomium::JSArray args;
    args.Push( Awesomium::JSValue( 0 ) );//line ) );
    args.Push( Awesomium::JSValue( Awesomium::WSLit( message.c_str() ) ) );
    
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        tab->mJSWindow.Invoke( Awesomium::WSLit("setError"), args );
    }
}
void CodeEditor::clearErrors()
{
    for( vector<TabRef>::iterator it = mTabs.begin(); it != mTabs.end(); ++it ){
        TabRef tab = *it;
        tab->mJSWindow.Invoke( Awesomium::WSLit("clearErrors"), Awesomium::JSArray() );
    }
}


void CodeEditor::registerCodeChanged( const std::string& fileName, std::function<void(const std::string&)> fn )
{
    TabRef tab = getTab( fileName );
    if( tab ){
        tab->mChangeCallback = fn;
    }
}

void CodeEditor::registerCodeChanged( const std::string& firstFileName, const std::string& secondFileName, std::function<void(const std::string&,const std::string&)> fn )
{
    TabRef firstTab = getTab( firstFileName );
    TabRef secondTab = getTab( secondFileName );
    if( firstTab && secondTab ){
        firstTab->mPairChangeCallback   = fn;
        secondTab->mPairChangeCallback  = fn;
        firstTab->mPairedTab            = secondTab;
        firstTab->mPairIsFirst          = true;
        secondTab->mPairedTab           = firstTab;
    }
    
}

void CodeEditor::Tab::OnDocumentReady(Awesomium::WebView* caller, const Awesomium::WebURL& url) {
    
    Awesomium::JSValue window = caller->ExecuteJavascriptWithResult( Awesomium::WSLit("window"), Awesomium::WSLit(""));
    
    if (window.IsObject()) {
        mJSWindow = window.ToObject();
        mJSWindow.SetCustomMethod( Awesomium::WSLit("changeCallback"), false );
        read( mFilePath );
        
        if( mFilePath.extension() == ".lua" )
            setMode( "lua" );
        
        mParent->mTabsReady++;
        
        if( mParent->mTabsReady >= mParent->mTabs.size() )
            mParent->setup();
    }
}


void CodeEditor::Tab::OnMethodCall(Awesomium::WebView* caller,unsigned int remote_object_id,const Awesomium::WebString& method_name,const Awesomium::JSArray& args)
{
    if( Awesomium::ToString( method_name ) == "changeCallback" ){
        if( mPairChangeCallback ){
            if( mPairIsFirst )
                mPairChangeCallback( getValue(), mPairedTab->getValue() );
            else
                mPairChangeCallback( mPairedTab->getValue(), getValue() );
        }
        else if( mChangeCallback )
            mChangeCallback( getValue() );
        if( mParent->mSettings.isAutoSaveEnabled() )
            write( mFilePath );
    }
}