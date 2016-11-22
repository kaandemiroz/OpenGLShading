// per-fragment interpolated values from the vertex shader
varying vec4 color;
varying vec4 position;
varying vec3 normal;
varying vec2 texCoord;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform vec4 Light1Position;
uniform vec4 Light2Position;
uniform float Shininess;

uniform int dispMode;
uniform int shading;
uniform int reflection;
uniform int lightMovement;
uniform int pointOn;
uniform int directionalOn;

uniform sampler2D tex;

vec4 light( vec4 LightPosition ){
	
	vec3 pos = position.xyz;
	
	if(lightMovement == 2) LightPosition = ModelView * LightPosition;
	
	vec3 L = normalize( LightPosition.xyz - pos); // light direction
	vec3 E = normalize( -pos ); // viewer direction
	vec3 H = normalize( L + E ); // halfway vector
	
	// Transform vertex normal into eye coordinates
	vec3 N = normalize( ModelView * vec4(normal, 0.0) ).xyz;
	vec3 R = 2.0 * dot(L, N) * N - L;
	
	// Compute terms in the illumination equation
	vec4 ambient = AmbientProduct;
	
	float Kd = max( dot(L, N), 0.0 ); //set diffuse to 0 if light is behind the surface point
	vec4 diffuse = Kd * DiffuseProduct;
	
	float Ks;
	if(reflection == 1) Ks = pow( max(dot(E, R), 0.0), Shininess );
	else Ks = pow( max(dot(N, H), 0.0), Shininess );
	vec4 specular = Ks * SpecularProduct;
	
	//ignore also specular component if light is behind the surface point
	if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}
	if(pointOn == 1 && directionalOn == 1) ambient *= 0.5;
	if(dispMode == 4)
		return (ambient + diffuse) * texture2D(tex, texCoord) + specular;
	else
		return ambient + diffuse + specular;
}

void main(){

	// Wireframe and Shading
	if(dispMode == 1 || dispMode == 2)
		// Gouraud
		if(shading != 1){
			gl_FragColor = color;
		}else{
			// Phong
			vec4 color = vec4(0);
			
			if(pointOn == 1) color += light(Light1Position);
			if(directionalOn == 1) color += light(Light2Position);
			color.a = 1.0;

			gl_FragColor = color;
		}
	else if(dispMode == 3)
		gl_FragColor = texture2D(tex, texCoord);
	else if(dispMode == 4)
		// Shading + Texture
		if(shading != 1){
			// Gouraud
			gl_FragColor = color * texture2D(tex, texCoord);
		}else{
			// Phong
			vec4 color = vec4(0);
			
			if(pointOn == 1) color += light(Light1Position);
			if(directionalOn == 1) color += light(Light2Position);
			color.a = 1.0;
			
			gl_FragColor = color;
		}
	else
		// If nothing works, show a gray silhouette
		gl_FragColor = vec4(0.1, 0.1, 0.1, 1.0);
	
}
