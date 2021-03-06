//---------------------------------------------------------------------------
// VREng (Virtual Reality Engine)	http://vreng.enst.fr/
//
// Copyright (C) 1997-2012 Philippe Dax
// Telecom-ParisTech (Ecole Nationale Superieure des Telecommunications)
//
// VREng is a free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public Licence as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// VREng is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//---------------------------------------------------------------------------
#include "vreng.hpp"
#include "smoke.hpp"
#include "timer.hpp"	// delta


const OClass Smoke::oclass(SMOKE_TYPE, "Smoke", Smoke::creator);

// local
static float norms[3][3][3] = {
{ {TILE*D0,TILE*D1,TILE*D2}    ,{TILE*D1,TILE*D1_1,TILE*D2_1}      ,{TILE*D2,TILE*D2_1,TILE*D2_2} },
{ {TILE*D1,TILE*D1_1,TILE*D2_1},{TILE*D1_1,TILE*D1_1_1,TILE*D2_1_1},{TILE*D2_1,TILE*D2_1_1,TILE*D2_2_1} },
{ {TILE*D2,TILE*D2_1,TILE*D2_2},{TILE*D2_1,TILE*D2_1_1,TILE*D2_2_1},{TILE*D2_2,TILE*D2_2_1,TILE*D2_2_2} }
};
static float distvar[3] = {-TILE,0,TILE};

struct SMOKE_PARTICLE {
  Vector3 p, v;
} offset[NTILEZ][NTILEY][NTILEZ];

static struct sParticle particles[PARTMAX];
static float deplfact[1<<8];
static Vector3 Src(0,5,-1.5);
static Vector3 FPartA(0.,0.,4.);
static Matrix3 M;
static Vector3 O;


/* creation from a file */
WObject * Smoke::creator(char *l)
{
  return new Smoke(l);
}

void Smoke::defaults()
{
  np = NBPARTS;
  speed = 0.5;
}

void Smoke::parser(char *l)
{
  defaults();
  l = tokenize(l);
  begin_while_parse(l) {
    l = parse()->parseAttributes(l, this);
    if (!l) break;
    if      (!stringcmp(l, "number")) l = parse()->parseUInt16(l, &np, "number");
    else if (!stringcmp(l, "speed"))  l = parse()->parseFloat(l, &speed, "speed");
  }
  end_while_parse(l);
}

void Smoke::behavior()
{
  enableBehavior(NO_ELEMENTARY_MOVE);
  enableBehavior(COLLIDE_NEVER);
  enableBehavior(SPECIFIC_RENDER);
  setRenderPrior(RENDER_HIGH);
}

Smoke::Smoke(char *l)
{
  parser(l);
  behavior();

  initializeMobileObject(0);
  enablePermanentMovement();

  makeSolid();
  inits();
}

void Smoke::inits()
{
  lasttime = 0;
  np = MIN(np, PARTMAX);
  anim = true;

  for (int i=0 ; i < (1<<8) ; i++)
    deplfact[i] = i/((float)(1<<8));

  //initialise gelly
  float t0, t1, t2, t3, tt0, tt1, tt2, tt3, ttt0, ttt1, ttt2, ttt3, xf, yf;
  float f0, f1, f2, f3, ff0, ff1, ff2, ff3, fff0, fff1, fff2, fff3;
  ttt0 = rnd2(M_2PI); f0=rnd2(ANGRND)+ANG0; ff0=rnd2(ANGRND)+ANG0; fff0=rnd2(ANGRND)+ANG0;
  ttt1 = rnd2(M_2PI); f1=rnd2(ANGRND)+ANG0; ff1=rnd2(ANGRND)+ANG0; fff1=rnd2(ANGRND)+ANG0;
  ttt2 = rnd2(M_2PI); f2=rnd2(ANGRND)+ANG0; ff2=rnd2(ANGRND)+ANG0; fff2=rnd2(ANGRND)+ANG0;
  ttt3 = rnd2(M_2PI); f3=rnd2(ANGRND)+ANG0; ff3=rnd2(ANGRND)+ANG0; fff3=rnd2(ANGRND)+ANG0;
  //error("ttt0=%.2f f0=%.2f ff0=%.2f fff0=%.2f",ttt0,f0,ff0,fff0);
  //error("ttt1=%.2f f1=%.2f ff1=%.2f fff1=%.2f",ttt1,f1,ff1,fff1);
  //error("ttt2=%.2f f2=%.2f ff2=%.2f fff2=%.2f",ttt2,f2,ff2,fff2);
  //error("ttt3=%.2f f3=%.2f ff3=%.2f fff3=%.2f",ttt3,f3,ff3,fff3);

  SMOKE_PARTICLE *p = &offset[0][0][0];
  for (int z=0; z<NTILEZ; z++) {
    //notused zf = float(z);
    tt0 = ttt0; tt1 = ttt1; tt2 = ttt3; tt3 = ttt3;
    ttt0 += fff0; ttt1 += fff1; ttt2 += fff2; ttt3 += fff3;
    for (int y=0; y<NTILEY; y++) {
      yf = float(y);
      t0 = tt0; t1 = tt1; t2 = tt3; t3 = tt3;
      tt0 += ff0; tt1 += ff1; tt2 += ff2; tt3 += ff3;
      for (int x=0; x<NTILEX; x++, p++) {
        xf = float(x);
        t0 += f0; t1 += f1; t2 += f2; t3 += f3;
        p->p.x = WARPF*(sin(ANGF*(t2-xf))+cos(ANGF*(t2+yf))+sin(ANGF*(t0-yf))-cos(ANGF*(t0+xf)));
        p->p.y = WARPF*(sin(ANGF*(t1-xf))+cos(ANGF*(t3+yf))+sin(ANGF*(t3-yf))-cos(ANGF*(t1+xf)));
        p->p.z = WARPF*(sin(ANGF*(t0-xf))+cos(ANGF*(t2+yf))+sin(ANGF*(t0-yf))-cos(ANGF*(t2+xf)));
        p->v.reset();
      }
    }
  }
}

void Smoke::draw(float ex, float ey, float dx, float dy, float a)
{
  float white[] = {.7, .7, .7, .7};
  float grey[] = {.1, .1, .1, .1};
  white[3] = a;
  grey[3] = a;
  //error("draw %.2f %.2f %.2f %.2f %.2f", ex,ey,dx,dy,a);

  glBegin(GL_TRIANGLE_FAN);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
    glVertex3f(ex, ex, ey);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, grey);
    glVertex3f(ex-dx, ex-dx, ey);
    glVertex3f(ex-dx*.5, ex-dx*.5, ey + M_SQRT3_2*dy);
    glVertex3f(ex+dx*.5, ex+dx*.5, ey + M_SQRT3_2*dy);
    glVertex3f(ex+dx, ex+dx, ey);
    glVertex3f(ex+dx*.5, ex+dx*.5, ey - M_SQRT3_2*dy);
    glVertex3f(ex-dx*.5, ex-dx*.5, ey - M_SQRT3_2*dy);
    glVertex3f(ex-dx, ex-dx, ey);
  glEnd();
}

void Smoke::changePermanent(float dt)
{
  struct sParticle *p = particles;
  float da = pow(0.2, dt);

  time += dt;
  //error("np=%d", np);
  motionAnimate(dt);
  for (int n=0; n < np; n++, p++) {
    //error("t=%.2F %.2f", p->t, dt);
    if ((p->t -= dt) <= 0)
      *p = particles[--np]; // kill it
    else { // animate it
      p->v += FPartA * dt;
      p->p += p->v * dt;
      motionWarp(p->p, dt);
      p->a *= da;
    }
  }
  lasttime += dt;
#if 1 //dax
  while (np < PARTMAX && (time - lasttime >= SMOKEDELTA)) { 
    Vector3 Dir(0,0,.25);
    lasttime += SMOKEDELTA;
    p = particles + (np++);
    p->p = Src; // + DS1*rnd2(0.25) + DS2*rnd2(0.25);
    p->v = Dir;
    motionWarp(p->p, (time - lasttime));
    p->t = SMOKELIFE + lasttime - time;
    float size = SMOKESIZE * (0.5+rnd2(0.5));
    float alpha = SMOKEALPHA * (float)pow(size/SMOKESIZE, 0.5);
    p->s.x = size;
    p->s.y = size;
    p->s.z = size;
    p->a = alpha*pow(da, (time - lasttime));
    p->s *= 0.5 + rnd2(0.5);
  }
#endif

  O.reset();
  M.apply(Src, &O);
  O = Src - O;
  p = particles;
  for (int n=0; n < np; n++, p++) {
    float Foc = 2.;
    Vector3 v;
    M.apply(p->p, &v);
    v += O;
    if (v.y > 1) {
      p->ex = ProjEX(v);
      p->ey = ProjEY(v);
      ProjEZ(p->ez, v);
      p->dx = Foc*p->s.x/v.y;
      p->dy = Foc*p->s.z/v.y;
    }
    else p->dx = 0;
  }
}

// Handles the wobbling gel, which is used to give the WarpMap it's shape
void Smoke::motionAnimate(float dt)
{
#if 1 //dax
  SMOKE_PARTICLE *p = &offset[0][0][0];
  for (int z=0; z<NTILEZ; z++) {
    int zh = z+1; int zl = z-1;
    for (int y=0; y<NTILEY; y++) {
      int yh = y+1; int yl = y-1;
      for (int x=0; x<NTILEX; x++,p++) {
        int xh = x+1; int xl = x-1;
        for (int zi=zl; zi<=zh; zi++) {
          for (int yi=yl; yi<=yh; yi++) {
            for (int xi=xl; xi<=xh; xi++) {
              if ((xi != x) || (yi != y) || (zi!=z)) {
                float norm = norms[abs(zi-z)][abs(yi-y)][abs(xi-x)];
                SMOKE_PARTICLE *p2 = &offset[zi&(NTILEZ-1)][yi&(NTILEY-1)][xi&(NTILEX-1)];
                Vector3 spring = p2->p;
                spring -= p->p;
                spring.x += distvar[xi-x+1];
                spring.y += distvar[yi-y+1];
                spring.z += distvar[zi-z+1];
                spring *= (norm-spring.length()) * (float)SPRINGF * dt;
                p2->v += spring;
              }
            }
          }
        }
      }
    }
  }
#endif
#if 1 //dax
  p = &offset[0][0][0];
  float ft = (float) pow(DYNF, dt);
  for (int z=0; z<NTILEZ; z++) {
    for (int y=0; y<NTILEY; y++) {
      for (int x=0; x<NTILEX; x++, p++) {
        p->p += p->v * dt;
        if (p->p.length2() > (MAXLEN*MAXLEN))
          p->v.reset();
        else
          p->v *= ft;
      }
    }
  }
#endif
}

void Smoke::motionWarp(Vector3 &p, float dt)
{
  static Vector3 v0,v1,v2,v3,v4,v5,v;

  int sx = (f2int((p.x)*(TILE*(1<<8)))+((NTILEX/2)<<8)) & ((NTILEX<<8)-1);
  int sy = (f2int((p.y)*(TILE*(1<<8)))+((NTILEY/2)<<8)) & ((NTILEY<<8)-1);
  int sz = (f2int((p.z)*(TILE*(1<<8)))) & ((NTILEZ<<8)-1);
  float fx = deplfact[sx&((1<<8)-1)];
  float fy = deplfact[sy&((1<<8)-1)];
  float fz = deplfact[sz&((1<<8)-1)];
  sx >>= 8; //notused int x2 = (sx+1)&(NTILEX-1);
  sy >>= 8; //notused int y2 = (sy+1)&(NTILEY-1);
  sz >>= 8; int z2 = (sz+1)&(NTILEZ-1);

  v0.lerp(offset[sz][sy][sx].p, offset[sz][sy][sx+1].p, fx);
  v1.lerp(offset[z2][sy][sx].p, offset[z2][sy][sx+1].p, fy);
  v.lerp(v0, v1, fz);
  v *= dt;
  p += v;
}

void Smoke::funcs() {}
