Cinder-CodeEditor
================

Full-Featured Code Editor for Cinder.
(Based on Awesomium and CodeMirror).

#####Features:
* Supported Languages: Glsl (XML, JSon and Lua are coming...).
* Syntax Highlighting.
* Code Completion.
* Search / Replace.
* In-Editor Error messages.
* Code Changed callback system.
* Multi-Window support.
* Auto-Save.
* Support c++11 Lambdas and Intializer Lists.

#####Block Depedencies
To use this block you will need:
* The [appRewrite branch](https://forum.libcinder.org/#Topic/23286000001389463)  of Cinder.
* Paul Houx's [Cinder-Awesomium block](https://github.com/paulhoux/Cinder-Awesomium).
* Some samples depends on my [LiveAssetManager](https://github.com/simongeilfus/Cinder-LiveAssetManager) block as well, but you don't need it to use the block.

#####Getting Started

A basic, one file editor is created like this:

```c
mCodeEditor = CodeEditor::create( "simple.frag" );
```

You can provide an initializer list to open multiple files:

```c
mCodeEditor = CodeEditor::create( { "simple.vert", "simple.frag" } );
```

If you don't specify any settings the editor will come with default options and will register all update/draw/mouse/keyboard signals of your app. You don't need to update or draw anything, the editor will get renderer on top of everything at the end of your "draw" function. By default the editor is visible but you can hide it by pressing `~` or by calling `mCodeEditor->hide()`.
Use `CodeEditor::Settings` if you want to override default settings:

```c
CodeEditor::Settings settings;
settings.disableCodeCompletion();
settings.enableLineNumbers();
mCodeEditor = CodeEditor::create( "simple.frag", settings );
```

This will have exactly the same effect:

```c
mCodeEditor = CodeEditor::create( "simple.frag", CodeEditor::Settings().codeCompletion(false).lineNumbers() );
```

You can provide an `app::WindowRef` if you don't want the editor to live in the main window:

```c
mCodeEditor = CodeEditor::create( "simple.frag", CodeEditor::Settings().window( otherWindow ) );
```

Just pass a null `app::WindowRef` if you want to take care of all the events yourself:

```c
mCodeEditor = CodeEditor::create( "simple.frag", CodeEditor::Settings().window( WindowRef() ) );
```
You will then need to call the corresponding methods in your app draw,update,mouseMove,mouseDown,... functions if you want the editor to work as expected.

Use `CodeEditor::registerCodeChanged` if you want to do something when the code get modified:

```c
mCodeEditor->registerCodeChanged( "simple.frag", [this](const string& frag) {
        try {
            mShader = gl::GlslProg( NULL, frag.c_str() );
        }
        catch( gl::GlslProgCompileExc exc ) {}
    } );
```

```c
mCodeEditor->registerCodeChanged( "simple.vert", "simple.frag", [this](const string& vert,const string& frag) {
        try {
            mShader = gl::GlslProg( vert.c_str(), frag.c_str() );
            mCodeEditor->clearErrors();
        }
        catch( gl::GlslProgCompileExc exc ) {}
    } );
```

Use `CodeEditor::setError` to display error messages in the editor:

```c
mCodeEditor->registerCodeChanged( "simple.frag", [this](const string& frag) {
        try {
            mShader = gl::GlslProg( NULL, frag.c_str() );
            mCodeEditor->clearErrors();
        }
        catch( gl::GlslProgCompileExc exc ) {
            mCodeEditor->setError( "Simple: " + string( exc.what() ) );
        }
    } );
```

You might want to remove focus from the editor `CodeEditor::blur` when using `keyDown` in your main app:

```c
void MyApp::keyDown( KeyEvent event )
{
    if( event.isAccelDown() && event.getCode() == KeyEvent::KEY_RETURN ){
        mCodeEditor->blur();
        setFullScreen( !isFullScreen() );
    }
}
```


#####Note on Files
You don't need to make sure the files already exists, if they don't the editor will take care of creating folders or files. This makes prototyping much more easy.

The editor supports drag'n'drop as well.

There's an Auto-Save feature that you can set in your settings, if you want your files to be saved every time they get modified. It might come handy if you are already watching your assets with something like [LiveAssetManager](https://github.com/simongeilfus/Cinder-LiveAssetManager).

#####Editor Shortcuts

* `Ctrl-F / Cmd-F`
Start searching
* `Ctrl-G / Cmd-G`
Find next
* `Shift-Ctrl-G / Shift-Cmd-G`
Find previous
* `Shift-Ctrl-F / Cmd-Option-F`
Replace
* `Shift-Ctrl-R / Shift-Cmd-Option-F`
Replace all
* `Cmd-S`
Save
* `~`
Toggle editor visibility
* `Shift+Tab`
Switch tab


#####CodeMirror

The editor itself is based on the excellent [CodeMirror](https://github.com/marijnh/CodeMirror). See license [here](https://github.com/simongeilfus/Cinder-CodeEditor/blob/master/CM-LICENSE).


#####Licence

Copyright (c) 2013, Simon Geilfus - All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

Redistribution and use in source and binary forms, with or without modification, are permitted provided that
the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and	the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
