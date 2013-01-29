//
//  CodeEditor.h
//  AwesomiumTest
//
//  Created by Simon Geilfus on 25/01/13.
//
//

#pragma once

#include "cinder/app/AppNative.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "cinder/DataTarget.h"

#include <Awesomium/WebCore.h>
#include <Awesomium/BitmapSurface.h>
#include <Awesomium/STLHelpers.h>

#include <fstream>

typedef std::shared_ptr< class CodeEditor > CodeEditorRef;

class CodeEditor {
public:
    
    struct Settings {
    public:
        Settings()
        : mCodeCompletionEnabled( true ), mLineWrappingEnabled( true ), mLineNumbersEnabled( false ), mAutoSaveEnabled( false ), mWindow( ci::app::getWindow() ), mPostDrawConnection( true ),mUpdateConnection( true ), mOpacity( 0.6f ), mFontSize( 11 ), mTheme( "light" ), mHeight( 0.0f )
        {}
        
        bool isCodeCompletionEnabled(){ return mCodeCompletionEnabled; }
        void enableCodeCompletion( bool enabled = true ){ mCodeCompletionEnabled = enabled; }
        void disableCodeCompletion(){ mCodeCompletionEnabled = false; }
        Settings& codeCompletion( bool enabled = true ){ enableCodeCompletion( enabled ); return *this; }
        
        bool isLineWrappingEnabled(){ return mLineWrappingEnabled; }
        void enableLineWrapping( bool enabled = true ){ mLineWrappingEnabled = enabled; }
        void disableLineWrapping(){ mLineWrappingEnabled = false; }
        Settings& lineWrapping( bool enabled = true ){ enableLineWrapping( enabled ); return *this; }
        
        bool isLineNumbersEnabled(){ return mLineNumbersEnabled; }
        void enableLineNumbers( bool enabled = true ){ mLineNumbersEnabled = enabled; }
        void disableLineNumbers(){ mLineNumbersEnabled = false; }
        Settings& lineNumbers( bool enabled = true ){ enableLineNumbers( enabled ); return *this; }
        
        bool isAutoSaveEnabled(){ return mAutoSaveEnabled; }
        void enableAutoSave( bool enabled = true ){ mAutoSaveEnabled = enabled; }
        void disableAutoSave(){ mAutoSaveEnabled = false; }
        Settings& autoSave( bool enabled = true ){ enableAutoSave( enabled ); return *this; }
        
        ci::app::WindowRef getWindow(){ return mWindow; }
        void setWindow( ci::app::WindowRef window ){ mWindow = window; }
        Settings& window( ci::app::WindowRef window ){ setWindow( window ); return *this; }
        
        bool isPostDrawConnectionEnabled(){ return mPostDrawConnection; }
        void enablePostDrawConnection( bool enabled = true ){ mPostDrawConnection = enabled; }
        void disablePostDrawConnection(){ mPostDrawConnection = false; }
        Settings& postDrawConnection( bool enabled = true ){ enablePostDrawConnection( enabled ); return *this; }
                
        bool isUpdateConnectionEnabled(){ return mUpdateConnection; }
        void enableUpdateConnection( bool enabled = true ){ mUpdateConnection = enabled; }
        void disableUpdateConnection(){ mUpdateConnection = false; }
        Settings& updateConnection( bool enabled = true ){ enableUpdateConnection( enabled ); return *this; }
        
        float getOpacity(){ return mOpacity; }
        void setOpacity( float alpha ){ mOpacity = alpha; }
        Settings& opacity( float alpha ){ setOpacity( alpha ); return *this; }
        
        int getFontSize(){ return mFontSize; }
        void setFontSize( int size ){ mFontSize = size; }
        Settings& fontSize( int size ){ setFontSize( size ); return *this; }
        
        std::string getTheme(){ return mTheme; }
        void setTheme( const std::string& name ){ mTheme = name; }
        Settings& theme( const std::string& name ){ setTheme( name ); return *this; }
        
        float getHeight(){ return mHeight; }
        void setHeight( float height ){ mHeight = height; }
        Settings& height( float height ){ setHeight( height ); return *this; }
        
    private:
        bool                mCodeCompletionEnabled;
        bool                mLineWrappingEnabled;
        bool                mLineNumbersEnabled;
        bool                mAutoSaveEnabled;
        bool                mPostDrawConnection;
        bool                mUpdateConnection;
        float               mOpacity;
        int                 mFontSize;
        float               mHeight;
        std::string         mTheme;
        ci::app::WindowRef  mWindow;
    };
    
    static CodeEditorRef create( const ci::fs::path& filePath = "", Settings settings = Settings() );
#if defined( CINDER_MAC )
    static CodeEditorRef create( std::initializer_list<ci::fs::path> filePaths, Settings settings = Settings() );
#else
    static CodeEditorRef create( std::vector<ci::fs::path> filePaths, Settings settings = Settings() );
#endif
    
    void write( const ci::fs::path& filePath );
    void read( const ci::fs::path& filePath );
    
    void        setValue( const std::string &value );
    std::string getValue();
    
    void setVisible( bool visible = true ){ mVisible = visible; }
    bool isVisible(){ return mVisible; }
    void hide(){ setVisible( false ); }
    void show(){ setVisible( true ); }
    
    bool hasFocus();
    void blur();
    
    void update();
    void draw();
    
    void shutdown();
	void resize();
	
	void mouseMove( ci::app::MouseEvent event );
	void mouseDown( ci::app::MouseEvent event );
	void mouseDrag( ci::app::MouseEvent event );
	void mouseUp( ci::app::MouseEvent event );
	void mouseWheel( ci::app::MouseEvent event );
	
	void keyDown( ci::app::KeyEvent event );
	void keyUp( ci::app::KeyEvent event );
    
    void fileDrop( ci::app::FileDropEvent event );
    
    void enableLineWrapping( bool enabled = true );
    void enableLineNumbers( bool enabled = true );
    
    void setOpacity( float alpha );
    void setFontSize( int size );
    void setTheme( const std::string& name );
    void setHeight( float height );
    
    void setError( /*uint16_t line, */const std::string &message );
    void clearErrors();
    
    void registerCodeChanged( const std::string& fileName, std::function<void(const std::string&)> fn );
    void registerCodeChanged( const std::string& firstFileName, const std::string& secondFileName, std::function<void(const std::string&,const std::string&)> fn );
    
protected:
    
    CodeEditor( const ci::fs::path& filePath, Settings settings = Settings() );
#if defined( CINDER_MAC )
    CodeEditor( std::initializer_list<ci::fs::path> filePaths, Settings settings = Settings() );
#else
    CodeEditor( std::vector<ci::fs::path> filePaths, Settings settings = Settings() );
#endif
    
    void setup();
    void connectWindow( ci::app::WindowRef window );
    void initTabs();
    void initAwesomium();
    
    class Tab;
    typedef std::shared_ptr<Awesomium::WebView> WebViewRef;
    typedef std::shared_ptr<Tab>                TabRef;
    
    class Tab : public Awesomium::WebViewListener::Load, public Awesomium::JSMethodHandler {
    public:
        Tab( CodeEditor* parent )
        : mParent( parent ), mPairIsFirst( false )
        {}
        
        void write( const ci::fs::path& filePath );
        void read( const ci::fs::path& filePath );
        
        void        setValue( const std::string &value );
        std::string getValue();
        
        void OnBeginLoadingFrame(Awesomium::WebView* caller,int64 frame_id,bool is_main_frame,const Awesomium::WebURL& url,bool is_error_page) {}
        void OnFailLoadingFrame(Awesomium::WebView* caller,int64 frame_id,bool is_main_frame,const Awesomium::WebURL& url,int error_code,const Awesomium::WebString& error_desc) {}
        void OnFinishLoadingFrame(Awesomium::WebView* caller,int64 frame_id,bool is_main_frame,const Awesomium::WebURL& url) {}
        void OnDocumentReady(Awesomium::WebView* caller, const Awesomium::WebURL& url);
        
        void                OnMethodCall(Awesomium::WebView* caller,unsigned int remote_object_id,const Awesomium::WebString& method_name,const Awesomium::JSArray& args);
        Awesomium::JSValue  OnMethodCallWithReturnValue(Awesomium::WebView* caller, unsigned int remote_object_id, const Awesomium::WebString& method_name, const Awesomium::JSArray& args){ return Awesomium::JSValue(); }
        
        WebViewRef                              mWebView;
        ci::fs::path                            mFileName;
        ci::fs::path                            mFilePath;
        Awesomium::JSObject                     mJSWindow;
        CodeEditor*                             mParent;
        std::function<void(const std::string&)> mChangeCallback;
        TabRef                                  mPairedTab;
        std::function<void(const std::string&,
                           const std::string&)> mPairChangeCallback;
        bool                                    mPairIsFirst;
        
        friend class CodeEditor;
    };
    
    
    void    deleteTab( TabRef tab );
    TabRef  getTab( ci::fs::path fileName );
    
    void autoComplete();
    
    
    Settings                    mSettings;
    
	Awesomium::WebCore*         mWebCorePtr;
	TabRef                      mCurrentTab;
    std::vector<TabRef>         mTabs;
    int                         mTabsReady;
    
    ci::gl::Texture             mWebTexture;
    bool                        mVisible;
};