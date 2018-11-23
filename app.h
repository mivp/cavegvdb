#ifndef __GVDB_APP_H
#define __GVDB_APP_H

// GVDB library
#include "gvdb.h"			
using namespace nvdb;

#ifdef OMEGALIB_MODULE
#include <omegaGl.h>
#else
#include "stdapp/GLInclude.h"
#endif

struct nvVertex {
	nvVertex(float x1, float y1, float z1, float tx1, float ty1, float tz1) { x=x1; y=y1; z=z1; tx=tx1; ty=ty1; tz=tz1; }
	float	x, y, z;
	float	nx, ny, nz;
	float	tx, ty, tz;
};
struct nvFace {
	nvFace(unsigned int x1, unsigned int y1, unsigned int z1) { a=x1; b=y1; c=z1; }
	unsigned int  a, b, c;
};

class GvdbApp {

protected:
    void checkGL( char* msg );
    void initGL();
    void initScreenQuadGL();
    void createScreenQuadGL ( int* glid, int w, int h );
    void renderScreenQuadGL ( int glid1, int glid2, float x1, float y1, float x2, float y2, char inv1, char inv2 );
    void renderScreenQuadGL( int glid, char inv1, int w, int h );

public:
    GvdbApp();
    ~GvdbApp();

    void init(int w, int h);
    void display(const float V[16], const float P[16], const float pos[3]);

    bool m_initialized;
    VolumeGVDB	gvdb;
    int			gl_screen_tex;
	Vector3DF	m_pretrans, m_scale, m_angs, m_trans;

private:
    int         m_width, m_height;
    int			m_screenquad_prog;
    int			m_screenquad_vshader;
    int			m_screenquad_fshader;
    int			m_screenquad_vbo[3];
    int			m_screenquad_utex1;
    int			m_screenquad_utex2;
    int			m_screenquad_utexflags;
    int			m_screenquad_ucoords;
    int			m_screenquad_uscreen;

    int m_framecount;
};


#endif