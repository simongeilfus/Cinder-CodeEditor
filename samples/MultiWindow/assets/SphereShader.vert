void main(){
  	vec4 pos = gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * pos; 
}