//---------------------------------------------------------------------------
// VREng (Virtual Reality Engine)       http://vreng.enst.fr/
//
// Copyright (C) 1997-2009 Philippe Dax
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
#include "river.hpp"


const OClass River::oclass(RIVER_TYPE, "River", River::creator);

const uint8_t River::DEF_WAVES = 10;
const float River::DEF_WIDTH = 20;
const float River::DEF_DEPTH = 5;
const GLfloat River::DEF_SCALE = 1;
const GLfloat River::DEF_COLOR[4] = {0, 0.1, 0.2, 1};


WObject * River::creator(char *l)
{
  return new River(l);
}

void River::defaults()
{
  waves = DEF_WAVES;
  width = DEF_WIDTH;
  depth = DEF_DEPTH;
  scale = DEF_SCALE;
  for (int i=0; i<4; i++) color[i] = DEF_COLOR[i];
}

void River::parser(char *l)
{
  defaults();
  l = tokenize(l);
  begin_while_parse(l) {
    l = parse()->parseAttributes(l, this);
    if (!l) break;
    if      (!stringcmp(l, "waves")) l = parse()->parseUInt8(l, &waves, "waves");
    else if (!stringcmp(l, "width")) l = parse()->parseFloat(l, &width, "width");
    else if (!stringcmp(l, "depth")) l = parse()->parseFloat(l, &depth, "depth");
    else if (!stringcmp(l, "scale")) l = parse()->parseFloat(l, &scale, "scale");
    else if (!stringcmp(l, "color")) l = parse()->parseVector3f(l, color, "color");
  }
  end_while_parse(l);
}

void River::behavior()
{
  enableBehavior(NO_BBABLE);
  enableBehavior(SPECIFIC_RENDER);
  if (width * depth > 10)
    setRenderPrior(RENDER_LOW);
  else
    setRenderPrior(RENDER_HIGH);

  initializeStillObject();
}

void River::inits()
{
  if (waves > DEF_WAVES) waves = DEF_WAVES;
  mesh = new float[2*waves*sizeof(float)+1];
  phase = new float[waves*sizeof(float)];
  speed = new float[waves*sizeof(float)];
  ampl = new float[waves*sizeof(float)];
  srand(time(0));

  for (int i=0; i<waves ; i++) {
    mesh[2*i] = (float) (rand()%(int)ceil(width));
    mesh[2*i+1] = (float) (rand()%(int)ceil(depth));
    phase[i] = (float) (rand()%10);
    speed[i] = (float) (rand()%100)/300;
    ampl[i] = 0.005* (float) (rand()%100)/100;
  }
}

River::River(char *l)
{
  parser(l);
  behavior();
  inits();
}

void River::wave(float a, float b)
{
  float x=0, y=0, z=0;
  float vx,vy;

  for (int i=0; i<waves ; i++) {
    vx = mesh[2*i];
    vy = mesh[2*i+1];
    x += vx * width * sin(a*vx+b*vy + phase[i]) * ampl[i];
    y += vy * depth * sin(a*vx+b*vy + phase[i]) * ampl[i];
  }
  glNormal3f(x, y, 1);

  for (int i=0; i<waves ; i++) {
    vx = mesh[2*i];
    vy = mesh[2*i+1];
    z += cos(a*vx+b*vy + phase[i])*ampl[i];
  }
  glVertex3d(a, b, z);
}

void River::render()
{
  //DAX glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  glEnable(GL_COLOR_MATERIAL);
  glMateriali(GL_FRONT_AND_BACK,GL_SHININESS, 30);
  glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION, color);
  glEnable(GL_LIGHTING);
  glColor4f(0.2, 0.4, 0.7, 0.9);

  glTranslatef(pos.x, pos.y, pos.z);
  glRotatef(RAD2DEG(pos.ax), 1, 0, 0);
  glRotatef(RAD2DEG(pos.ay), 0, 1, 0);
  glRotatef(RAD2DEG(pos.az), 0, 0, 1);
  glScalef(scale, scale, scale);

  glBegin(GL_QUADS);
  for (int a=0; a < 2*ceil(width) ; a++) {
    for (int b=0; b < 2*ceil(depth) ; b++) {
      wave((float) a, (float) b);
      wave((float) a+1, (float) b);
      wave((float) a+1, (float) b+1);
      wave((float) a, (float) b+1);
    }
  }
  glEnd();
  for (int i=0; i<waves ; i++)
    phase[i] += speed[i];

  glDisable(GL_LIGHTING);
  glPopMatrix();
  //DAX glPopAttrib();
}

void River::quit()
{
  if (mesh) delete[] mesh;
  if (phase) delete[] phase;
  if (speed) delete[] speed;
  if (ampl) delete[] ampl;
} 

void River::funcs()
{
} 
