#include "app.h"
//#include "camera.h"


static const char *g_screenquad_vert =
	"#version 440 core\n"
	"layout(location = 0) in vec3 vertex;\n"
	"layout(location = 1) in vec3 normal;\n"
	"layout(location = 2) in vec3 texcoord;\n"
	"uniform vec4 uCoords;\n"
	"uniform vec2 uScreen;\n"
	"out vec3 vtc;\n"
	"void main() {\n"
	"   vtc = texcoord*0.5+0.5;\n"
	"   gl_Position = vec4( -1.0 + (uCoords.x/uScreen.x) + (vertex.x+1.0f)*(uCoords.z-uCoords.x)/uScreen.x,\n"
	"                       -1.0 + (uCoords.y/uScreen.y) + (vertex.y+1.0f)*(uCoords.w-uCoords.y)/uScreen.y,\n"
	"                       0.0f, 1.0f );\n"
	"}\n";

static const char *g_screenquad_frag =
	"#version 440\n"
	"uniform sampler2D uTex1;\n"
	"uniform sampler2D uTex2;\n"
	"uniform int uTexFlags;\n"
	"in vec3 vtc;\n"
	"out vec4 outColor;\n"
	"void main() {\n"
	"   vec4 op1 = ((uTexFlags & 0x01)==0) ? texture ( uTex1, vtc.xy) : texture ( uTex1, vec2(vtc.x, 1.0-vtc.y));\n"
	"   if ( (uTexFlags & 0x02) != 0 ) {\n"
	"		vec4 op2 = ((uTexFlags & 0x04)==0) ? texture ( uTex2, vtc.xy) : texture ( uTex2, vec2(vtc.x, 1.0-vtc.y));\n"
	"		outColor = vec4( op1.xyz*(1.0-op2.w) + op2.xyz * op2.w, 1 );\n"
	"   } else { \n"
	"		outColor = vec4( op1.xyz, 1 );\n"
	"   }\n"
	"}\n";


GvdbApp::GvdbApp(): m_initialized(false), m_framecount(0)
{

}

GvdbApp::~GvdbApp() {

}

void GvdbApp::init(int w, int h) {

	// init openGL
	initGL();

	// GVDB
	m_width = w;
	m_height = h;
    gl_screen_tex = -1;	
	m_pretrans.Set(-125, -160, -125);
	m_scale.Set(1, 1, 1);
	m_angs.Set(0, 0, 0);
	m_trans.Set(0, 0, 0);	

	// Initialize GVDB	
	gvdb.SetVerbose ( true );
	//gvdb.SetCudaDevice ( GVDB_DEV_FIRST );
	gvdb.SetCudaDevice(-1);
	gvdb.Initialize ();
	gvdb.AddPath ( "./data/" );
	gvdb.AddPath ( "./" );
	//gvdb.AddPath ( ASSET_PATH );

    // Load VBX
	char scnpath[1024];		
	if ( !gvdb.getScene()->FindFile ( "explosion.vbx", scnpath ) ) {
		//nvprintf ( "Cannot find vbx file.\n" );
		//nverror();
        printf("cannot find file");
        exit(1);
	}
	printf ( "Loading VBX. %s\n", scnpath );
	gvdb.SetChannelDefault ( 16, 16, 16 );
	gvdb.LoadVBX ( scnpath );	

	// Set volume params
	gvdb.SetTransform(m_pretrans, m_scale, m_angs, m_trans);
	gvdb.getScene()->SetSteps ( .25, 16, .25 );				// Set raycasting steps
	gvdb.getScene()->SetExtinct ( -1.0f, 1.0f, 0.0f );		// Set volume extinction
	gvdb.getScene()->SetVolumeRange ( 0.1f, 0.0f, .5f );	// Set volume value range
	gvdb.getScene()->SetCutoff ( 0.005f, 0.005f, 0.0f );
	gvdb.getScene()->SetBackgroundClr ( 0.1f, 0.2f, 0.4f, 1.0 );
	gvdb.getScene()->LinearTransferFunc(0.00f, 0.25f, Vector4DF(0, 0, 0, 0), Vector4DF(1, 0, 0, 0.05f));
	gvdb.getScene()->LinearTransferFunc(0.25f, 0.50f, Vector4DF(1, 0, 0, 0.05f), Vector4DF(1, .5f, 0, 0.1f));
	gvdb.getScene()->LinearTransferFunc(0.50f, 0.75f, Vector4DF(1, .5f, 0, 0.1f), Vector4DF(1, 1, 0, 0.15f));
	gvdb.getScene()->LinearTransferFunc(0.75f, 1.00f, Vector4DF(1, 1, 0, 0.15f), Vector4DF(1, 1, 1, 0.2f));
	gvdb.CommitTransferFunc ();

	// Create Camera 
	Camera3D* cam = new Camera3D;	
	cam->setFov ( 50.0 );
	//cam->setOrbit ( Vector3DF(20,30,0), Vector3DF(0,0,0), 700, 1.0 );	
	gvdb.getScene()->SetCamera( cam );
	
	// Create Light
	Light* lgt = new Light;								
	lgt->setOrbit ( Vector3DF(299,57.3f,0), Vector3DF(132,-20,50), 200, 1.0 );
	gvdb.getScene()->SetLight ( 0, lgt );	

	// Add render buffer
	printf ( "Creating screen buffer. %d x %d\n", w, h );
	gvdb.AddRenderBuf ( 0, w, h, 4 );	

	// Create opengl texture for display
	// This is a helper func in sample utils (not part of gvdb),
	// which creates or resizes an opengl 2D texture.
	createScreenQuadGL ( &gl_screen_tex, w, h );

	m_initialized = true;
}

void printMatrix(const char* name, const float m[16]) {
	printf("%s ", name);
	for(int i=0; i < 16; i++) {
		printf("%0.2f ", m[i]);
	}
	printf("\n");
}

void GvdbApp::display(const float V[16], const float P[16], const float campos[3]) {
	if(!m_initialized) return;

	m_framecount++;

	Camera3D* cam = gvdb.getScene()->getCamera();
	Vector3DF pos;
	//pos.Set(campos[0], campos[1], campos[2]);
	pos.Set(0, 0, 0);
	cam->setMatrices(V, P, pos);
	
	/*
	if(m_framecount % 60 == 0)  {
		//printf("%0.4f, %0.4f, %0.4f\n", cam->from_pos.x, cam->from_pos.y, cam->from_pos.z);
		//printMatrix("V", V);
		//printMatrix("P", P);
		printf("campos: %0.2f %0.2f %0.2f\n", campos[0], campos[1], campos[2]);
		cam->view_matrix.Print();
		cam->proj_matrix.Print();
	}
	*/
	
	gvdb.Render ( SHADE_VOLUME, 0, 0 );

	// Copy render buffer into opengl texture
	// This function does a gpu-gpu device copy from the gvdb cuda output buffer
	// into the opengl texture, avoiding the cpu readback found in ReadRenderBuf
	gvdb.ReadRenderTexGL ( 0, gl_screen_tex );

	// Render screen-space quad with texture
	// This is a helper func in sample utils (not part of gvdb),
	// which renders an opengl 2D texture to the screen.
	renderScreenQuadGL ( gl_screen_tex, 0, m_width, m_height );
}

void GvdbApp::checkGL( char* msg ) {
	GLenum errCode;
    //const GLubyte* errString;
    errCode = glGetError();
    if (errCode != GL_NO_ERROR) {
		const char * message = "";
		switch( errCode )
		{
		case GL_INVALID_ENUM:
			message = "Invalid enum";
			break;
		case GL_INVALID_VALUE:
			message = "Invalid value";
			break;
		case GL_INVALID_OPERATION:
			message = "Invalid operation";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			message = "Invalid framebuffer operation";
			break;
		case GL_OUT_OF_MEMORY:
			message = "Out of memory";
			break;
		default:
			message = "Unknown error";
		}

        //printf ( "%s, ERROR: %s\n", msg, gluErrorString(errCode) );
		printf ( "%s %s\n", msg, message );
    }
}

void GvdbApp::initGL() {
	initScreenQuadGL();
	glFinish();
}

void GvdbApp::initScreenQuadGL() {
	int status;
	int maxLog = 65536, lenLog;
	char log[65536];

	// Create a screen-space shader
	m_screenquad_prog = (int)glCreateProgram();
	GLuint vShader = (int)glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, (const GLchar**)&g_screenquad_vert, NULL);
	glCompileShader(vShader);
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(vShader, maxLog, &lenLog, log);
		printf("*** Compile Error in init_screenquad vShader\n");
		printf("  %s\n", log);
	}

	GLuint fShader = (int)glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, (const GLchar**)&g_screenquad_frag, NULL);
	glCompileShader(fShader);
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(fShader, maxLog, &lenLog, log);
		printf("*** Compile Error in init_screenquad fShader\n");
		printf("  %s\n", log);
	}
	glAttachShader(m_screenquad_prog, vShader);
	glAttachShader(m_screenquad_prog, fShader);
	glLinkProgram(m_screenquad_prog);
	glGetProgramiv(m_screenquad_prog, GL_LINK_STATUS, &status);
	if (!status) {
		printf("*** Error! Failed to link in init_screenquad\n");
	}
	checkGL ( "glLinkProgram (init_screenquad)" );
	
	// Get texture parameter
	m_screenquad_utex1 = glGetUniformLocation (m_screenquad_prog, "uTex1" );
	m_screenquad_utex2 = glGetUniformLocation (m_screenquad_prog, "uTex2");
	m_screenquad_utexflags = glGetUniformLocation(m_screenquad_prog, "uTexFlags");
	m_screenquad_ucoords = glGetUniformLocation ( m_screenquad_prog, "uCoords" );
	m_screenquad_uscreen = glGetUniformLocation ( m_screenquad_prog, "uScreen" );


	// Create a screen-space quad VBO
	std::vector<nvVertex> verts;
	std::vector<nvFace> faces;
	verts.push_back(nvVertex(-1, -1, 0, -1, 1, 0));
	verts.push_back(nvVertex(1, -1, 0, 1, 1, 0));
	verts.push_back(nvVertex(1, 1, 0, 1, -1, 0));
	verts.push_back(nvVertex(-1, 1, 0, -1, -1, 0));
	faces.push_back(nvFace(0, 1, 2));
	faces.push_back(nvFace(2, 3, 0));

	glGenBuffers(1, (GLuint*)&m_screenquad_vbo[0]);
	glGenBuffers(1, (GLuint*)&m_screenquad_vbo[1]);
	checkGL("glGenBuffers (init_screenquad)");
	glGenVertexArrays(1, (GLuint*)&m_screenquad_vbo[2]);
	glBindVertexArray(m_screenquad_vbo[2]);
	checkGL("glGenVertexArrays (init_screenquad)");
	glBindBuffer(GL_ARRAY_BUFFER, m_screenquad_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(nvVertex), &verts[0].x, GL_STATIC_DRAW_ARB);
	checkGL("glBufferData[V] (init_screenquad)");
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(nvVertex), 0);				// pos
	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(nvVertex), (void*)12);	// norm
	glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(nvVertex), (void*)24);	// texcoord
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_screenquad_vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * 3 * sizeof(int), &faces[0].a, GL_STATIC_DRAW_ARB);
	checkGL("glBufferData[F] (init_screenquad)");
	glBindVertexArray(0);
}

void GvdbApp::createScreenQuadGL ( int* glid, int w, int h ) {
	if ( *glid == -1 ) glDeleteTextures ( 1, (GLuint*) glid );
	glGenTextures ( 1, (GLuint*) glid );
	glBindTexture ( GL_TEXTURE_2D, *glid );
	checkGL ( "glBindTexture (createScreenQuadGL)" );
	glPixelStorei ( GL_UNPACK_ALIGNMENT, 4 );	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);	
	glTexImage2D  ( GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);	
	checkGL ( "glTexImage2D (createScreenQuadGL)" );
	glBindTexture ( GL_TEXTURE_2D, 0 );
}

void GvdbApp::renderScreenQuadGL ( int glid1, int glid2, float x1, float y1, 
									float x2, float y2, char inv1, char inv2 ) {
	// Prepare pipeline
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	// Select shader	
	glBindVertexArray(m_screenquad_vbo[2]);
	glUseProgram(m_screenquad_prog);
	checkGL("glUseProgram");
	// Select VBO	
	glBindBuffer(GL_ARRAY_BUFFER, m_screenquad_vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(nvVertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(nvVertex), (void*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(nvVertex), (void*)24);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_screenquad_vbo[1]);
	checkGL("glBindBuffer");
	// Select texture
	
	//glEnable ( GL_TEXTURE_2D );
	
	glProgramUniform4f ( m_screenquad_prog, m_screenquad_ucoords, x1, y1, x2, y2 );
	glProgramUniform2f ( m_screenquad_prog, m_screenquad_uscreen, x2, y2 );
	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D, glid1 );
	
	glProgramUniform1i(m_screenquad_prog, m_screenquad_utex1, 0);
	int flags = 0;
	if (inv1 > 0) flags |= 1;												// y-invert tex1

	if (glid2 >= 0) {
		flags |= 2;															// enable tex2 compositing
		if (inv2 > 0) flags |= 4;											// y-invert tex2
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glid2);
		glProgramUniform1i(m_screenquad_prog, m_screenquad_utex2, 1);
	}

	glProgramUniform1i(m_screenquad_prog, m_screenquad_utexflags, flags );	

	// Draw
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1);
	
	checkGL("glDraw");
	glUseProgram(0);

	glDepthMask(GL_TRUE);
}

void GvdbApp::renderScreenQuadGL( int glid, char inv1, int w, int h ) {
	renderScreenQuadGL ( glid, -1, (float)0, (float)0, (float)w, (float)h, inv1, 0); 
}