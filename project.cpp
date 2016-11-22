//
//  programming_assignment_3.cpp
//  COMP410
//
//  Created by Osman Kaan Demiröz on 09/04/16.
//  Copyright © 2016 Osman Kaan Demiröz. All rights reserved.
//

#include "Angel.h"
#include "fstream"
using std::ifstream;

typedef Angel::vec4	point4;
typedef Angel::vec4	color4;

const int MAX_CHARS_PER_LINE = 50;
const char* const DELIMITER = " \t";
int NumVertices, NumFaces, TextureSize;
float width, height, angle = 0.0;
unsigned long sizeOfPoints = 0, sizeOfTextures = 0;

bool trackingMouse = false;

point4* points;
point4* vertices;
vec3* normalvectors;
vec3* faces;
vec3* normals;
vec2* textures;
vec2* tex_coords;
GLubyte* image;

enum {ortographic = 1, perspective = 2};
enum {wireframe = 1, shade = 2, texture = 3, both = 4};
enum {red = 1, green = 2, blue = 3, cyan = 4, magenta = 5, yellow = 6, white = 7, gray = 8, black = 9};
enum {plastic = 1, metallic = 2};
enum {fixed = 1, moving = 2};
enum {point = 1, directional = 2};
enum {phong = 1, modphong = 2, gouraud = 3};

// Begin with a solid human model
int projection = ortographic;
int dispmode = shade;
int lightMovement = fixed;
int reflection = phong;
int shading = phong;

GLfloat scaleFactor = 1.0;
GLfloat shininess = 5.0;
vec3 viewer_pos( 0.0, 0.0, 0.0 );
vec3 lastPos(0.0, 0.0, 0.0);
vec3 theta(0.0, -25.0, 8.0);
vec3 lastTheta = theta;

point4 light1_position( 2.0, 0.0, 2.0, 1.0 );		// Point from Right
point4 light2_position( -2.0, 0.0, 2.0, 0.0 );		// Directional from Left

// Initialize shader lighting parameters
color4 light_ambient( 0.4, 0.4, 0.4, 1.0 );
color4 light_diffuse( 0.7, 0.7, 0.7, 1.0 );
color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

color4 material_ambient( 1.0, 0.7, 0.0, 1.0 );
color4 material_diffuse( 1.0, 0.9, 0.0, 1.0 );
color4 material_specular( 1.0, 1.0, 1.0, 1.0 );

// Model-view and projection matrices uniform location
GLuint ModelView, Projection, PointOn, DirectionalOn, DispMode, Shininess, AmbientProduct, DiffuseProduct, SpecularProduct, LightMovement, Shading, Reflection;

bool pointOn = true;
bool directionalOn = false;

// Generate a triangle for the three points specified
int NumPoints = 0;
void triangle(int a, int b, int c){
	tex_coords[NumPoints] = textures[a]; normals[NumPoints] = normalvectors[a]; points[NumPoints] = vertices[a]; NumPoints++;
	tex_coords[NumPoints] = textures[b]; normals[NumPoints] = normalvectors[b]; points[NumPoints] = vertices[b]; NumPoints++;
	tex_coords[NumPoints] = textures[c]; normals[NumPoints] = normalvectors[c]; points[NumPoints] = vertices[c]; NumPoints++;
}

// Read the file and fill the vertices and faces arrays
void parseOffx(const char* filename){
	int vindex = 0, findex = 0, nindex = 0, tindex = 0;
	ifstream fin(filename);
	if(!fin.good()){
		printf("Error: File not found.");
		return;
	}
	
	while (!fin.eof()) {
		
		char buf[MAX_CHARS_PER_LINE];
		fin.getline(buf, MAX_CHARS_PER_LINE, '\n');
		
		// parse the line
		char* token = strtok(buf, DELIMITER);
		if (token){ // zero if line is blank
			// Skip the OFFX line
			if(strcmp(token,"OFFX\r") != 0){
				float val, val2, val3;
				// Read the first three numbers
				if(strcmp(token, "vt") == 0){
					val = (float) strtod(strtok(NULL,DELIMITER), NULL);
					val2 = (float) strtod(strtok(NULL,DELIMITER), NULL);
					textures[tindex++] = vec2(val,1-val2);
				}else if(strcmp(token, "vn") == 0){
					val = (float) strtod(strtok(NULL,DELIMITER), NULL);
					val2 = (float) strtod(strtok(NULL,DELIMITER), NULL);
					val3 = (float) strtod(strtok(NULL,DELIMITER), NULL);
					normalvectors[nindex++] = vec3(val,val2,val3);
				}else{
					val = (float) strtod(token, NULL);
					val2 = (float) strtod(strtok(NULL,DELIMITER), NULL);
					val3 = (float) strtod(strtok(NULL,DELIMITER), NULL);
					if(val > 3){	// If val > 3, we are at the second line
						NumVertices = val;
						NumFaces = val2;
						// Allocate arrays according to the sizes that are read
						sizeOfPoints = NumFaces * 3 * sizeof(point4);
						sizeOfTextures = NumFaces * 3 * sizeof(vec2);
						vertices = (point4*)malloc(NumVertices * sizeof(point4));
						normals = (vec3*)malloc(NumFaces * 3 * sizeof(vec3));
						normalvectors = (vec3*)malloc(NumVertices * sizeof(vec3));
						tex_coords = (vec2*)malloc(NumFaces * 3 * sizeof(vec2));
						textures = (vec2*)malloc(NumVertices * sizeof(vec2));
						points = (point4*)malloc(sizeOfPoints);
						faces = (vec3*)malloc(NumFaces * sizeof(vec3));
//						printf("NumVertices: %d\nNumFaces: %d\n",NumVertices,NumFaces);
					}else if(val == 3){		// If val == 3, then we are reading faces
						float val4 = (float) strtod(strtok(NULL,DELIMITER), NULL);
						faces[findex++] = vec3(val2,val3,val4);
					}else{					// Else we are reading vertices
						vertices[vindex++] = point4(val,val2,val3,1.0);
					}
				}
			}
			
		}
		
	}
	
	
}

void parsePPM(const char* filename){
	FILE *fd;
	int n, m, k, nm;
	char c, b[100];
	int red, green, blue;
	fd = fopen(filename, "r");
	fscanf(fd,"%[^\n] ",b);
	
	if(b[0]!='P'|| b[1] != '3'){
		printf("%s is not a PPM file!\n", b);
		return;
	}
	
	fscanf(fd, "%c",&c);
	while(c == '#'){
		fscanf(fd, "%[^\n] ", b);
		fscanf(fd, "%c",&c);
	}
	ungetc(c,fd);
	
	fscanf(fd, "%d %d %d", &n, &m, &k);
	printf("%d rows, %d columns, max value = %d\n", n, m, k);
	TextureSize = n;
	
	nm = n * m;
	image = (GLubyte*)malloc(nm * 3 * sizeof(GLubyte));
	
	for(int i = 0; i<nm; i++){
		fscanf(fd,"%d %d %d", &red, &green, &blue );
		image[3 * i]	 = red;
		image[3 * i + 1] = green;
		image[3 * i + 2] = blue;
	}
	
}

// Form triangle faces according to the parsed human model file
void humanmodel(){
	parsePPM("texture.ppm");
	parseOffx("shapeX.offx");
	for(int i=0; i<NumFaces; i++){
		triangle(faces[i].x,faces[i].y,faces[i].z);
	}
}

// OpenGL initialization
void init(){
	
	// Load the human model
	humanmodel();
	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
				 GL_RGB, GL_UNSIGNED_BYTE, image );
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR); //try here different alternatives
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //try here different alternatives
	
	// Create a vertex array object
	GLuint vao;
	glGenVertexArraysAPPLE( 1, &vao );
	glBindVertexArrayAPPLE( vao );
	
	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, 2 * sizeOfPoints + sizeOfTextures, NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeOfPoints, points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeOfPoints, sizeOfPoints, normals );
	glBufferSubData( GL_ARRAY_BUFFER, 2 * sizeOfPoints, sizeOfTextures, tex_coords );
	
	// Load shaders and use the resulting shader program
	GLuint program = InitShader( "vshader_p.glsl", "fshader_p.glsl" );
	
	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
						  BUFFER_OFFSET(0) );
	
	GLuint vNormal = glGetAttribLocation( program, "vNormal" );
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
						  BUFFER_OFFSET(sizeOfPoints) );
	
	GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
						  BUFFER_OFFSET(2*sizeOfPoints) );

	
	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;
	
	glUniform4fv( glGetUniformLocation(program, "Light1Position"),
				 1, light1_position );
	glUniform4fv( glGetUniformLocation(program, "Light2Position"),
				 1, light2_position );
	
	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");
	PointOn = glGetUniformLocation(program, "pointOn");
	DirectionalOn = glGetUniformLocation(program, "directionalOn");
	DispMode = glGetUniformLocation(program, "dispMode");
	Shininess = glGetUniformLocation(program, "Shininess");
	AmbientProduct = glGetUniformLocation(program, "AmbientProduct");
	DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
	SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
	LightMovement = glGetUniformLocation(program, "lightMovement");
	Shading = glGetUniformLocation(program, "shading");
	Reflection = glGetUniformLocation(program, "reflection");
	
	glUniform4fv( AmbientProduct,  1, ambient_product );
	glUniform4fv( DiffuseProduct,  1, diffuse_product );
	glUniform4fv( SpecularProduct, 1, specular_product );
	
	glUniform1i(PointOn, pointOn);
	glUniform1i(DirectionalOn, directionalOn);
	glUniform1i(DispMode, dispmode);
	glUniform1f(Shininess, shininess);
	glUniform1i(LightMovement, lightMovement);
	glUniform1i(Shading, shading);
	glUniform1i(Reflection, reflection);
	
	glUseProgram( program );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//  Generate tha model-view matrix
	const vec3 displacement = vec3(-0.2, -0.08, 0.0);
	mat4 model_view = (Scale(scaleFactor, scaleFactor, scaleFactor) *
					   Translate(displacement - viewer_pos) *
					   RotateX( theta.x ) *
					   RotateY( theta.y ) *
					   RotateZ( theta.z ) );
	
	glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
	
	if(dispmode == wireframe) glPolygonMode(GL_FRONT, GL_LINE);
	else glPolygonMode(GL_FRONT, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
	
	glutSwapBuffers();
}

// Keyboard keypress functions
void keyboard(unsigned char key, int x, int y){
	
	switch (key) {
			// Reset the rotation to 0,0 and center the object in the middle
		case 'i':
			theta = lastTheta = vec3(0.0, -25.0, 8.0);
			break;
			
			// "Zoom-in" to the object
		case 'z':
			if(projection == ortographic) scaleFactor *= 1.1;
			else viewer_pos -= vec3(0.0, 0.0, 0.2);
			break;
			
			// "Zoom-out" from the object
		case 'Z':
			if(projection == ortographic) scaleFactor *= 0.9;
			else viewer_pos += vec3(0.0, 0.0, 0.2);
			break;
			
			// Print help instructions
		case 'h':
			printf("HELP: \n Drag with the left mouse button to rotate \n Up and Down arrows -- set the object rotation around X-axis \n Left and Right arrows -- set the object rotation around Z-axis \n i -- initialize rotation \n z -- zoom-in, Z -- zoom-out \n q -- quit (exit) the program\n");
			break;
			
			// Exit the application
		case 'q':
			exit(EXIT_SUCCESS);
			break;
			
		default:
			break;
	}
	glutPostRedisplay();
}

//  IMPORTANT: Most of the following two methods (trackball_ptov and rotate)
//	have been taken from the example codes of the textbook in Chapter 3, "track.cpp".
//	However, they have been modified significantly to fit the assignment.
vec3 trackball_ptov(int x, int y, int width, int height){
	float d, a;
	vec3 v;
	
	// project x,y onto a hemi-sphere centered within width, height
	v.x = (2.0 * x - width) / width;
	v.y = (height - 2.0 * y) / height;
	
	// ensure that we are inside the circle on the z = 0 plane
	d = sqrt(v.x * v.x + v.y * v.y);
	if(d < 1.0) v.z = cos((M_PI/2.0) * d);
	else v.z = 0.0;
	
	a = 1.0 / sqrt(dot(v,v));
	v *= a;
	
	return v;
}

void rotate(int x, int y){
	vec3 curPos = trackball_ptov(x, y, width, height);
	if(trackingMouse){
		float a = dot(curPos,curPos);
		
		//check if mouse moved
		if (a){
			// slow down rotation if needed by changing speed
			float speed = 1.1;
			angle = speed * (M_PI/2.0) * sqrt(a);
			
			// compute and normalize rotation direction vector
			theta = sin(angle/2.0) * (lastTheta / sin(angle/2.0) + 100 * cross(lastPos, curPos));
		}
	}
	glutPostRedisplay();
}

// Mouse click functions
void mouse(int btn, int state, int x, int y){
	// Reset the rotations and then position the object under the mouse on left click
	if(btn == GLUT_LEFT_BUTTON) switch(state){
		case GLUT_DOWN:
			trackingMouse = true;
			lastPos = trackball_ptov(x, y, width, height);
			break;
		case GLUT_UP:
			trackingMouse = false;
			lastTheta = theta;
			break;
	}
	glutPostRedisplay();
}

// Adjust rotation parameters on directional button presses
void special(int key, int x, int y){
	switch (key) {
		case GLUT_KEY_UP:
			theta.x -= 3.0;
			
			if (theta.x > 360.0){
				theta.x -= 360.0;
			}
			break;
			
		case GLUT_KEY_DOWN:
			theta.x += 3.0;
			
			if (theta.x < -360.0){
				theta.x += 360.0;
			}
			break;
			
		case GLUT_KEY_LEFT:
			theta.y -= 3.0;
			
			if (theta.y > 360.0){
				theta.y -= 360.0;
			}
			break;
			
		case GLUT_KEY_RIGHT:
			theta.y += 3.0;
			
			if (theta.y < -360.0){
				theta.y += 360.0;
			}
			break;
			
		default:
			break;
	}
	lastTheta = theta;
	glutPostRedisplay();
}

// Reshape the application window, preserving the object aspect ratio
void reshape(int w, int h){
	width = w;
	height = h;
	glViewport( 0, 0, w, h );
	GLfloat aspect = GLfloat(w) / h;
	
	mat4 prj;
	switch (projection) {
			
		case ortographic:
			viewer_pos.z = 0.0;
			if (w <= h)
				prj = Ortho(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect, -1.0, 1.0);
			else
				prj = Ortho(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0, -1.0, 1.0);
			break;
			
		case perspective:
			scaleFactor = 1.0;
			viewer_pos.z = 2.5;
			prj = Perspective( 45.0, aspect, 0.5, 5.0 );
			break;
			
		default:
			break;
			
	}
	glUniformMatrix4fv( Projection, 1, GL_TRUE, prj );
}

// Main right-click menu
void right_menu(int id){}

// "Projection" submenu (Ortographic or Perspective)
void projection_menu(int id){
	projection = id;
	reshape(width, height);
	glutPostRedisplay();
}

// "Color" submenu
void color_menu(int id){
	float ambient_coeff = 0.7;
	float diffuse_coeff = 0.9;
	
	color4 color;
	
	switch (id) {
		case red:
			color = color4( 1.0, 0.0, 0.0, 1.0 );
			break;
			
		case green:
			color = color4( 0.0, 1.0, 0.0, 1.0 );
			break;
			
		case blue:
			color = color4( 0.0, 0.0, 1.0, 1.0 );
			break;
			
		case cyan:
			color = color4( 0.0, 1.0, 1.0, 1.0 );
			break;
			
		case magenta:
			color = color4( 1.0, 0.0, 1.0, 1.0 );
			break;
			
		case yellow:
			color = color4( 1.0, 1.0, 0.0, 1.0 );
			break;
			
		case white:
			color = color4( 1.0, 1.0, 1.0, 1.0 );
			break;
			
		case black:
			color = color4( 0.1, 0.1, 0.1, 1.0 );
			color = color4( 0.2, 0.2, 0.2, 1.0 );
			break;
			
		default:
			color = color4( 1.0, 1.0, 1.0, 1.0 );
			break;
	}
	
	material_ambient = color * ambient_coeff;
	material_diffuse = color * diffuse_coeff;
	
	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	
	glUniform4fv( AmbientProduct,  1, ambient_product );
	glUniform4fv( DiffuseProduct,  1, diffuse_product );
	
	glutPostRedisplay();
}

// "Display" submenu (Wireframe, Shading and Texture)
void display_menu(int id){
	dispmode = id;
	if(dispmode == both) color_menu(white);
	glUniform1i(DispMode, dispmode);
	glutPostRedisplay();
}

// Phong or Gouraud Shading submenu
void shading_menu(int id){
	shading = id;
	glUniform1i(Shading, shading);
	glutPostRedisplay();
}

// Reflection Model submenu (Phong vs. Modified Phong)
void reflection_menu(int id){
	reflection = id;
	glUniform1i(Reflection, reflection);
	glutPostRedisplay();
}

// "Toggle Lights" submenu
void toggle_lights_menu(int id){
	switch(id){
		case point:
			pointOn = !pointOn;
			glUniform1i(PointOn, pointOn);
			break;
		case directional:
			directionalOn = !directionalOn;
			glUniform1i(DirectionalOn, directionalOn);
			break;
		default:
			break;
	}
	glutPostRedisplay();
}

// Fixed vs. Moving light sources submenu
void light_movement_menu(int id){
	lightMovement = id;
	glUniform1i(LightMovement, lightMovement);
	glutPostRedisplay();
}

// "Material" submenu (Plastic and Metallic)
void material_menu(int id){
	switch (id) {
		case plastic:
			shininess = 10000.0;
			break;
		case metallic:
			shininess = 5.0;
			break;
	}
	glUniform1f(Shininess, shininess);
	glutPostRedisplay();
}

// "Background Color" submenu
void back_menu(int id){
	switch (id) {
			
		case white:
			glClearColor(1.0, 1.0, 1.0, 1.0);
			break;
			
		case gray:
			glClearColor(0.5, 0.5, 0.5, 1.0);
			break;
			
		case black:
			glClearColor(0.0, 0.0, 0.0, 1.0);
			break;
			
		default:
			break;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv){
	int p_menu, d_menu, s_menu, r_menu, tl_menu, lm_menu, m_menu, c_menu, b_menu;
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(320, 180);
	glutCreateWindow("Project");
	
	init();
	
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutSpecialFunc(special);
	glutReshapeFunc(reshape);
	glutMotionFunc(rotate);
	
	p_menu = glutCreateMenu(projection_menu);
	glutAddMenuEntry("Ortographic", ortographic);
	glutAddMenuEntry("Perspective", perspective);
	d_menu = glutCreateMenu(display_menu);
	glutAddMenuEntry("Wireframe", wireframe);
	glutAddMenuEntry("Shading", shade);
	glutAddMenuEntry("Texture", texture);
	glutAddMenuEntry("Shading + Texture", both);
	s_menu = glutCreateMenu(shading_menu);
	glutAddMenuEntry("Phong", phong);
	glutAddMenuEntry("Gouraud", gouraud);
	r_menu = glutCreateMenu(reflection_menu);
	glutAddMenuEntry("Phong", phong);
	glutAddMenuEntry("Modified Phong", modphong);
	tl_menu = glutCreateMenu(toggle_lights_menu);
	glutAddMenuEntry("Point", point);
	glutAddMenuEntry("Directional", directional);
	lm_menu = glutCreateMenu(light_movement_menu);
	glutAddMenuEntry("Fixed", fixed);
	glutAddMenuEntry("Following", moving);
	m_menu = glutCreateMenu(material_menu);
	glutAddMenuEntry("Plastic", plastic);
	glutAddMenuEntry("Metallic", metallic);
	c_menu = glutCreateMenu(color_menu);
	glutAddMenuEntry("Red", red);
	glutAddMenuEntry("Green", green);
	glutAddMenuEntry("Blue", blue);
	glutAddMenuEntry("Cyan", cyan);
	glutAddMenuEntry("Magenta", magenta);
	glutAddMenuEntry("Yellow", yellow);
	glutAddMenuEntry("White", white);
	glutAddMenuEntry("Black", black);
	b_menu = glutCreateMenu(back_menu);
	glutAddMenuEntry("White", white);
	glutAddMenuEntry("Gray", gray);
	glutAddMenuEntry("Black", black);
	
	glutCreateMenu(right_menu);
	glutAddSubMenu("Projection", p_menu);
	glutAddSubMenu("Display Mode", d_menu);
	glutAddSubMenu("Shading", s_menu);
	glutAddSubMenu("Reflection", r_menu);
	glutAddSubMenu("Toggle Lights", tl_menu);
	glutAddSubMenu("Light Movement", lm_menu);
	glutAddSubMenu("Material", m_menu);
	glutAddSubMenu("Color", c_menu);
	glutAddSubMenu("Background Color", b_menu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
	return 0;
	
}