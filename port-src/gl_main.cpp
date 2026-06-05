// port-src/gl_main.cpp — native-Linux Minecraft client over the ported engine.
// Title screen, options menu, HUD/hotbar/crosshair (real font/logo/gui atlases), walking + gravity
// + AABB collision with a fly toggle, captured low-sensitivity mouse look, day/night, live ticking,
// entities, and streamed per-chunk meshing. Pure GL/X11/png TU (no engine stdafx — header clash).
#include <GL/glx.h>
#include <GL/gl.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XInput2.h>    // raw relative mouse (correct FPS look, no warp jitter)
#include <png.h>
#include <unistd.h>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include "gl_engine.h"

static int W=1280, H=720;
static const int CH=128;
static double g_lookDX=0, g_lookDY=0;   // accumulated raw mouse motion (XI2), applied per frame

// ============================ textures ============================
struct Tex { GLuint id=0; int w=0,h=0; };
static Tex TEX_TERRAIN, TEX_FONT, TEX_LOGO, TEX_GUI, TEX_ICONS, TEX_ITEMS;
static unsigned char FONT_W[256];   // proportional glyph widths (px within an 8px cell)

static Tex loadTex(const char* path, unsigned char** keepPixels=nullptr, int* kw=nullptr, int* kh=nullptr){
    Tex t; FILE* fp=fopen(path,"rb"); if(!fp){ printf("[gl] missing %s\n",path); return t; }
    png_structp p=png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0); png_infop info=png_create_info_struct(p);
    if(setjmp(png_jmpbuf(p))){ fclose(fp); return t; }
    png_init_io(p,fp); png_read_png(p,info,PNG_TRANSFORM_EXPAND|PNG_TRANSFORM_GRAY_TO_RGB,0);
    int w=png_get_image_width(p,info),h=png_get_image_height(p,info),ch=png_get_channels(p,info);
    png_bytepp rows=png_get_rows(p,info);
    unsigned char* px=(unsigned char*)malloc(w*h*4);
    for(int y=0;y<h;y++)for(int x=0;x<w;x++){unsigned char*s=&rows[y][x*ch];unsigned char*d=&px[(y*w+x)*4];
        d[0]=s[0];d[1]=ch>1?s[1]:s[0];d[2]=ch>2?s[2]:s[0];d[3]=ch>3?s[3]:255;}
    png_destroy_read_struct(&p,&info,0); fclose(fp);
    glGenTextures(1,&t.id); glBindTexture(GL_TEXTURE_2D,t.id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,px);
    t.w=w; t.h=h;
    if(keepPixels){ *keepPixels=px; if(kw)*kw=w; if(kh)*kh=h; } else free(px);
    return t;
}
static void computeFontWidths(unsigned char* px,int w,int h){
    int cw=w/16, chh=h/16;            // 8x8 cells for 128x128
    for(int c=0;c<256;c++){
        int cx=(c%16)*cw, cy=(c/16)*chh; int maxx=0;
        for(int yy=0;yy<chh;yy++)for(int xx=0;xx<cw;xx++){
            if(px[((cy+yy)*w+(cx+xx))*4+3] > 16){ if(xx+1>maxx) maxx=xx+1; }
        }
        FONT_W[c] = maxx>0 ? maxx+1 : (c==' '?4:cw/2);
    }
    FONT_W[' ']=4;
}

// ============================ 2D drawing ============================
static void begin2D(){
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0,W,H,0,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST); glDisable(GL_FOG); glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1,1,1,1);                 // don't inherit a stale 3D-pass colour
}
static void end2D(){
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE); glDisable(GL_BLEND); glColor4f(1,1,1,1);
}
// draw a sub-rect of a texture (atlas coords in pixels) to screen rect
static void blit(const Tex& t,float dx,float dy,float dw,float dh,float sx,float sy,float sw,float sh){
    glBindTexture(GL_TEXTURE_2D,t.id); glEnable(GL_TEXTURE_2D);
    float u0=sx/t.w,v0=sy/t.h,u1=(sx+sw)/t.w,v1=(sy+sh)/t.h;
    glBegin(GL_QUADS);
    glTexCoord2f(u0,v0);glVertex2f(dx,dy); glTexCoord2f(u1,v0);glVertex2f(dx+dw,dy);
    glTexCoord2f(u1,v1);glVertex2f(dx+dw,dy+dh); glTexCoord2f(u0,v1);glVertex2f(dx,dy+dh);
    glEnd();
}
static float textW(const std::string& s,float scale){ float w=0; for(unsigned char c:s) w+=(FONT_W[c]+1)*scale; return w; }
static void drawTextC(const std::string& s,float x,float y,float scale,float r,float g,float b){
    glBindTexture(GL_TEXTURE_2D,TEX_FONT.id); glEnable(GL_TEXTURE_2D); glColor3f(r,g,b);
    int cw=TEX_FONT.w/16, chh=TEX_FONT.h/16; float px=x;
    glBegin(GL_QUADS);
    for(unsigned char c:s){
        int gx=(c%16)*cw, gy=(c/16)*chh; int gw=FONT_W[c];
        float u0=gx/(float)TEX_FONT.w,v0=gy/(float)TEX_FONT.h,u1=(gx+gw)/(float)TEX_FONT.w,v1=(gy+chh)/(float)TEX_FONT.h;
        float dw=gw*scale, dh=chh*scale;
        glTexCoord2f(u0,v0);glVertex2f(px,y); glTexCoord2f(u1,v0);glVertex2f(px+dw,y);
        glTexCoord2f(u1,v1);glVertex2f(px+dw,y+dh); glTexCoord2f(u0,v1);glVertex2f(px,y+dh);
        px += (gw+1)*scale;
    }
    glEnd();
}
static void drawText(const std::string& s,float x,float y,float scale,float r=1,float g=1,float b=1){
    drawTextC(s,x+scale,y+scale,scale,r*0.25f,g*0.25f,b*0.25f);   // shadow
    drawTextC(s,x,y,scale,r,g,b);
}
static void fillRect(float x,float y,float w,float h,float r,float g,float b,float a){
    glDisable(GL_TEXTURE_2D); glColor4f(r,g,b,a);
    glBegin(GL_QUADS); glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h); glEnd();
    glEnable(GL_TEXTURE_2D);
}

// ============================ block atlas tiles ============================
// Full block-id -> terrain.png (col,row) mapping, classic Beta/console atlas layout, derived from
// the engine's Tile::*_Id constants. face: 0=top,1=bottom,2..5=sides.
static void tileFor(int id,int face,int& col,int& row){
    bool top=(face==0), bot=(face==1);
    switch(id){
        case 1:  col=1;row=0; break;                                     // stone
        case 2:  if(top){col=0;row=0;} else if(bot){col=2;row=0;} else {col=3;row=0;} break; // grass
        case 3:  col=2;row=0; break;                                     // dirt
        case 4:  col=0;row=1; break;                                     // cobblestone
        case 5:  col=4;row=0; break;                                     // planks
        case 6:  col=15;row=0; break;                                    // sapling
        case 7:  col=1;row=1; break;                                     // bedrock
        case 8: case 9:  col=13;row=12; break;                           // water
        case 10: case 11: col=13;row=14; break;                          // lava
        case 12: col=2;row=1; break;                                     // sand
        case 13: col=3;row=1; break;                                     // gravel
        case 14: col=0;row=2; break;                                     // gold ore
        case 15: col=1;row=2; break;                                     // iron ore
        case 16: col=2;row=2; break;                                     // coal ore
        case 17: if(top||bot){col=5;row=1;} else {col=4;row=1;} break;   // log
        case 18: col=4;row=3; break;                                     // leaves
        case 19: col=0;row=3; break;                                     // sponge
        case 20: col=1;row=3; break;                                     // glass
        case 21: col=0;row=10; break;                                    // lapis ore
        case 22: col=0;row=9; break;                                     // lapis block
        case 23: if(top){col=14;row=3;} else if(face==2){col=14;row=2;} else {col=13;row=2;} break; // dispenser
        case 24: if(top){col=0;row=11;} else if(bot){col=0;row=13;} else {col=0;row=12;} break;     // sandstone
        case 25: col=10;row=4; break;                                    // note block
        case 35: col=0;row=4; break;                                     // wool (white)
        case 37: col=13;row=0; break;                                    // dandelion
        case 38: col=12;row=0; break;                                    // rose
        case 41: col=7;row=1; break;                                     // gold block
        case 42: col=6;row=1; break;                                     // iron block
        case 43: case 44: if(top||bot){col=6;row=0;} else {col=5;row=0;} break; // stone slab
        case 45: col=7;row=0; break;                                     // brick
        case 46: if(top){col=9;row=0;} else if(bot){col=10;row=0;} else {col=8;row=0;} break; // tnt
        case 47: if(top||bot){col=4;row=0;} else {col=3;row=2;} break;   // bookshelf
        case 48: col=4;row=2; break;                                     // mossy cobble
        case 49: col=5;row=2; break;                                     // obsidian
        case 50: col=0;row=5; break;                                     // torch
        case 52: col=1;row=4; break;                                     // mob spawner
        case 53: col=4;row=0; break;                                     // wood stairs
        case 54: if(top){col=9;row=1;} else if(face==3){col=11;row=1;} else {col=10;row=1;} break; // chest
        case 55: col=4;row=10; break;                                    // redstone dust
        case 56: col=2;row=3; break;                                     // diamond ore
        case 57: col=8;row=1; break;                                     // diamond block
        case 58: if(top){col=11;row=2;} else if(face==2||face==4){col=11;row=3;} else {col=12;row=3;} break; // workbench
        case 59: col=15;row=5; break;                                    // crops
        case 60: if(top){col=6;row=5;} else {col=2;row=0;} break;        // farmland
        case 61: if(top){col=14;row=3;} else if(face==3){col=12;row=2;} else {col=13;row=2;} break; // furnace
        case 62: if(top){col=14;row=3;} else if(face==3){col=13;row=3;} else {col=13;row=2;} break; // lit furnace
        case 65: col=3;row=5; break;                                     // ladder
        case 73: case 74: col=3;row=3; break;                            // redstone ore
        case 78: col=2;row=4; break;                                     // snow layer
        case 79: col=3;row=4; break;                                     // ice
        case 80: col=2;row=4; break;                                     // snow block
        case 81: if(top){col=5;row=4;} else if(bot){col=7;row=4;} else {col=6;row=4;} break; // cactus
        case 82: col=8;row=4; break;                                     // clay
        case 83: col=9;row=4; break;                                     // sugar cane
        case 31: case 32: col=7;row=2; break;                            // tall grass / dead bush
        default: col=1;row=0; break;                                     // unknown -> stone
    }
}
static inline bool isAir(int t){ return t==0; }
static inline bool isLiquid(int t){ return t==8||t==9||t==10||t==11; }
// transparent / non-cube blocks don't hide neighbour faces
static inline bool occludes(int t){
    switch(t){ case 0: case 6: case 8: case 9: case 10: case 11: case 18: case 20:
        case 31: case 32: case 37: case 38: case 39: case 40: case 44: case 50: case 51: case 55: case 59:
        case 65: case 66: case 78: case 79: case 83: return false; }
    return true;
}
static inline bool isSolidForPlayer(int t){           // collidable (incl. glass/ice/leaves)
    switch(t){ case 0: case 6: case 8: case 9: case 10: case 11: case 31: case 32:
        case 37: case 38: case 39: case 40: case 50: case 51: case 55: case 59: case 66: case 83: return false; }
    return true;
}

static inline bool isTranslucent(int t){ return t==8||t==9||t==20||t==79; } // water/glass/ice
static inline float alphaFor(int t){ if(t==8||t==9)return 0.64f; if(t==79)return 0.55f; if(t==20)return 0.80f; return 1.0f; }
// non-cube render shapes
enum RClass { RC_CUBE, RC_CROSS, RC_SLAB, RC_TORCH, RC_LAYER };
static inline RClass rclass(int id){
    switch(id){
        case 6: case 31: case 32: case 37: case 38: case 39: case 40: case 59: case 83: return RC_CROSS; // plants
        case 44: return RC_SLAB;        // single stone slab
        case 50: return RC_TORCH;       // torch
        case 78: return RC_LAYER;       // snow layer
        default: return RC_CUBE;
    }
}

// ============================ chunk meshing (display lists) ============================
struct ChunkMesh { GLuint list=0; GLuint tlist=0; };  // opaque + translucent passes
static std::unordered_map<long long,ChunkMesh> g_chunks;
static inline long long ckey(int cx,int cz){ return ((long long)(unsigned)cx<<32)|(unsigned)cz; }
static const float FACE[6][4][3]={
    {{0,1,0},{0,1,1},{1,1,1},{1,1,0}},{{0,0,0},{1,0,0},{1,0,1},{0,0,1}},
    {{1,0,0},{1,1,0},{0,1,0},{0,0,0}},{{0,0,1},{0,1,1},{1,1,1},{1,0,1}},
    {{0,0,0},{0,1,0},{0,1,1},{0,0,1}},{{1,0,1},{1,1,1},{1,1,0},{1,0,0}},
};
static const int NB[6][3]={{0,1,0},{0,-1,0},{0,0,-1},{0,0,1},{-1,0,0},{1,0,0}};
static const float DIRSHADE[6]={1.0f,0.5f,0.8f,0.8f,0.6f,0.6f};
// in-plane tangent axes for each face (0=x,1=y,2=z) — the plane perpendicular to NB[f]
static const int TAN[6][2]={{0,2},{0,2},{0,1},{0,1},{1,2},{1,2}};
static inline float Bn(int x,int y,int z){ float b=engine_getBrightness(x,y,z)/15.0f; return b<0.16f?0.16f:b; }
// Smooth lighting + implicit ambient occlusion, replicated from TileRenderer.cpp:
// per-vertex brightness = average of the 4 neighbour-plane cells touching that corner
// (ll1 = (llxyZ+llxy0+ll0yZ+ll0y0)/4). Solid blocks in the plane lower a corner -> AO crease.
static void emitFace(int id,int f,int x,int y,int z,float a){
    int col,row; tileFor(id,f,col,row); const float T=1/16.0f;
    float u0=col*T,v0=row*T,u1=u0+T,v1=v0+T; const float uv[4][2]={{u0,v1},{u0,v0},{u1,v0},{u1,v1}};
    int bx=x+NB[f][0], by=y+NB[f][1], bz=z+NB[f][2];          // neighbour plane origin
    int a1=TAN[f][0], a2=TAN[f][1];
    int t1[3]={0,0,0}, t2[3]={0,0,0}; t1[a1]=1; t2[a2]=1;     // tangent unit vectors
    float grid[3][3];                                         // 3x3 brightness sample of the plane
    for(int i=-1;i<=1;i++)for(int j=-1;j<=1;j++)
        grid[i+1][j+1]=Bn(bx+i*t1[0]+j*t2[0], by+i*t1[1]+j*t2[1], bz+i*t1[2]+j*t2[2]);
    float tr=1,tg=1,tb=1;
    if((id==2&&f==0)||id==18){ tr=0.49f;tg=0.78f;tb=0.34f; }  // biome green tint
    if(id==8||id==9){ tr=0.35f;tg=0.52f;tb=0.95f; }           // water blue tint
    float ds=DIRSHADE[f];
    for(int i=0;i<4;i++){
        int s1=FACE[f][i][a1]>0.5f?1:-1, s2=FACE[f][i][a2]>0.5f?1:-1;
        float vb=(grid[1][1]+grid[1+s1][1]+grid[1][1+s2]+grid[1+s1][1+s2])*0.25f;  // 4-corner average
        float s=ds*vb;
        glColor4f(s*tr,s*tg,s*tb,a); glTexCoord2f(uv[i][0],uv[i][1]);
        glVertex3f(x+FACE[f][i][0],y+FACE[f][i][1],z+FACE[f][i][2]); }
}
// one face of an arbitrary sub-cube box [x0..x1]x[y0..y1]x[z0..z1] (local 0..1), full-tile UV
static void emitFaceG(int id,int f,int x,int y,int z,float x0,float y0,float z0,float x1,float y1,float z1){
    int col,row; tileFor(id,f,col,row); const float T=1/16.0f;
    float u0=col*T,v0=row*T,u1=u0+T,v1=v0+T; const float uv[4][2]={{u0,v1},{u0,v0},{u1,v0},{u1,v1}};
    float light=engine_getBrightness(x+NB[f][0],y+NB[f][1],z+NB[f][2])/15.0f; if(light<0.22f)light=0.22f;
    float s=DIRSHADE[f]*light;
    for(int i=0;i<4;i++){ glColor4f(s,s,s,1.0f); glTexCoord2f(uv[i][0],uv[i][1]);
        glVertex3f(x+x0+FACE[f][i][0]*(x1-x0), y+y0+FACE[f][i][1]*(y1-y0), z+z0+FACE[f][i][2]*(z1-z0)); }
}
static void emitBoxAll(int id,int x,int y,int z,float x0,float y0,float z0,float x1,float y1,float z1){
    for(int f=0;f<6;f++) emitFaceG(id,f,x,y,z,x0,y0,z0,x1,y1,z1);
}
// two crossed, double-sided quads — flowers/saplings/crops/grass/sugar cane
static void emitCross(int id,int x,int y,int z){
    int col,row; tileFor(id,2,col,row); const float T=1/16.0f;
    float u0=col*T,v0=row*T,u1=u0+T,v1=v0+T;
    float light=engine_getBrightness(x,y,z)/15.0f; if(light<0.25f)light=0.25f;
    float r=light,g=light,b=light;
    if(id==31||id==32||id==6){ r=light*0.50f; g=light*0.85f; b=light*0.32f; }   // foliage tint
    const float q[2][4][3]={{{0,0,0},{1,0,1},{1,1,1},{0,1,0}},{{1,0,0},{0,0,1},{0,1,1},{1,1,0}}};
    const float uv[4][2]={{u0,v1},{u1,v1},{u1,v0},{u0,v0}};
    for(int d=0;d<2;d++){
        for(int i=0;i<4;i++){ glColor4f(r,g,b,1.0f); glTexCoord2f(uv[i][0],uv[i][1]); glVertex3f(x+q[d][i][0],y+q[d][i][1],z+q[d][i][2]); }
        for(int i=3;i>=0;i--){ glColor4f(r,g,b,1.0f); glTexCoord2f(uv[i][0],uv[i][1]); glVertex3f(x+q[d][i][0],y+q[d][i][1],z+q[d][i][2]); }
    }
}
static void meshChunk(int cx,int cz){
    ChunkMesh& cm=g_chunks[ckey(cx,cz)];
    if(!cm.list) cm.list=glGenLists(1);
    if(!cm.tlist) cm.tlist=glGenLists(1);
    int bx=cx*16,bz=cz*16;
    // opaque pass
    glNewList(cm.list,GL_COMPILE); glBegin(GL_QUADS);
    for(int lx=0;lx<16;lx++)for(int lz=0;lz<16;lz++){ int x=bx+lx,z=bz+lz;
        for(int y=0;y<CH;y++){ int id=engine_getTile(x,y,z); if(!id||isTranslucent(id))continue;
            RClass rc=rclass(id);
            if(rc==RC_CROSS){ emitCross(id,x,y,z); continue; }
            if(rc==RC_TORCH){ emitBoxAll(id,x,y,z, 0.4375f,0,0.4375f, 0.5625f,0.625f,0.5625f); continue; }
            if(rc==RC_SLAB||rc==RC_LAYER){ float h=(rc==RC_LAYER)?0.125f:0.5f;
                for(int f=0;f<6;f++){ int nt=engine_getTile(x+NB[f][0],y+NB[f][1],z+NB[f][2]);
                    bool show=(f==0)?true:!occludes(nt);            // top always; others if neighbour open
                    if(show) emitFaceG(id,f,x,y,z, 0,0,0, 1,h,1); }
                continue; }
            for(int f=0;f<6;f++){ int nt=engine_getTile(x+NB[f][0],y+NB[f][1],z+NB[f][2]);
                bool show=isLiquid(id)?isAir(nt):!occludes(nt); if(show)emitFace(id,f,x,y,z,1.0f); } } }
    glEnd(); glEndList();
    // translucent pass (water/glass/ice) — shared faces between same id are hidden
    glNewList(cm.tlist,GL_COMPILE); glBegin(GL_QUADS);
    for(int lx=0;lx<16;lx++)for(int lz=0;lz<16;lz++){ int x=bx+lx,z=bz+lz;
        for(int y=0;y<CH;y++){ int id=engine_getTile(x,y,z); if(!isTranslucent(id))continue;
            for(int f=0;f<6;f++){ int nt=engine_getTile(x+NB[f][0],y+NB[f][1],z+NB[f][2]);
                bool show = isLiquid(id)? isAir(nt) : (nt!=id && !occludes(nt));
                if(show)emitFace(id,f,x,y,z,alphaFor(id)); } } }
    glEnd(); glEndList();
}
static void markDirty(int x,int z){ int cx=x>>4,cz=z>>4;
    for(int dx=-1;dx<=1;dx++)for(int dz=-1;dz<=1;dz++){ auto it=g_chunks.find(ckey(cx+dx,cz+dz)); if(it!=g_chunks.end())meshChunk(cx+dx,cz+dz);} }

// ============================ camera / player ============================
struct Player { float x,y,z; float vx=0,vy=0,vz=0; float yaw=45,pitch=-10; bool onGround=false; bool fly=true; };
static void viewDir(const Player& p,float& dx,float& dy,float& dz){
    float cp=cosf(p.pitch*M_PI/180),sp=sinf(p.pitch*M_PI/180),cy=cosf(p.yaw*M_PI/180),sy=sinf(p.yaw*M_PI/180);
    dx=-sy*cp; dy=sp; dz=-cy*cp;
}
static void perspective(float fovy,float asp,float zn,float zf){
    float f=1/tanf(fovy*M_PI/360);
    float m[16]={f/asp,0,0,0,0,f,0,0,0,0,(zf+zn)/(zn-zf),-1,0,0,(2*zf*zn)/(zn-zf),0};
    glMatrixMode(GL_PROJECTION); glLoadMatrixf(m);
}
// player collision: AABB 0.6x1.8 centred on x,z; y is feet
static const float PW=0.3f, PHh=1.8f;
static bool boxHitsWorld(float x,float y,float z){
    int x0=(int)floorf(x-PW),x1=(int)floorf(x+PW),y0=(int)floorf(y),y1=(int)floorf(y+PHh),z0=(int)floorf(z-PW),z1=(int)floorf(z+PW);
    for(int bx=x0;bx<=x1;bx++)for(int by=y0;by<=y1;by++)for(int bz=z0;bz<=z1;bz++)
        if(isSolidForPlayer(engine_getTile(bx,by,bz))) return true;
    return false;
}
static const float STEP=0.6f;   // Minecraft auto step-up height (slabs/snow/path/0.6 ledges)
// climb the SMALLEST height (≤STEP) that clears, so stepping is smooth, not a 0.6 pop; refuse if none clears
static bool stepUp(Player& p,float nx,float nz){
    if(!p.onGround) return false;
    for(float s=0.1f; s<=STEP+1e-4f; s+=0.1f)
        if(!boxHitsWorld(nx,p.y+s,nz) && !boxHitsWorld(p.x,p.y+s,p.z)){ p.x=nx; p.z=nz; p.y+=s; return true; }
    return false;
}
static void moveAxis(Player& p,float dx,float dy,float dz){
    // sub-step large displacements (sprint can exceed 0.5/step) so we never tunnel through a 1-thick wall
    float m=fmaxf(fabsf(dx),fmaxf(fabsf(dy),fabsf(dz)));
    if(m>0.4f){ int n=(int)ceilf(m/0.4f); for(int i=0;i<n;i++) moveAxis(p,dx/n,dy/n,dz/n); return; }
    if(dx!=0){ if(!boxHitsWorld(p.x+dx,p.y,p.z)) p.x+=dx; else if(!stepUp(p,p.x+dx,p.z)) p.vx=0; }
    if(dz!=0){ if(!boxHitsWorld(p.x,p.y,p.z+dz)) p.z+=dz; else if(!stepUp(p,p.x,p.z+dz)) p.vz=0; }
    if(dy!=0){ float ny=p.y+dy; if(!boxHitsWorld(p.x,ny,p.z))p.y=ny; else { if(dy<0)p.onGround=true; p.vy=0; } }
}
// ONE 50ms physics tick — the single source of truth used by PLAY, --video and --playtest.
static void stepPhysics(Player& p,float fwd,float strafe,float up,bool jump,float sprint,int sy){
    float yaw=p.yaw*M_PI/180; float fxv=-sinf(yaw),fzv=-cosf(yaw),rxv=cosf(yaw),rzv=-sinf(yaw);
    if(p.fly){
        float sp=0.5f*sprint; p.x+=(fxv*fwd+rxv*strafe)*sp; p.z+=(fzv*fwd+rzv*strafe)*sp; p.y+=up*sp; p.vy=0; p.onGround=false;
    } else {
        bool inWater=isLiquid(engine_getTile((int)floorf(p.x),(int)floorf(p.y+0.4f),(int)floorf(p.z)));
        float acc=0.16f*sprint;
        p.vx+=(fxv*fwd+rxv*strafe)*acc; p.vz+=(fzv*fwd+rzv*strafe)*acc;
        if(inWater){ p.vy-=0.02f; p.vy*=0.80f;
            if(jump){ p.vy=0.10f;
                if(!isLiquid(engine_getTile((int)floorf(p.x),(int)floorf(p.y+1.2f),(int)floorf(p.z)))) p.vy=0.34f; } } // hop out at surface
        else       { p.vy-=0.08f; p.vy*=0.98f; }
        p.onGround=false;
        moveAxis(p,p.vx,0,0); moveAxis(p,0,0,p.vz); moveAxis(p,0,p.vy,0);
        p.vx*=0.70f; p.vz*=0.70f;
        if(jump&&p.onGround&&!inWater) p.vy=0.42f;
        if(p.y<-32){ p.y=(float)(sy+4); p.vy=0; }
    }
}
// raycast against the world: a block is selectable if non-air and not liquid (so glass/leaves/flowers count)
static inline bool selectable(int t){ return t!=0 && !isLiquid(t); }
struct RayHit { bool hit=false; int hx,hy,hz, px,py,pz; };
// Amanatides–Woo voxel DDA: exact block + the exact face neighbour for placement (~5 cell tests, not 167)
static RayHit raycast(const Player& p,float reach=5.0f){
    RayHit r; float dx,dy,dz; viewDir(p,dx,dy,dz);
    float ox=p.x, oy=p.y+1.62f, oz=p.z;
    int x=(int)floorf(ox), y=(int)floorf(oy), z=(int)floorf(oz);
    int sx=dx>0?1:-1, sy=dy>0?1:-1, sz=dz>0?1:-1;
    float tDX=dx!=0?fabsf(1.0f/dx):1e30f, tDY=dy!=0?fabsf(1.0f/dy):1e30f, tDZ=dz!=0?fabsf(1.0f/dz):1e30f;
    float tx=dx!=0?(((dx>0?x+1:x)-ox)/dx):1e30f;
    float ty=dy!=0?(((dy>0?y+1:y)-oy)/dy):1e30f;
    float tz=dz!=0?(((dz>0?z+1:z)-oz)/dz):1e30f;
    int px=x,py=y,pz=z;
    for(float t=0;t<=reach;){
        if(selectable(engine_getTile(x,y,z))){ r.hit=true; r.hx=x;r.hy=y;r.hz=z; r.px=px;r.py=py;r.pz=pz; return r; }
        px=x;py=y;pz=z;
        if(tx<ty&&tx<tz){ x+=sx; t=tx; tx+=tDX; }
        else if(ty<tz){ y+=sy; t=ty; ty+=tDY; }
        else { z+=sz; t=tz; tz+=tDZ; }
    }
    return r;
}
static bool placeWouldHitPlayer(const Player& p,int bx,int by,int bz){
    float x0=p.x-PW,x1=p.x+PW,y0=p.y,y1=p.y+PHh,z0=p.z-PW,z1=p.z+PW;
    return (bx+1>x0&&bx<x1)&&(by+1>y0&&by<y1)&&(bz+1>z0&&bz<z1);
}

// ============================ entities ============================
// ===================== textured mob box-models =====================
// Authentic replicated from *Model.cpp (QuadrupedModel/HumanoidModel/CreeperModel/
// SpiderModel/ChickenModel) + the exact Cube.cpp face unwrap and Polygon.cpp UV-corner inset.
// Model units = 1/16 block; model +Y is DOWN; ground (feet) sits at model-y = groundY.
struct MBox { float px,py,pz, baseRx; int x0,y0,z0,w,h,d, tx,ty; int swing; };
struct MModel { std::vector<MBox> parts; float groundY; const char* tex; };
static std::unordered_map<int,MModel> g_models;          // type -> model
static std::unordered_map<int,Tex>    g_mobTex;          // texname-keyed-by-type GL texture
static GLuint g_sheepWoolTex=0;                          // sheep wool overlay (sheep_fur.png)

static void buildQuadruped(int type,int L,const char* tex){
    MModel m; m.groundY=24; m.tex=tex;
    m.parts.push_back({0,(float)(12+6-L),-6, 0,      -4,-4,-8, 8,8,8,   0,0,  0});  // head
    m.parts.push_back({0,(float)(11+6-L), 2, 90.0f,  -5,-10,-7,10,16,8, 28,8, 0}); // body (xRot 90)
    m.parts.push_back({-3,(float)(18+6-L),7, 0, -2,0,-2,4,L,4, 0,16,  1});          // legs
    m.parts.push_back({ 3,(float)(18+6-L),7, 0, -2,0,-2,4,L,4, 0,16, -1});
    m.parts.push_back({-3,(float)(18+6-L),-5,0, -2,0,-2,4,L,4, 0,16, -1});
    m.parts.push_back({ 3,(float)(18+6-L),-5,0, -2,0,-2,4,L,4, 0,16,  1});
    g_models[type]=m;
}
static void buildHumanoid(int type,const char* tex){
    MModel m; m.groundY=24; m.tex=tex;
    m.parts.push_back({0,0,0, 0,   -4,-8,-4, 8,8,8,  0,0,   0});   // head
    m.parts.push_back({0,0,0, 0,   -4,0,-2,  8,12,4, 16,16, 0});  // body
    m.parts.push_back({-5,2,0,0,   -3,-2,-2, 4,12,4, 40,16,-1});  // arm0
    m.parts.push_back({ 5,2,0,0,   -1,-2,-2, 4,12,4, 40,16, 1});  // arm1
    m.parts.push_back({-1.9f,12,0,0,-2,0,-2, 4,12,4, 0,16,  1});  // leg0
    m.parts.push_back({ 1.9f,12,0,0,-2,0,-2, 4,12,4, 0,16, -1});  // leg1
    g_models[type]=m;
}
static void buildCreeper(const char* tex){
    MModel m; m.groundY=24; m.tex=tex; float yo=6;
    m.parts.push_back({0,yo,0, 0, -4,-8,-4,8,8,8, 0,0,  0});   // head
    m.parts.push_back({0,yo,0, 0, -4,0,-2, 8,12,4,16,16, 0});  // body
    m.parts.push_back({-2,12+yo,4, 0,-2,0,-2,4,6,4,0,16, 1});  // legs (6 tall)
    m.parts.push_back({ 2,12+yo,4, 0,-2,0,-2,4,6,4,0,16,-1});
    m.parts.push_back({-2,12+yo,-4,0,-2,0,-2,4,6,4,0,16,-1});
    m.parts.push_back({ 2,12+yo,-4,0,-2,0,-2,4,6,4,0,16, 1});
    g_models[ET_CREEPER]=m;
}
static void buildSpider(const char* tex){
    MModel m; m.tex=tex; float yo=15; m.groundY=yo+1;
    m.parts.push_back({0,yo,-3, 0, -4,-4,-8,8,8,8,  32,4, 0}); // head
    m.parts.push_back({0,yo,0,  0, -3,-3,-3,6,6,6,  0,0,  0}); // body0
    m.parts.push_back({0,yo,9,  0, -5,-4,-6,10,8,12,0,12, 0}); // body1 (abdomen)
    for(int i=0;i<4;i++){ float z=2-i;
        m.parts.push_back({-4,yo,z, 0, -15,-1,-1,16,2,2, 18,0, 0}); // left legs
        m.parts.push_back({ 4,yo,z, 0,  -1,-1,-1,16,2,2, 18,0, 0}); // right legs
    }
    g_models[ET_SPIDER]=m;
}
static void buildChicken(const char* tex){
    MModel m; m.groundY=24; m.tex=tex; float yo=16;
    m.parts.push_back({0,-1+yo,-4,0, -2,-6,-2,4,6,3, 0,0,  0});  // head
    m.parts.push_back({0,-1+yo,-4,0, -2,-4,-4,4,2,2, 14,0, 0});  // beak
    m.parts.push_back({0,-1+yo,-4,0, -1,-2,-3,2,2,2, 14,4, 0});  // wattle
    m.parts.push_back({0,yo,0,   90.0f, -3,-4,-3,6,8,6, 0,9, 0}); // body (xRot 90)
    m.parts.push_back({-2,3+yo,1,0, -1,0,-3,3,5,3, 26,0, 1});    // legs
    m.parts.push_back({ 1,3+yo,1,0, -1,0,-3,3,5,3, 26,0,-1});
    m.parts.push_back({-4,-3+yo,0,0,  0,0,-3,1,4,6, 24,13, 0});  // wings
    m.parts.push_back({ 4,-3+yo,0,0, -1,0,-3,1,4,6, 24,13, 0});
    g_models[ET_CHICKEN]=m;
}
static void initMobModels(){
    buildQuadruped(ET_PIG,6,"Common/res/mob/pig.png");
    buildQuadruped(ET_COW,12,"Common/res/mob/cow.png");
    buildQuadruped(ET_SHEEP,12,"Common/res/mob/sheep.png");
    buildQuadruped(ET_WOLF,8,"Common/res/mob/wolf.png");
    buildChicken("Common/res/mob/chicken.png");
    buildHumanoid(ET_VILLAGER,"Common/res/mob/char.png");
    buildHumanoid(ET_ZOMBIE,"Common/res/mob/zombie.png");
    buildHumanoid(ET_SKELETON,"Common/res/mob/skeleton.png");
    buildCreeper("Common/res/mob/creeper.png");
    buildSpider("Common/res/mob/spider.png");
    for(auto&kv:g_models) g_mobTex[kv.first]=loadTex(kv.second.tex);
    g_sheepWoolTex=loadTex("Common/res/mob/sheep_fur.png").id;   // wool overlay layer
}

// One textured quad with the Polygon.cpp corner mapping + 0.1-texel inset (64x32 atlas).
static void faceQuad(const float*a,const float*b,const float*c,const float*d,int u0,int v0,int u1,int v1){
    const float TW=64.f,TH=32.f;
    float us=(u1>u0?0.1f:-0.1f)/TW, vs=(v1>v0?0.1f:-0.1f)/TH;
    float U0=u0/TW,U1=u1/TW,V0=v0/TH,V1=v1/TH;
    glTexCoord2f(U1-us,V0+vs); glVertex3fv(a);
    glTexCoord2f(U0+us,V0+vs); glVertex3fv(b);
    glTexCoord2f(U0+us,V1-vs); glVertex3fv(c);
    glTexCoord2f(U1-us,V1-vs); glVertex3fv(d);
}
static void drawMBox(const MBox& bx,float grow){
    float x0=bx.x0-grow,y0=bx.y0-grow,z0=bx.z0-grow, x1=bx.x0+bx.w+grow,y1=bx.y0+bx.h+grow,z1=bx.z0+bx.d+grow;
    float u0[3]={x0,y0,z0},u1[3]={x1,y0,z0},u2[3]={x1,y1,z0},u3[3]={x0,y1,z0};
    float l0[3]={x0,y0,z1},l1[3]={x1,y0,z1},l2[3]={x1,y1,z1},l3[3]={x0,y1,z1};
    int tx=bx.tx,ty=bx.ty,w=bx.w,h=bx.h,d=bx.d;
    glBegin(GL_QUADS);
    faceQuad(l1,u1,u2,l2, tx+d+w,     ty+d,   tx+d+w+d,   ty+d+h); // +x
    faceQuad(u0,l0,l3,u3, tx,         ty+d,   tx+d,       ty+d+h); // -x
    faceQuad(l1,l0,u0,u1, tx+d,       ty,     tx+d+w,     ty+d);   // up
    faceQuad(u2,u3,l3,l2, tx+d+w,     ty+d,   tx+d+w+w,   ty);     // down
    faceQuad(u1,u0,u3,u2, tx+d,       ty+d,   tx+d+w,     ty+d+h); // -z (front)
    faceQuad(l0,l1,l2,l3, tx+d+w+d,   ty+d,   tx+d+w+d+w, ty+d+h); // +z (back)
    glEnd();
}
static float g_anim=0;
static void drawMobModel(const MModel& m,GLuint tex,float ex,float ey,float ez,float yaw,float light,float phase,
                         float grow=0.0f,unsigned partBits=0xFFFFFFFFu){
    glBindTexture(GL_TEXTURE_2D,tex);
    glColor3f(light,light,light);
    glPushMatrix();
    glTranslatef(ex,ey,ez);
    glRotatef(180.0f-yaw,0,1,0);
    glScalef(0.0625f,-0.0625f,0.0625f);   // model->blocks, flip Y (model down -> world up)
    glTranslatef(0,-m.groundY,0);
    float sw=sinf(g_anim*0.9f+phase)*28.0f;   // gentle leg/arm swing (degrees)
    unsigned idx=0;
    for(const MBox& b:m.parts){
        if(partBits&(1u<<idx)){
            glPushMatrix();
            glTranslatef(b.px,b.py,b.pz);
            float rx=b.baseRx + b.swing*sw;
            if(rx!=0) glRotatef(rx,1,0,0);
            drawMBox(b,grow);
            glPopMatrix();
        }
        idx++;
    }
    glPopMatrix();
}

static void drawEntities(){
    int n=engine_entitySnapshot();
    g_anim+=0.18f;
    glDisable(GL_CULL_FACE);   // negative Y scale on models flips winding
    glEnable(GL_TEXTURE_2D);
    for(int i=0;i<n;i++){ float x,y,z,w,h,yaw=0; int type;
        if(!engine_getEntity(i,&x,&y,&z,&w,&h,&type,&yaw))continue;
        auto it=g_models.find(type);
        if(it!=g_models.end()){
            int bx=(int)floorf(x),by=(int)floorf(y+h*0.5f),bz=(int)floorf(z);
            float light=engine_getBrightness(bx,by,bz)/15.0f; if(light<0.3f)light=0.3f;
            if(type==ET_SHEEP){
                // base head + legs (face/legs show), then an inflated fluffy WOOL body over the torso
                drawMobModel(it->second, g_mobTex[type].id, x,y,z, yaw, light, (i%8)*0.7f, 0.0f, ~(1u<<1));
                drawMobModel(it->second, g_sheepWoolTex,    x,y,z, yaw, light, (i%8)*0.7f, 0.6f,  (1u<<1));
                continue;
            }
            drawMobModel(it->second, g_mobTex[type].id, x,y,z, yaw, light, (i%8)*0.7f);
            continue;
        }
        // fallback: shaded box (items / unmodelled mobs)
        glDisable(GL_TEXTURE_2D);
        float r=1,g=1,b=1; if(type==ET_ITEM){r=1;g=.9f;b=.2f;if(w<.25f)w=.25f;if(h<.25f)h=.25f;} else {r=.8f;g=.8f;b=.85f;}
        float x0=x-w/2,x1=x+w/2,y0=y,y1=y+h,z0=z-w/2,z1=z+w/2;
        glColor3f(r,g,b);
        glBegin(GL_QUADS);
        glVertex3f(x0,y0,z0);glVertex3f(x1,y0,z0);glVertex3f(x1,y1,z0);glVertex3f(x0,y1,z0);
        glVertex3f(x0,y0,z1);glVertex3f(x0,y1,z1);glVertex3f(x1,y1,z1);glVertex3f(x1,y0,z1);
        glVertex3f(x0,y0,z0);glVertex3f(x0,y1,z0);glVertex3f(x0,y1,z1);glVertex3f(x0,y0,z1);
        glVertex3f(x1,y0,z0);glVertex3f(x1,y0,z1);glVertex3f(x1,y1,z1);glVertex3f(x1,y1,z0);
        glVertex3f(x0,y1,z0);glVertex3f(x1,y1,z0);glVertex3f(x1,y1,z1);glVertex3f(x0,y1,z1);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }
    glEnable(GL_CULL_FACE);
    glColor3f(1,1,1);
}

// ============================ options ============================
struct Options { int renderDist=6; float sens=0.055f; int fov=70; int fly=1; };  // sens tuned down (was 0.09, "too high")
static Options OPT;

static void skyColor(float day,float& r,float& g,float& b){
    if(day<0)day=0; if(day>1)day=1; float dusk=4*day*(1-day);
    r=.04f+day*.58f+dusk*.20f; g=.05f+day*.70f+dusk*.06f; b=.10f+day*.90f;
}
static GLuint g_dummy=0;
static int g_meshCenterX=999999,g_meshCenterZ=999999;
static void streamChunks(int ccx,int ccz,int budget){
    int R=OPT.renderDist;
    for(int r=0;r<=R&&budget>0;r++)for(int dx=-r;dx<=r&&budget>0;dx++)for(int dz=-r;dz<=r&&budget>0;dz++){
        if(std::max(abs(dx),abs(dz))!=r) continue;
        int cx=ccx+dx,cz=ccz+dz; if(g_chunks.count(ckey(cx,cz))) continue;
        engine_ensureChunk(cx,cz); meshChunk(cx,cz); budget--;
    }
    // evict far chunks (free display lists) so memory/FPS stay bounded + render-dist shrink takes effect
    for(auto it=g_chunks.begin(); it!=g_chunks.end();){
        int cx=(int)(int32_t)(it->first>>32), cz=(int)(int32_t)(it->first&0xffffffffu);
        if(std::max(abs(cx-ccx),abs(cz-ccz))>R+3){   // wide dead-band: no evict/regen thrash when pacing a border
            if(it->second.list)  glDeleteLists(it->second.list,1);
            if(it->second.tlist) glDeleteLists(it->second.tlist,1);
            it=g_chunks.erase(it);
        } else ++it;
    }
}
static RayHit g_look;   // current crosshair target (recomputed each PLAY frame)
static void drawSelection(const RayHit& h){
    if(!h.hit) return;
    glDisable(GL_TEXTURE_2D); glDisable(GL_FOG); glDepthMask(GL_FALSE);
    glColor4f(0,0,0,0.4f); glLineWidth(2.0f);
    float e=0.002f, x0=h.hx-e,y0=h.hy-e,z0=h.hz-e,x1=h.hx+1+e,y1=h.hy+1+e,z1=h.hz+1+e;
    glBegin(GL_LINES);
    float c[8][3]={{x0,y0,z0},{x1,y0,z0},{x1,y0,z1},{x0,y0,z1},{x0,y1,z0},{x1,y1,z0},{x1,y1,z1},{x0,y1,z1}};
    int E[12][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    for(auto&ed:E){ glVertex3fv(c[ed[0]]); glVertex3fv(c[ed[1]]); }
    glEnd();
    glDepthMask(GL_TRUE); glEnable(GL_TEXTURE_2D); glEnable(GL_FOG); glColor4f(1,1,1,1);
}
// sun / moon discs + night stars, billboarded to the camera, drawn at "infinity" (no fog, depth off)
static void drawCelestial(const Player& p,float day){
    float ry=p.yaw*M_PI/180;
    float fwd[3]; viewDir(p,fwd[0],fwd[1],fwd[2]);
    float rgt[3]={cosf(ry),0,-sinf(ry)};                 // screen-right (⊥ forward, horizontal)
    float up[3]={ rgt[1]*fwd[2]-rgt[2]*fwd[1],            // screen-up = right × forward
                  rgt[2]*fwd[0]-rgt[0]*fwd[2],
                  rgt[0]*fwd[1]-rgt[1]*fwd[0] };
    long long t=engine_getTimeRaw()%24000; if(t<0)t+=24000;
    float phi=(float)((t-6000)/24000.0*2*M_PI);     // 0 -> sun overhead at tick 6000
    float sdx=sinf(phi), sdy=cosf(phi);             // sun direction (z=0 plane)
    float ex=p.x, ey=p.y+1.62f, ez=p.z, D=120;
    glDisable(GL_FOG); glDepthMask(GL_FALSE); glDisable(GL_TEXTURE_2D);
    auto disc=[&](float dirx,float diry,float dirz,float sz,float r,float g,float b,float a){
        if(a<=0)return; float cx=ex+dirx*D,cy=ey+diry*D,cz=ez+dirz*D;
        glColor4f(r,g,b,a); glBegin(GL_QUADS);
        glVertex3f(cx-rgt[0]*sz-up[0]*sz,cy-rgt[1]*sz-up[1]*sz,cz-rgt[2]*sz-up[2]*sz);
        glVertex3f(cx+rgt[0]*sz-up[0]*sz,cy+rgt[1]*sz-up[1]*sz,cz+rgt[2]*sz-up[2]*sz);
        glVertex3f(cx+rgt[0]*sz+up[0]*sz,cy+rgt[1]*sz+up[1]*sz,cz+rgt[2]*sz+up[2]*sz);
        glVertex3f(cx-rgt[0]*sz+up[0]*sz,cy-rgt[1]*sz+up[1]*sz,cz-rgt[2]*sz+up[2]*sz);
        glEnd();
    };
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    // stars at night (cheap deterministic field opposite the sun-lit sky)
    if(day<0.5f){ float sa=(0.5f-day)*2.0f; glColor4f(1,1,1,sa*0.8f); glPointSize(2.0f); glBegin(GL_POINTS);
        unsigned s=12345; for(int i=0;i<200;i++){ s=s*1103515245+12345; float a1=((s>>9)&1023)/1023.0f*6.2832f;
            s=s*1103515245+12345; float a2=((s>>9)&1023)/1023.0f-0.5f; float cy=sinf(a2*3.0f);
            float cx=cosf(a1)*cosf(a2*3.0f),cz=sinf(a1)*cosf(a2*3.0f);
            if(cy>0) glVertex3f(ex+cx*D,ey+cy*D,ez+cz*D); }
        glEnd(); }
    disc(sdx, sdy, 0,  10, 1.0f,0.96f,0.72f, day>0.05f?1.0f:0.0f);          // sun
    disc(-sdx,-sdy, 0,  7,  0.92f,0.92f,1.0f, day<0.6f?(0.6f-day)*1.6f:0.0f); // moon
    glDepthMask(GL_TRUE); glEnable(GL_FOG); glEnable(GL_TEXTURE_2D); glColor4f(1,1,1,1);
}
static void renderWorld(const Player& p,float day){
    float sr,sg,sb; skyColor(day,sr,sg,sb);
    glClearColor(sr,sg,sb,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,W,H); perspective((float)OPT.fov,(float)W/H,0.08f,500.0f);
    GLfloat fc[4]={sr,sg,sb,1}; glFogfv(GL_FOG_COLOR,fc); glEnable(GL_FOG); glFogi(GL_FOG_MODE,GL_LINEAR);
    glFogf(GL_FOG_START,(OPT.renderDist-1)*16.0f); glFogf(GL_FOG_END,(OPT.renderDist+0.5f)*16.0f);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glRotatef(-p.pitch,1,0,0); glRotatef(-p.yaw,0,1,0);
    glTranslatef(-p.x,-(p.y+1.62f),-p.z);
    drawCelestial(p,day);
    glBindTexture(GL_TEXTURE_2D,TEX_TERRAIN.id); glEnable(GL_TEXTURE_2D); glColor4f(1,1,1,1);
    for(auto&kv:g_chunks) if(kv.second.list) glCallList(kv.second.list);   // opaque
    drawEntities();
    drawSelection(g_look);
    // translucent pass: water/glass/ice, blended, no depth write (after opaque + entities)
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST); glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_2D,TEX_TERRAIN.id); glColor4f(1,1,1,1);
    for(auto&kv:g_chunks) if(kv.second.tlist) glCallList(kv.second.tlist);
    glDepthMask(GL_TRUE); glEnable(GL_ALPHA_TEST); glDisable(GL_BLEND);
    float dark=(1-day)*0.30f;   // block brightness already darkens at night; keep night playable
    if(dark>0.01f){ begin2D(); fillRect(0,0,W,H,0,0,0.06f,dark); end2D(); }
}

// hotbar palette
static const int HOTBAR[9]={1,4,3,5,17,18,12,20,45};
static int g_sel=0;

static void drawHUD(const Player& p,int fps){
    begin2D();
    // crosshair from icons.png (top-left 16x16)
    glBlendFunc(GL_ONE_MINUS_DST_COLOR,GL_ONE_MINUS_SRC_COLOR);  // invert (classic crosshair)
    blit(TEX_ICONS, W/2-8, H/2-8, 16,16, 0,0,16,16);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    // hotbar widget from gui.png (0,0 182x22), scaled x2
    float sc=2.0f, hbw=182*sc, hbh=22*sc, hx=W/2-hbw/2, hy=H-hbh-4;
    blit(TEX_GUI, hx,hy, hbw,hbh, 0,0,182,22);
    blit(TEX_GUI, hx+ g_sel*20*sc -1*sc, hy-1*sc, 24*sc,24*sc, 0,22,24,24);   // selection
    // block icons in slots (terrain top tile)
    for(int i=0;i<9;i++){ int id=HOTBAR[i],col,row; tileFor(id,0,col,row);
        float ix=hx+(3+i*20)*sc, iy=hy+3*sc, is=16*sc;
        blit(TEX_TERRAIN, ix,iy,is,is, col*16,row*16,16,16);
        if((id==2)||id==18){ /* slight tint already via texture is grayscale; leave */ }
    }
    // text: title, coords, fps
    drawText("Minecraft (Linux port)", 6,6, 2.0f, 1,1,1);
    char buf[128];
    snprintf(buf,sizeof(buf),"XYZ: %.1f / %.1f / %.1f   fps:%d   %s", p.x,p.y,p.z, fps, p.fly?"[fly]":"[walk]");
    drawText(buf, 6,30, 1.5f, 0.9f,0.9f,0.9f);
    drawText("WASD move  Ctrl sprint  Space jump/swim  Shift down  F fly  1-9/scroll block  LMB break  RMB place  Esc menu",
             6, H-18, 1.0f, 0.8f,0.8f,0.8f);
    end2D();
}

// ============================ menus ============================
struct Button { float x,y,w,h; std::string label; };
static bool hit(const Button& b,int mx,int my){ return mx>=b.x&&mx<=b.x+b.w&&my>=b.y&&my<=b.y+b.h; }
static void drawButton(const Button& b,bool hover){
    float sy = hover? 86 : 66;               // gui.png button rows
    blit(TEX_GUI, b.x,b.y, b.w/2,b.h, 0,sy,100,20);
    blit(TEX_GUI, b.x+b.w/2,b.y, b.w/2,b.h, 200-100,sy,100,20);
    float tw=textW(b.label,2.0f);
    drawText(b.label, b.x+b.w/2-tw/2, b.y+b.h/2-8, 2.0f, 1,1,1);
}
static void drawDirtBG(){
    glBindTexture(GL_TEXTURE_2D,TEX_TERRAIN.id); glEnable(GL_TEXTURE_2D); glColor3f(0.28f,0.28f,0.28f);
    float ts=48; const float T=1/16.0f; float u0=2*T,v0=0,u1=3*T,v1=T;   // dirt tile
    glBegin(GL_QUADS);
    for(float y=0;y<H;y+=ts)for(float x=0;x<W;x+=ts){
        glTexCoord2f(u0,v0);glVertex2f(x,y); glTexCoord2f(u1,v0);glVertex2f(x+ts,y);
        glTexCoord2f(u1,v1);glVertex2f(x+ts,y+ts); glTexCoord2f(u0,v1);glVertex2f(x,y+ts);
    }
    glEnd(); glColor3f(1,1,1);
}

// classic yellow pulsing splash, rotated, anchored at the logo's lower-right
static void drawSplash(float x,float y,float pulse){
    const char* s="Now on Linux!";
    float ps=2.0f+0.12f*sinf(pulse);
    glPushMatrix(); glTranslatef(x,y,0); glRotatef(-18,0,0,1);
    drawText(s,-textW(s,ps)/2,-4*ps,ps,1.0f,1.0f,0.25f);
    glPopMatrix();
}
// the clean single-line MINECRAFT logo (gui/logo.png) is in the top-left of a 256² texture;
// blit ONLY its content rect at its true 6.12:1 aspect (was: stretching the whole texture -> squished)
static void drawLogo(bool splash,float pulse){
    const float LU=5,LV=4,LW=245,LH=40;
    float lw=W*0.52f, lh=lw*LH/LW, lx=W/2-lw/2, ly=48;
    blit(TEX_LOGO,lx,ly,lw,lh,LU,LV,LW,LH);
    if(splash) drawSplash(lx+lw*0.86f, ly+lh*1.18f, pulse);   // just below the logo's tail, not over it
}

enum State { TITLE, OPTIONS, PLAY };

// ============================ main ============================
static long long nowms(){ struct timeval tv; gettimeofday(&tv,0); return (long long)tv.tv_sec*1000+tv.tv_usec/1000; }

int main(int argc,char** argv){
    setbuf(stdout,nullptr);
    bool demo=(argc>1&&strcmp(argv[1],"--demo")==0);
    bool video=(argc>1&&strcmp(argv[1],"--video")==0);
    bool playtest=(argc>1&&strcmp(argv[1],"--playtest")==0);
    bool shots=(argc>1&&strcmp(argv[1],"--shots")==0);
    bool soak=(argc>1&&strcmp(argv[1],"--soak")==0);
    printf("=== Minecraft (native Linux) — %s ===\n", demo?"demo":(video?"video":"interactive"));
    if(!engine_boot()){ printf("boot failed\n"); return 1; }
    int sx,sy,sz; engine_spawn(&sx,&sy,&sz); engine_spawnDemoMobs(sx,sy,sz);

    Display* dpy=XOpenDisplay(0); if(!dpy){printf("no X\n");return 1;}
    int attrs[]={GLX_RGBA,GLX_DEPTH_SIZE,24,GLX_DOUBLEBUFFER,None};
    XVisualInfo* vi=glXChooseVisual(dpy,DefaultScreen(dpy),attrs); if(!vi){printf("no visual\n");return 1;}
    Window root=RootWindow(dpy,vi->screen); Colormap cmap=XCreateColormap(dpy,root,vi->visual,AllocNone);
    XSetWindowAttributes swa; swa.colormap=cmap;
    swa.event_mask=ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|StructureNotifyMask|FocusChangeMask;
    Window win=XCreateWindow(dpy,root,0,0,W,H,0,vi->depth,InputOutput,vi->visual,CWColormap|CWEventMask,&swa);
    Atom wmDelete=XInternAtom(dpy,"WM_DELETE_WINDOW",False); XSetWMProtocols(dpy,win,&wmDelete,1);
    XStoreName(dpy,win,"Minecraft (native Linux)"); XMapWindow(dpy,win);
    GLXContext ctx=glXCreateContext(dpy,vi,0,GL_TRUE); glXMakeCurrent(dpy,win,ctx);
    printf("[gl] %s | %s\n",(const char*)glGetString(GL_VERSION),(const char*)glGetString(GL_RENDERER));
    // invisible cursor for capture
    Pixmap bm=XCreatePixmap(dpy,win,1,1,1); XColor bc; memset(&bc,0,sizeof bc);
    Cursor blank=XCreatePixmapCursor(dpy,bm,bm,&bc,&bc,0,0);
    // XInput2 raw relative mouse — driver-level deltas, pre-acceleration, no warp feedback loop
    int xi_opcode=0,xi_ev=0,xi_err=0; bool xiOK=false;
    if(XQueryExtension(dpy,"XInputExtension",&xi_opcode,&xi_ev,&xi_err)){
        int xmaj=2,xmin=0;
        if(XIQueryVersion(dpy,&xmaj,&xmin)==Success){
            unsigned char mask[XIMaskLen(XI_RawMotion)]={0}; XISetMask(mask,XI_RawMotion);
            XIEventMask em; em.deviceid=XIAllMasterDevices; em.mask_len=sizeof(mask); em.mask=mask;
            XISelectEvents(dpy,root,&em,1); XFlush(dpy); xiOK=true;
            printf("[gl] XInput2 raw mouse enabled\n");
        }
    }
    if(!xiOK) printf("[gl] WARNING: XInput2 unavailable, mouse-look degraded\n");

    TEX_TERRAIN=loadTex("Common/res/terrain.png");
    unsigned char* fpx=nullptr; int fw,fh; TEX_FONT=loadTex("Common/res/font/default.png",&fpx,&fw,&fh);
    if(fpx){ computeFontWidths(fpx,fw,fh); free(fpx); }
    TEX_LOGO=loadTex("Common/res/gui/logo.png");   // clean single-line logo (not the 2-row mclogo)
    TEX_GUI=loadTex("Common/res/gui/gui.png");
    TEX_ICONS=loadTex("Common/res/gui/icons.png");
    initMobModels();

    glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glAlphaFunc(GL_GREATER,0.4f); glEnable(GL_ALPHA_TEST);

    Player pl; pl.fly=OPT.fly; pl.x=sx+0.5f; pl.z=sz+0.5f;
    { int top=sy+2; for(int y=120;y>2;--y) if(engine_getTile(sx,y,sz)!=0){ top=y+1; break; } pl.y=top; }

    State st = demo? PLAY : TITLE;
    Button bPlay{0,0,300,40,"Play Game"}, bOpt{0,0,300,40,"Options..."}, bQuit{0,0,300,40,"Quit"};
    Button bDone{0,0,300,40,"Done"};
    Button oRD{0,0,300,40,""}, oSe{0,0,300,40,""}, oFov{0,0,300,40,""}, oFly{0,0,300,40,""};
    bool keys[512]={false}; bool kShift=false, kCtrl=false; int mx=0,my=0; bool captured=false; long long lastTick=nowms(); long long frame=0;
    int fps=0,fpsCount=0; long long fpsT=nowms();

    auto layoutMenu=[&](){
        float cx=W/2-150; bPlay.x=cx;bPlay.y=H/2-30; bOpt.x=cx;bOpt.y=H/2+20; bQuit.x=cx;bQuit.y=H/2+70;
        oRD.x=cx;oRD.y=H/2-90; oSe.x=cx;oSe.y=H/2-40; oFov.x=cx;oFov.y=H/2+10; oFly.x=cx;oFly.y=H/2+60;
        bDone.x=cx;bDone.y=H/2+120;
    };
    auto setCapture=[&](bool on){
        captured=on;
        if(on){ XDefineCursor(dpy,win,blank);
            XGrabPointer(dpy,win,True,ButtonPressMask,GrabModeAsync,GrabModeAsync,win,blank,CurrentTime);
            XWarpPointer(dpy,None,win,0,0,0,0,W/2,H/2); XSync(dpy,False);   // centre so first look-delta is ~0
            g_lookDX=g_lookDY=0; }
        else  { XUngrabPointer(dpy,CurrentTime); XUndefineCursor(dpy,win); }
    };
    if(st==PLAY) setCapture(true);

    // pre-mesh a small bubble around spawn (fast startup); the rest streams in as you move
    streamChunks(((int)pl.x)>>4, ((int)pl.z)>>4, 49);
    float spawnY=pl.y;

    if(shots){
        // Comprehensive visual battery: render the game in many states/angles, dump labelled PNGs to inspect.
        auto cap=[&](const char* path){
            std::vector<unsigned char> px(W*H*3); glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,px.data());
            FILE* fp=fopen(path,"wb"); png_structp p=png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
            png_infop info=png_create_info_struct(p); png_init_io(p,fp);
            png_set_IHDR(p,info,W,H,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
            png_write_info(p,info); std::vector<png_bytep> r(H); for(int y=0;y<H;y++)r[y]=&px[(H-1-y)*W*3];
            png_write_image(p,r.data()); png_write_end(p,0); png_destroy_write_struct(&p,&info); fclose(fp); printf("[shots] %s\n",path);
        };
        layoutMenu();
        // 1: title  (capture the BACK buffer BEFORE swapping — reading after swap is undefined on Mesa)
        glViewport(0,0,W,H); begin2D(); drawDirtBG();
        { drawLogo(true,5.0f);
          drawButton(bPlay,true); drawButton(bOpt,false); drawButton(bQuit,false);
          drawText("Minecraft Linux Edition",W/2-260,H-28,1.6f,0.85f,0.9f,1.0f); }
        end2D(); cap("shot_1_title.png"); glXSwapBuffers(dpy,win);
        // 2: options
        begin2D(); drawDirtBG(); drawText("Options",W/2-60,40,3.0f,1,1,1);
        { char b1[64],b2[64],b3[64],b4[64];
          snprintf(b1,64,"Render Distance: %d",OPT.renderDist); snprintf(b2,64,"Mouse Sensitivity: %d%%",(int)((OPT.sens-0.02f)/0.38f*100));
          snprintf(b3,64,"FOV: %d",OPT.fov); snprintf(b4,64,"Movement: %s",OPT.fly?"Fly":"Walk");
          oRD.label=b1;oSe.label=b2;oFov.label=b3;oFly.label=b4;
          drawButton(oRD,false);drawButton(oSe,true);drawButton(oFov,false);drawButton(oFly,false);drawButton(bDone,false);
          drawText("left-click +   right-click -",W/2-140,H/2+170,1.4f,0.8f,0.8f,0.8f); }
        end2D(); cap("shot_2_options.png"); glXSwapBuffers(dpy,win);
        // gameplay: stream around the camera before EVERY shot; capture before swap
        pl.fly=true; engine_setTime(6000);
        auto shot=[&](const char* path,float yaw,float pitch,bool hud){
            pl.yaw=yaw; pl.pitch=pitch;
            streamChunks(((int)floorf(pl.x))>>4,((int)floorf(pl.z))>>4,260);
            g_look=raycast(pl);
            renderWorld(pl,engine_dayLight()); if(hud) drawHUD(pl,60); cap(path); glXSwapBuffers(dpy,win);
        };
        for(int t=0;t<5;t++) engine_tick();
        // pitch sweep at spawn — isolate the "black when looking down" bug
        shot("shot_3_pitch_-85.png",30,-85,true);
        shot("shot_4_pitch_-45.png",30,-45,true);
        shot("shot_5_pitch_000.png",30,  0,true);
        shot("shot_6_pitch_+45.png",30, 45,false);
        shot("shot_7_pitch_+85.png",30, 85,false);
        // mob close-ups (find a specific species, frame it from ~2.2m)
        auto mobShot=[&](const char* path,int want){
            int n=engine_entitySnapshot(); float mx=0,my=0,mz=0; bool found=false;
            for(int i=0;i<n;i++){ float x,y,z,w,h,yw;int ty; if(engine_getEntity(i,&x,&y,&z,&w,&h,&ty,&yw)&&ty==want){mx=x;my=y;mz=z;found=true;break;} }
            if(found){ pl.x=mx+2.0f; pl.z=mz+2.0f; pl.y=my+0.9f;
                float yaw=atan2f(-(mx-pl.x),-(mz-pl.z))*180.0f/M_PI; shot(path,yaw,-8,false); }
            else printf("[shots] no mob type %d in world\n",want);
        };
        mobShot("shot_8_sheep.png",ET_SHEEP);
        mobShot("shot_9_pig.png",ET_PIG);
        mobShot("shot_10_cow.png",ET_COW);
        // block faces: a 2x2 wall directly in front of a fixed camera (foolproof framing) -> see all faces
        { int by=(int)spawnY+4;
          engine_setTile(sx,by+1,sz-4,1);  engine_setTile(sx+1,by+1,sz-4,2);    // top row: stone, grass
          engine_setTile(sx,by,  sz-4,17); engine_setTile(sx+1,by,  sz-4,12);   // bottom row: log, sand
          markDirty(sx,sz-4);
          pl.x=sx+1.0f; pl.z=sz-1.0f; pl.y=by+0.6f;
          printf("[shots] blocks at y=%d, cam=(%.1f,%.1f,%.1f)\n",by,pl.x,pl.y,pl.z);
          shot("shot_11_blocks.png", 10, -4, false); }   // yaw 10 -> a sliver of the side faces too
        // night at spawn
        pl.x=sx+0.5f; pl.z=sz+0.5f; pl.y=spawnY+2; engine_setTime(18000); for(int t=0;t<5;t++) engine_tick();
        shot("shot_12_night.png",30,-8,true);
        printf("[shots] done\n");
        XUngrabPointer(dpy,CurrentTime); return 0;
    }

    if(soak){
        // Full ~5-minute playthrough: do EVERYTHING, capture a labelled frame at each activity.
        int fc=0;
        auto cap=[&](const char* label){
            char path[160]; snprintf(path,sizeof path,"soak_%02d_%s.png",fc++,label);
            std::vector<unsigned char> px((size_t)W*H*3); glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,px.data());
            FILE* f=fopen(path,"wb"); png_structp p=png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
            png_infop info=png_create_info_struct(p); png_init_io(p,f);
            png_set_IHDR(p,info,W,H,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
            png_write_info(p,info); std::vector<png_bytep> r(H); for(int y=0;y<H;y++)r[y]=&px[(H-1-y)*W*3];
            png_write_image(p,r.data()); png_write_end(p,0); png_destroy_write_struct(&p,&info); fclose(f); printf("[soak] %s\n",path);
        };
        auto frame=[&](const char* label,bool hud){
            streamChunks(((int)floorf(pl.x))>>4,((int)floorf(pl.z))>>4,260);
            g_look=raycast(pl); renderWorld(pl,engine_dayLight()); if(hud)drawHUD(pl,60); cap(label); glXSwapBuffers(dpy,win);
        };
        auto sim=[&](float fwd,float strafe,float up,bool jump,float sprint,int ticks){
            for(int i=0;i<ticks;i++){ int cx=((int)floorf(pl.x))>>4,cz=((int)floorf(pl.z))>>4;
                engine_ensureChunk(cx,cz); streamChunks(cx,cz,8); stepPhysics(pl,fwd,strafe,up,jump,sprint,sy); engine_tick(); }
        };
        auto faceTo=[&](float tx,float tz){ float dx=tx-pl.x,dz=tz-pl.z; pl.yaw=atan2f(-dx,-dz)*180.0f/M_PI; };
        auto findMob=[&](int want,float& mx,float& my,float& mz)->bool{ int n=engine_entitySnapshot();
            for(int i=0;i<n;i++){ float x,y,z,w,h,yw;int ty; if(engine_getEntity(i,&x,&y,&z,&w,&h,&ty,&yw)&&ty==want){mx=x;my=y;mz=z;return true;} } return false; };

        engine_setTime(6000); pl.fly=false; pl.x=sx+0.5f; pl.z=sz+0.5f; pl.y=spawnY+2;
        sim(0,0,0,false,1,40);                                            // settle on ground
        // 1) look around (4 compass directions)
        pl.pitch=-8; pl.yaw=0;   frame("look_N",true);
        pl.yaw=90;  frame("look_E",true); pl.yaw=180; frame("look_S",true); pl.yaw=270; frame("look_W",true);
        // 2) walk + jump over terrain, then sprint
        pl.yaw=20; sim(1,0,0,true,1.0f,70);  frame("walk",true);
        sim(1,0,0,true,1.4f,70);             frame("sprint",true);
        // 3) dig straight down 6 blocks (mining), see underground
        pl.fly=true; pl.pitch=-88;
        for(int d=0;d<6;d++){ g_look=raycast(pl); if(g_look.hit){ engine_setTile(g_look.hx,g_look.hy,g_look.hz,0); markDirty(g_look.hx,g_look.hz);} pl.y-=1.0f; }
        pl.pitch=-20; frame("mined_down",true);
        // 4) place a row of ALL 9 hotbar block types, then view it (verify each renders)
        pl.fly=true; pl.x=sx+0.5f; pl.z=sz+0.5f; pl.y=spawnY+3;
        { int by=(int)spawnY+3; for(int k=0;k<9;k++) engine_setTile(sx+3+k,by,sz-5,HOTBAR[k]);
          markDirty(sx+3,sz-5); markDirty(sx+11,sz-5);
          pl.x=sx+7.5f; pl.z=sz-1.0f; pl.y=by+1.0f; faceTo(sx+7.5f,sz-5.0f); pl.pitch=-6; frame("placed_blocks",false); }
        // 5) visit every mob species up close
        int mt[]={ET_PIG,ET_COW,ET_SHEEP,ET_CHICKEN,ET_WOLF,ET_VILLAGER,ET_ZOMBIE,ET_SKELETON,ET_CREEPER,ET_SPIDER};
        const char* mn[]={"pig","cow","sheep","chicken","wolf","villager","zombie","skeleton","creeper","spider"};
        for(int t=0;t<10;t++){ float mx,my,mz; if(findMob(mt[t],mx,my,mz)){
            pl.fly=true; pl.x=mx+2.2f; pl.z=mz+2.2f; pl.y=my+1.0f; faceTo(mx,mz); pl.pitch=-8;
            char lbl[32]; snprintf(lbl,sizeof lbl,"mob_%s",mn[t]); frame(lbl,false); }
            else printf("[soak] (no %s found)\n",mn[t]); }
        // 6) water — find a liquid near spawn, stand in it
        { int wx=0,wy=0,wz=0; bool fw=false;
          for(int r=1;r<48&&!fw;r++)for(int dx=-r;dx<=r&&!fw;dx++)for(int dz=-r;dz<=r&&!fw;dz++){
              int X=sx+dx,Z=sz+dz; engine_ensureChunk(X>>4,Z>>4);
              for(int Y=68;Y>54;--Y){ int t=engine_getTile(X,Y,Z); if(t==8||t==9){wx=X;wy=Y;wz=Z;fw=true;break;} } }
          if(fw){ pl.fly=false; pl.x=wx+0.5f; pl.z=wz+0.5f; pl.y=wy+1.0f; sim(0,0,0,false,1,15);
            pl.pitch=-10; faceTo((float)wx,(float)wz-3); frame("water",true); } else printf("[soak] (no water near spawn)\n"); }
        // 7) fly high — aerial survey
        pl.fly=true; pl.x=sx+0.5f; pl.z=sz+0.5f; pl.y=spawnY+28; pl.pitch=-45; pl.yaw=20; frame("aerial",false);
        // 8) full day/night cycle
        pl.y=spawnY+3; pl.pitch=-8; pl.yaw=30;
        int tms[]={0,3000,6000,9000,12000,15000,18000,21000};
        const char* tn[]={"t0_dawn","t3000","t6000_noon","t9000","t12000_dusk","t15000","t18000_midnight","t21000"};
        for(int t=0;t<8;t++){ engine_setTime(tms[t]); for(int k=0;k<3;k++) engine_tick(); frame(tn[t],true); }
        // 9) break/place verification (looking down)
        pl.fly=true; engine_setTime(6000); pl.pitch=-80; pl.x=sx+20.5f; pl.z=sz+0.5f; pl.y=spawnY+3;
        sim(0,0,0,false,1,1); g_look=raycast(pl);
        if(g_look.hit){ int oid=engine_getTile(g_look.hx,g_look.hy,g_look.hz);
            engine_setTile(g_look.hx,g_look.hy,g_look.hz,0); markDirty(g_look.hx,g_look.hz);
            printf("[soak] break: %d -> %d (ok=%d)\n",oid,engine_getTile(g_look.hx,g_look.hy,g_look.hz),engine_getTile(g_look.hx,g_look.hy,g_look.hz)==0);
            frame("after_break",false);
            engine_setTile(g_look.px,g_look.py,g_look.pz,57); markDirty(g_look.px,g_look.pz);  // place diamond block
            frame("after_place",false); }
        printf("[soak] DONE — %d frames captured\n",fc);
        XUngrabPointer(dpy,CurrentTime); return 0;
    }

    if(playtest){
        // Automated QA: drive the REAL stepPhysics/streaming/raycast through scenarios; assert invariants.
        FILE* rep=fopen("/home/hex/Minecraft/minecraft/playtest_report.txt","w");
        int issues=0; size_t maxChunks=0;
        auto chk=[&](bool ok,const char* msg){ if(!ok){ issues++; fprintf(rep,"  !! FAIL: %s\n",msg); printf("[playtest] FAIL: %s\n",msg);} };
        auto bad=[&](){ return std::isnan(pl.x)||std::isnan(pl.y)||std::isnan(pl.z)||std::isinf(pl.y); };
        // teleport to spawn and let gravity settle onto the surface (independent scenarios)
        auto reset=[&](){ pl.fly=false; pl.x=sx+0.5f; pl.z=sz+0.5f; pl.y=(float)(sy+3); pl.vx=pl.vy=pl.vz=0; pl.pitch=0;
            engine_ensureChunk(sx>>4,sz>>4); for(int i=0;i<40;i++) stepPhysics(pl,0,0,0,false,1,sy); };
        // run a scenario: optionally reset to spawn, face `yawDeg`, hold inputs for `ticks`, streaming each tick
        auto run=[&](const char* name,bool rst,float yawDeg,float fwd,float strafe,float up,bool jump,float sprint,bool fly,int ticks){
            if(rst) reset();
            pl.fly=fly; pl.yaw=yawDeg; pl.vx=pl.vy=pl.vz=0;
            float x0=pl.x,y0=pl.y,z0=pl.z; float minY=pl.y,maxY=pl.y;
            for(int i=0;i<ticks;i++){
                int ccx=((int)floorf(pl.x))>>4, ccz=((int)floorf(pl.z))>>4;
                engine_ensureChunk(ccx,ccz); streamChunks(ccx,ccz,8);
                stepPhysics(pl,fwd,strafe,up,jump,sprint,sy); engine_tick();
                if(bad()){ chk(false,(std::string(name)+": NaN/inf position").c_str()); break; }
                chk(pl.y>-40.0f,(std::string(name)+": fell into the void").c_str());
                if(pl.y<minY)minY=pl.y; if(pl.y>maxY)maxY=pl.y;
                if(g_chunks.size()>maxChunks)maxChunks=g_chunks.size();
            }
            float dh=hypotf(pl.x-x0,pl.z-z0);
            fprintf(rep,"[%-14s] moved h=%.1f  dy=%+.1f (min %.1f max %.1f)  end(%.1f,%.1f,%.1f) onGround=%d chunks=%zu\n",
                name,dh,pl.y-y0,minY,maxY,pl.x,pl.y,pl.z,(int)pl.onGround,g_chunks.size());
            printf("[playtest] %-14s h=%.1f dy=%+.1f end=(%.0f,%.0f,%.0f) chunks=%zu\n",name,dh,pl.y-y0,pl.x,pl.y,pl.z,g_chunks.size());
            return dh;
        };
        fprintf(rep,"=== PLAYTEST: native Linux Minecraft client ===\nspawn=(%d,%d,%d)\n\n",sx,sy,sz);
        // settle on ground first
        run("settle",true,0,0,0,0,false,1,false,40);
        chk(pl.onGround,"player never landed on ground after spawn");
        // walk the 4 compass directions (jump=true to climb terrain like a real player). Each from spawn.
        float wN=run("walk N",true,  0,1,0,0,true,1,false,90);
        float wE=run("walk E",true, 90,1,0,0,true,1,false,90);
        float wS=run("walk S",true,180,1,0,0,true,1,false,90);
        float wW=run("walk W",true,270,1,0,0,true,1,false,90);
        chk(wN>3&&wE>3&&wS>3&&wW>3,"player stuck in some direction (collision/step-up broken)");
        // sprint vs walk, both from spawn facing north, jumping over terrain
        float wk=run("walk ref",true,0,1,0,0,true,1.0f,false,90);
        float sp=run("sprint",  true,0,1,0,0,true,1.4f,false,90);
        chk(sp>wk,"sprint not faster than walk");
        // jump on the spot (continuous): must not launch/clip; stays in a sane height band, never voids
        { run("jump",true,0,0,0,0,true,1,false,60);
          chk(pl.y>(float)sy-3 && pl.y<(float)sy+7,"jump launched/dropped the player abnormally");
          chk(pl.y>-40,"fell through world while jumping"); }
        // fly up high, drop into survival, must land (not fall through the world)
        run("fly up",true,0,0,0,1,false,1,true,40);
        { run("fall",false,0,0,0,0,false,1,false,160); chk(pl.onGround,"did not land after falling from fly"); chk(pl.y>-40,"fell through world after fly-drop"); }
        // travel far by flying (terrain-independent): exercises streaming + eviction over distance
        float tf=run("travel far",true,0,1,0,0,false,1.2f,true,500);
        chk(tf>100,"fly-travel stalled (streaming/loop broken)");
        chk(maxChunks < (size_t)((2*(OPT.renderDist+5)+1)*(2*(OPT.renderDist+5)+1)),"chunk count unbounded (eviction failing)");
        // break & place via the DDA raycast
        pl.fly=true; pl.pitch=-80; g_look=raycast(pl);
        if(g_look.hit){ int t0=engine_getTile(g_look.hx,g_look.hy,g_look.hz);
            engine_setTile(g_look.hx,g_look.hy,g_look.hz,0); chk(engine_getTile(g_look.hx,g_look.hy,g_look.hz)==0,"break did not clear block");
            engine_setTile(g_look.hx,g_look.hy,g_look.hz,t0); }
        else fprintf(rep,"  (no block under crosshair to break-test)\n");
        fprintf(rep,"\n=== RESULT: %s  (%d issue%s, peak chunks=%zu) ===\n", issues?"FAIL":"PASS", issues, issues==1?"":"s", maxChunks);
        printf("\n[playtest] ===== %s : %d issue(s), peak chunks=%zu =====\n", issues?"FAIL":"PASS", issues, maxChunks);
        fclose(rep);
        return issues?2:0;
    }
    if(video){
        // Record an actual gameplay clip (walking the terrain, looking around, breaking/placing,
        // a full day->night cycle) and pipe raw frames straight into ffmpeg -> mp4 (+ gif).
        const int FPS=30; const int NF=FPS*15;            // ~15s
        glReadBuffer(GL_BACK);
        char cmd[640];
        snprintf(cmd,sizeof cmd,
            "ffmpeg -y -loglevel error -f rawvideo -pixel_format rgb24 -video_size %dx%d -framerate %d "
            "-i - -vf vflip -c:v libx264 -pix_fmt yuv420p -preset veryfast "
            "/home/hex/Minecraft/minecraft/gameplay.mp4", W,H,FPS);
        FILE* ff=popen(cmd,"w"); if(!ff){ printf("[video] ffmpeg pipe failed\n"); return 1; }
        std::vector<unsigned char> px((size_t)W*H*3);
        auto emit=[&](){ glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,px.data()); fwrite(px.data(),1,(size_t)W*H*3,ff); };
        layoutMenu();
        // --- title segment (~2s) with hover sweeping onto Play ---
        int titleF=FPS*2;
        for(int f=0;f<titleF;f++){ glViewport(0,0,W,H); begin2D(); drawDirtBG();
            drawLogo(true,f*0.15f);
            bool hov=(f>titleF*0.6f);
            drawButton(bPlay,hov);drawButton(bOpt,false);drawButton(bQuit,false);
            drawText("Minecraft Linux Edition",W/2-260,H-28,1.6f,0.85f,0.9f,1.0f);
            end2D(); emit(); glXSwapBuffers(dpy,win); }
        // --- gameplay segment: walk + look + interact + day/night ---
        pl.fly=false; float yaw0=pl.yaw=40;
        { // plant a small showcase garden in front of spawn so the clip opens on the non-cube blocks
            auto surf=[&](int X,int Z){ for(int Y=120;Y>2;--Y) if(engine_getTile(X,Y,Z)!=0) return Y+1; return (int)pl.y; };
            int deco[8]={37,38,31,6,50,44,78,18};   // flower,rose,grass,sapling,torch,slab,snow,leaves
            for(int r=0;r<2;r++)for(int i=0;i<8;i++){ int X=(int)pl.x-4+i, Z=(int)pl.z-3-r*2, Y=surf(X,Z);
                engine_setTile(X,Y,Z,deco[(i+r)%8]); markDirty(X,Z); }
        }
        int gf=NF-titleF;
        for(int f=0;f<gf;f++){ float tt=f/(float)FPS;
            engine_setTime(1500 + (long long)(f*(22000.0/gf)));      // morning -> night across the clip
            float fwd,strafe;
            if(f < FPS*3){                                           // intro: stand still, pan the garden
                float pr=f/(float)(FPS*3);
                pl.yaw=-35+pr*70.0f; pl.pitch=-16+sinf(pr*3.14159f)*6.0f; fwd=0; strafe=0;
            } else {
                pl.yaw  = yaw0 + sinf(tt*0.55f)*60.0f;               // slow look-around
                pl.pitch= -6 + sinf(tt*0.9f)*9.0f;
                fwd=(sinf(tt*0.45f)>-0.25f)?1.0f:0.0f; strafe=sinf(tt*0.7f)*0.35f;
            }
            float yr=pl.yaw*M_PI/180; float fxv=-sinf(yr),fzv=-cosf(yr),rxv=cosf(yr),rzv=-sinf(yr);
            float acc=0.09f;
            pl.vx+=(fxv*fwd+rxv*strafe)*acc; pl.vz+=(fzv*fwd+rzv*strafe)*acc;
            pl.vy-=0.08f; pl.vy*=0.98f; pl.onGround=false;
            moveAxis(pl,pl.vx,0,0); moveAxis(pl,0,0,pl.vz); moveAxis(pl,0,pl.vy,0);
            pl.vx*=0.60f; pl.vz*=0.60f;
            if(pl.onGround && fwd>0 && fabsf(pl.vx)+fabsf(pl.vz)<0.02f) pl.vy=0.42f;  // auto-jump if stuck
            engine_tick();
            int ccx=((int)floorf(pl.x))>>4, ccz=((int)floorf(pl.z))>>4; streamChunks(ccx,ccz,6);
            g_look=raycast(pl);
            if(f%80==40 && g_look.hit){ engine_setTile(g_look.hx,g_look.hy,g_look.hz,0); markDirty(g_look.hx,g_look.hz); }
            if(f%80==70 && g_look.hit && !placeWouldHitPlayer(pl,g_look.px,g_look.py,g_look.pz)){
                engine_setTile(g_look.px,g_look.py,g_look.pz,HOTBAR[g_sel]); markDirty(g_look.px,g_look.pz); }
            if(f%55==0) g_sel=(g_sel+1)%9;
            renderWorld(pl,engine_dayLight()); drawHUD(pl,FPS);
            emit(); glXSwapBuffers(dpy,win);
        }
        pclose(ff);
        printf("[video] wrote gameplay.mp4 — building gif...\n");
        system("ffmpeg -y -loglevel error -i /home/hex/Minecraft/minecraft/gameplay.mp4 "
               "-vf \"fps=15,scale=720:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse\" "
               "/home/hex/Minecraft/minecraft/gameplay.gif");
        printf("[video] wrote gameplay.mp4 + gameplay.gif\n");
        return 0;
    }
    if(demo){
        // capture title + an in-game HUD shot
        st=TITLE; layoutMenu();
        begin2D(); end2D();
        // title frame
        for(int f=0;f<2;f++){ glViewport(0,0,W,H); begin2D(); drawDirtBG();
            drawLogo(false,0);
            drawButton(bPlay,false);drawButton(bOpt,false);drawButton(bQuit,false);
            drawText("native Linux port - real ported engine",W/2-200,H-30,1.5f,0.8f,0.85f,0.9f);
            end2D(); glXSwapBuffers(dpy,win);}
        savePNG: ; {
            std::vector<unsigned char> px(W*H*3); glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,px.data());
            FILE* fp=fopen("frame_title.png","wb"); png_structp p=png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
            png_infop info=png_create_info_struct(p); png_init_io(p,fp);
            png_set_IHDR(p,info,W,H,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
            png_write_info(p,info); std::vector<png_bytep> r(H); for(int y=0;y<H;y++)r[y]=&px[(H-1-y)*W*3];
            png_write_image(p,r.data()); png_write_end(p,0); png_destroy_write_struct(&p,&info); fclose(fp); printf("[gl] wrote frame_title.png\n");
        }
        // gameplay frame with HUD
        engine_setTime(1000); for(int t=0;t<8;t++) engine_tick();   // morning: sun low + visible
        { // showcase row of non-cube blocks directly in front (yaw=0 faces -z)
            auto surf=[&](int X,int Z){ for(int Y=120;Y>2;--Y) if(engine_getTile(X,Y,Z)!=0) return Y+1; return (int)pl.y; };
            int row[7]={37,38,31,6,50,44,78};   // flower,rose,tall grass,sapling,torch,slab,snow
            int Z=(int)pl.z-4;
            for(int i=0;i<7;i++){ int X=(int)pl.x-3+i, Y=surf(X,Z); engine_setTile(X,Y,Z,row[i]); markDirty(X,Z); }
        }
        pl.pitch=-18; pl.yaw=0; g_look=raycast(pl);
        for(int f=0;f<3;f++){ renderWorld(pl,engine_dayLight()); drawHUD(pl,60); glXSwapBuffers(dpy,win);}
        { std::vector<unsigned char> px(W*H*3); glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,px.data());
          FILE* fp=fopen("frame_game.png","wb"); png_structp p=png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
          png_infop info=png_create_info_struct(p); png_init_io(p,fp);
          png_set_IHDR(p,info,W,H,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
          png_write_info(p,info); std::vector<png_bytep> r(H); for(int y=0;y<H;y++)r[y]=&px[(H-1-y)*W*3];
          png_write_image(p,r.data()); png_write_end(p,0); png_destroy_write_struct(&p,&info); fclose(fp); printf("[gl] wrote frame_game.png\n"); }
        // sky verification frame: noon sun
        engine_setTime(6000); for(int t=0;t<2;t++) engine_tick();
        pl.pitch=88; pl.yaw=0; g_look.hit=false;
        for(int f=0;f<3;f++){ renderWorld(pl,engine_dayLight()); glXSwapBuffers(dpy,win);}
        { std::vector<unsigned char> px(W*H*3); glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,px.data());
          FILE* fp=fopen("frame_sky.png","wb"); png_structp p=png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
          png_infop info=png_create_info_struct(p); png_init_io(p,fp);
          png_set_IHDR(p,info,W,H,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
          png_write_info(p,info); std::vector<png_bytep> r(H); for(int y=0;y<H;y++)r[y]=&px[(H-1-y)*W*3];
          png_write_image(p,r.data()); png_write_end(p,0); png_destroy_write_struct(&p,&info); fclose(fp); printf("[gl] wrote frame_sky.png\n"); }
        printf("[gl] demo done\n"); return 0;
    }

    bool running=true;
    while(running){
        layoutMenu();
        while(XPending(dpy)){
            XEvent e; XNextEvent(dpy,&e);
            // XInput2 raw relative motion (the FPS look source)
            if(e.type==GenericEvent && xiOK && e.xcookie.extension==xi_opcode){
                if(XGetEventData(dpy,&e.xcookie)){
                    if(e.xcookie.evtype==XI_RawMotion){
                        XIRawEvent* re=(XIRawEvent*)e.xcookie.data;
                        const double* v=re->raw_values; int idx=0; double dx=0,dy=0;
                        if(XIMaskIsSet(re->valuators.mask,0)) dx=v[idx++];
                        if(XIMaskIsSet(re->valuators.mask,1)) dy=v[idx++];
                        if(captured){ g_lookDX+=dx; g_lookDY+=dy; }
                    }
                    XFreeEventData(dpy,&e.xcookie);
                }
                continue;
            }
            if(e.type==ClientMessage && (Atom)e.xclient.data.l[0]==wmDelete){ setCapture(false); running=false; }
            else if(e.type==FocusOut){ memset(keys,0,sizeof keys); kShift=kCtrl=false;   // don't walk forever on alt-tab
                if(captured){ setCapture(false); st=TITLE; } }                            // and never trap the cursor
            else if(e.type==ConfigureNotify){ W=e.xconfigure.width; H=e.xconfigure.height; }
            else if(e.type==KeyPress||e.type==KeyRelease){
                KeySym ks=XLookupKeysym(&e.xkey,0); bool down=(e.type==KeyPress);
                if(ks<512) keys[ks]=down;
                if(ks==XK_Shift_L||ks==XK_Shift_R) kShift=down;
                if(ks==XK_Control_L||ks==XK_Control_R) kCtrl=down;
                if(down && ks==XK_Escape){
                    if(st==PLAY){ st=TITLE; setCapture(false); }
                    else if(st==OPTIONS){ st=TITLE; }
                    else running=false;
                }
                if(down && st==PLAY){
                    if(ks==XK_f){ pl.fly=!pl.fly; }
                    if(ks>=XK_1&&ks<=XK_9) g_sel=ks-XK_1;
                }
            }
            else if(e.type==ButtonPress){
                if(st==PLAY){
                    if(e.xbutton.button==4){ g_sel=(g_sel+8)%9; }        // scroll up
                    else if(e.xbutton.button==5){ g_sel=(g_sel+1)%9; }   // scroll down
                    else { RayHit h=raycast(pl);
                        if(h.hit){
                            if(e.xbutton.button==1){ engine_setTile(h.hx,h.hy,h.hz,0); markDirty(h.hx,h.hz); }
                            else if(e.xbutton.button==3 && !placeWouldHitPlayer(pl,h.px,h.py,h.pz)){
                                engine_setTile(h.px,h.py,h.pz,HOTBAR[g_sel]); markDirty(h.px,h.pz); }
                        }
                    }
                } else {
                    int cx=e.xbutton.x, cy=e.xbutton.y;
                    if(st==TITLE){
                        if(hit(bPlay,cx,cy)){ st=PLAY; setCapture(true); }
                        else if(hit(bOpt,cx,cy)) st=OPTIONS;
                        else if(hit(bQuit,cx,cy)) running=false;
                    } else if(st==OPTIONS){
                        bool rc=(e.xbutton.button==3);
                        if(hit(oRD,cx,cy)) OPT.renderDist += rc?-1:1, OPT.renderDist=OPT.renderDist<3?3:(OPT.renderDist>14?14:OPT.renderDist);
                        else if(hit(oSe,cx,cy)) OPT.sens += rc?-0.02f:0.02f, OPT.sens=OPT.sens<0.02f?0.02f:(OPT.sens>0.4f?0.4f:OPT.sens);
                        else if(hit(oFov,cx,cy)) OPT.fov += rc?-5:5, OPT.fov=OPT.fov<50?50:(OPT.fov>110?110:OPT.fov);
                        else if(hit(oFly,cx,cy)) OPT.fly=!OPT.fly, pl.fly=OPT.fly;
                        else if(hit(bDone,cx,cy)) st=TITLE;
                    }
                }
            }
        }

        if(st==PLAY){
            // mouse look — poll absolute pointer, take delta from window centre, warp back.
            // (warp works under XWayland where XI2 raw relative motion does NOT — that was the dead-mouse bug.)
            if(captured){
                Window rr,cr; int rx,ry,wx,wy; unsigned mb; int cx=W/2,cy=H/2;
                if(XQueryPointer(dpy,win,&rr,&cr,&rx,&ry,&wx,&wy,&mb)){
                    float ddx=(float)(wx-cx), ddy=(float)(wy-cy);
                    if(fabsf(ddx)<400.0f && fabsf(ddy)<400.0f){           // reject the big jump on (re)capture
                        pl.yaw  -= ddx*OPT.sens;
                        pl.pitch-= ddy*OPT.sens;
                        if(pl.pitch>89)pl.pitch=89; if(pl.pitch<-89)pl.pitch=-89;
                        if(pl.yaw>360)pl.yaw-=360; if(pl.yaw<0)pl.yaw+=360;
                    }
                    if(wx!=cx||wy!=cy){ XWarpPointer(dpy,None,win,0,0,0,0,cx,cy); XSync(dpy,False); }
                }
            }
            g_lookDX=g_lookDY=0;
            // movement input -> world space
            float fwd=0,strafe=0,up=0;
            if(keys[XK_w])fwd+=1; if(keys[XK_s])fwd-=1; if(keys[XK_a])strafe-=1; if(keys[XK_d])strafe+=1;
            if(keys[XK_space])up+=1; if(kShift)up-=1;
            float sprint = (kCtrl && fwd>0)?1.4f:1.0f;          // Ctrl = sprint
            float yaw=pl.yaw*M_PI/180; float fxv=-sinf(yaw),fzv=-cosf(yaw),rxv=cosf(yaw),rzv=-sinf(yaw);
            // ensure the player's own chunk exists before physics (never fall through ungenerated space)
            { int pcx=((int)floorf(pl.x))>>4, pcz=((int)floorf(pl.z))>>4; engine_ensureChunk(pcx,pcz); }
            (void)fxv;(void)fzv;(void)rxv;(void)rzv;   // movement math now lives in stepPhysics
            // fixed-step physics (single source of truth)
            long long t=nowms(); int steps=0;
            while(t-lastTick>=50 && steps<5){ lastTick+=50; steps++;
                stepPhysics(pl, fwd,strafe,up, keys[XK_space], sprint, sy);
                engine_tick();
            }
            int ccx=((int)floorf(pl.x))>>4, ccz=((int)floorf(pl.z))>>4;
            streamChunks(ccx,ccz,6);
            g_look=raycast(pl);
            renderWorld(pl,engine_dayLight()); drawHUD(pl,fps);
        } else {
            glViewport(0,0,W,H);
            // background
            if(st==TITLE||st==OPTIONS){ begin2D(); drawDirtBG();
                int pmx,pmy; { Window rr,cr;int rx,ry;unsigned m; XQueryPointer(dpy,win,&rr,&cr,&rx,&ry,&pmx,&pmy,&m); }
                if(st==TITLE){
                    drawLogo(true,frame*0.15f);
                    drawButton(bPlay,hit(bPlay,pmx,pmy)); drawButton(bOpt,hit(bOpt,pmx,pmy)); drawButton(bQuit,hit(bQuit,pmx,pmy));
                    drawText("Minecraft Linux Edition",W/2-260,H-28,1.6f,0.85f,0.9f,1.0f);
                } else {
                    drawText("Options",W/2-60,40,3.0f,1,1,1);
                    char b1[64],b2[64],b3[64],b4[64];
                    snprintf(b1,64,"Render Distance: %d",OPT.renderDist);
                    snprintf(b2,64,"Mouse Sensitivity: %d%%",(int)((OPT.sens-0.02f)/(0.40f-0.02f)*100));
                    snprintf(b3,64,"FOV: %d",OPT.fov);
                    snprintf(b4,64,"Movement: %s",OPT.fly?"Fly":"Walk");
                    oRD.label=b1;oSe.label=b2;oFov.label=b3;oFly.label=b4;
                    drawButton(oRD,hit(oRD,pmx,pmy));drawButton(oSe,hit(oSe,pmx,pmy));
                    drawButton(oFov,hit(oFov,pmx,pmy));drawButton(oFly,hit(oFly,pmx,pmy));
                    drawButton(bDone,hit(bDone,pmx,pmy));
                    drawText("left-click +   right-click -",W/2-140,H/2+170,1.4f,0.8f,0.8f,0.8f);
                }
                end2D();
            }
        }
        glXSwapBuffers(dpy,win);
        fpsCount++; long long tn=nowms(); if(tn-fpsT>=1000){ fps=fpsCount; fpsCount=0; fpsT=tn; }
        usleep(8000); frame++;
    }
    // always release the pointer + clean up so we never leave the desktop's mouse trapped
    XUngrabPointer(dpy,CurrentTime); XUndefineCursor(dpy,win); XFlush(dpy);
    glXMakeCurrent(dpy,0,0); glXDestroyContext(dpy,ctx); XDestroyWindow(dpy,win); XCloseDisplay(dpy);
    printf("[gl] bye.\n"); return 0;
}
