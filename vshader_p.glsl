attribute vec4 vPosition;
attribute vec3 vNormal;
attribute vec2 vTexCoord;

// output values that will be interpretated per-fragment
varying vec4 color;
varying vec4 position;
varying vec3 normal;
varying vec2 texCoord;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 Light1Position;
uniform vec4 Light2Position;
uniform float Shininess;

uniform int lightMovement;
uniform int shading;
uniform int reflection;
uniform int pointOn;
uniform int directionalOn;

vec4 newcolor( vec4 LightPosition ){
	
	vec3 pos = (ModelView * vPosition).xyz;
	
	if(lightMovement == 2) LightPosition = ModelView * LightPosition;
	
	vec3 L = normalize( LightPosition.xyz - pos); // light direction
	vec3 E = normalize( -pos ); // viewer direction
	vec3 H = normalize( L + E ); // halfway vector
	
	// Transform vertex normal into eye coordinates
	vec3 N = normalize( ModelView * vec4(vNormal, 0.0) ).xyz;
	vec3 R = 2.0 * dot(L, N) * N - L;	//
	
	// Compute terms in the illumination equation
	vec4 ambient = AmbientProduct;
	
	float Kd = max( dot(L, N), 0.0 ); //set diffuse to 0 if light is behind the surface point
	vec4  diffuse = Kd * DiffuseProduct;
	
	float Ks;
	if(reflection == 1) Ks = pow( max(dot(E, R), 0.0), Shininess );	// Phong Reflection Model
	else Ks = pow( max(dot(N, H), 0.0), Shininess );	// Modified Phong Reflection Model
	vec4  specular = Ks * SpecularProduct;
	
	//ignore also specular component if light is behind the surface point
	if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	return ambient + diffuse + specular;
	
}

void main(){
	
	// Gouraud Shading
	if (shading != 1){
		color = vec4(0);
		if (pointOn == 1) color += newcolor(Light1Position);
		if (directionalOn == 1) color += newcolor(Light2Position);
		color.a = 1.0;
	}
	
	// Store variables for Phong Shading
	normal = vNormal;
	texCoord = vTexCoord;
	position = ModelView * vPosition;
	gl_Position = Projection * position;
	
}