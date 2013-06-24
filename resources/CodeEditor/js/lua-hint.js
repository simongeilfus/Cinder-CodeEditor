(function () {
  function forEach(arr, f) {
    for (var i = 0, e = arr.length; i < e; ++i) f(arr[i]);
  }
  
  function arrayContains(arr, item) {
    if (!Array.prototype.indexOf) {
      var i = arr.length;
      while (i--) {
        if (arr[i] === item) {
          return true;
        }
      }
      return false;
    }
    return arr.indexOf(item) != -1;
  }


  var types =  ("Area Camera CameraPersp Channel Color ColorA MouseEvent KeyEvent Font Matrix44f Matrix33f " +
    " MayaCamUI Perlin Quatf Rectf Surface TextLayout TextBox TriMesh Vec2f Vec2i Vec3f vector_float " +
    " vector_int vector_uint32_t vector_string vector_Vec2i vector_Vec2f vector_Vec3f vector_Color vector_ColorA " +
    " gl.Fbo gl.Fbo.Format gl.GlslProg gl.Texture gl.VboMesh gl.VboMesh.Layout " + 
    " DataSourceRef ImageSourceRef").split(" ");

  var typesQualifiers =  ("local global" ).split(" ");
  var preProcessor =  ("#define #undef #if #ifdef #ifndef #else #elif #endif #error #pragma #line #version #extension" ).split(" ");

  var keywords = ("and break elseif false nil not or return " +
    "true function end if then else do " +
    "while repeat until for in local " +
    "function if repeat do " +
    "end until " ).split(" ");

  var builtins = ( "_G _VERSION assert collectgarbage dofile error getfenv getmetatable ipairs load " +
    "loadfile loadstring module next pairs pcall print rawequal rawget rawset require " +
    "select setfenv setmetatable tonumber tostring type unpack xpcall " +
    "coroutine.create coroutine.resume coroutine.running coroutine.status coroutine.wrap coroutine.yield " +
    "debug.debug debug.getfenv debug.gethook debug.getinfo debug.getlocal debug.getmetatable " +
    "debug.getregistry debug.getupvalue debug.setfenv debug.sethook debug.setlocal debug.setmetatable " +
    "debug.setupvalue debug.traceback " +
    "close flush lines read seek setvbuf write " +
    "io.close io.flush io.input io.lines io.open io.output io.popen io.read io.stderr io.stdin " +
    "io.stdout io.tmpfile io.type io.write " +
    "math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg " +
    "math.exp math.floor math.fmod math.frexp math.huge math.ldexp math.log math.log10 math.max " +
    "math.min math.modf math.pi math.pow math.rad math.random math.randomseed math.sin math.sinh " +
    "math.sqrt math.tan math.tanh " +
    "os.clock os.date os.difftime os.execute os.exit os.getenv os.remove os.rename os.setlocale " +
    "os.time os.tmpname " +
    "package.cpath package.loaded package.loaders package.loadlib package.path package.preload " +
    "package.seeall " +
    "string.byte string.char string.dump string.find string.format string.gmatch string.gsub " +
    "string.len string.lower string.match string.rep string.reverse string.sub string.upper " +
    "table.concat table.insert table.maxn table.remove table.sort" +
    "gl.isExtensionAvailable gl.clear gl.setMatrices gl.setModelView gl.setProjection gl.pushModelView " +
    "gl.popModelView gl.pushMatrices gl.popMatrices gl.multModelView gl.multProjection gl.getModelView " +
    "gl.getProjection gl.setMatricesWindowPersp gl.setMatricesWindow " +
    "gl.getViewport gl.translate gl.scale gl.rotate gl.Begin gl.End gl.vertex " +
    "gl.color gl.enable gl.disable gl.enableAlphaBlending gl.disableAlphaBlending gl.enableAdditiveBlending " +
    "gl.enableAlphaTest gl.disableAlphaTest gl.enableWireframe gl.disableWireframe " +
    "gl.disableDepthRead gl.disableDepthWrite gl.enableDepthRead gl.enableDepthWrite " +
    "gl.drawLine gl.drawCube gl.drawColorCube gl.drawStrokedCube " +
    "gl.drawSphere gl.drawSolidCircle gl.drawStrokedCircle gl.drawSolidRect " +
    "gl.drawSolidRect gl.drawStrokedRect gl.drawCoordinateFrame gl.drawVector gl.drawFrustum gl.drawTorus " +
    "gl.drawCylinder gl.draw gl.drawRange gl.drawArrays " +
    "gl.drawBillboard gl.drawString gl.drawStringCentered gl.drawStringRight " +
    "getWindowWidth getWindowHeight getWindowCenter getWindowSize getWindowAspectRatio getWindowBounds " +
    "getFrameRate setFrameRate getAverageFps getFpsSampleInterval setFpsSampleInterval " +
    "isFullScreen setFullScreen " +
    "getMousePos hideCursor showCursor " +
    "getElapsedSeconds getElapsedFrames " +
    "quit getAppPath console " +
    "randSeed randBool randInt randFloat randPosNegFloat randVec2f randVec3f " +
    "loadFile loadResource loadAsset loadImage ").split(" ");

  function scriptHint(editor, getToken, options) {
    // Find the token at the cursor
    var cur = editor.getCursor(), token = getToken(editor, cur), tprop = token;
    // If it's not a 'word-style' token, ignore the token.
		/*if (!/^[\w$_]*$/.test(token.string)) {
      token = tprop = {start: cur.ch, end: cur.ch, string: "", state: token.state,
                       type: token.string == "." ? "property" : null};
    }*/

    var variableNames = [];
    for( var line = 0; line < editor.lineCount() && line < editor.getCursor().line; line++ ){
      var lineContent = editor.getLine( line );
      var containsType = false;
      var typeIndex;

      for (var i = 0, e = types.length; i < e; ++i){
        typeIndex = lineContent.indexOf( types[i] );
        if( typeIndex != -1 ){
          containsType = true;
          break;
        }
      }

      if( containsType ){
        var variableName = lineContent.substring( typeIndex );
        variableName = variableName.substring( variableName.search( /\s/ ) );
        variableName = variableName.replace(/\s+/g, '');
        variableName = variableName.substring( 0, variableName.search( /[^A-Za-z]/ ) );
        if( variableName != "main" )
          variableNames.push( variableName );
      }
      
    }

    return {list: getCompletions(token, variableNames, options),
            from: {line: cur.line, ch: token.start},
            to: {line: cur.line, ch: token.end}};
  }

  CodeMirror.glslHint = function(editor, options) {
    return scriptHint(editor,
                      function (e, cur) {return e.getTokenAt(cur);},
                      options);
  };


  function getCompletions(token, variableNames, options) {
    var found = [], start = token.string;
    function maybeAdd(str) {
      if (str.indexOf(start) == 0 && !arrayContains(found, str)) found.push(str);
    }
    for (var i = 0; i < variableNames.length; i++){
      maybeAdd( variableNames[i] );
    }

    forEach(types, maybeAdd);
    forEach(typesQualifiers, maybeAdd);
    forEach(preProcessor, maybeAdd);
    forEach(keywords, maybeAdd);
    forEach(builtins, maybeAdd);
    
    return found;
  }
})();
