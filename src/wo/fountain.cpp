//---------------------------------------------------------------------------
// VREng (Virtual Reality Engine)	http://vreng.enst.fr/
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
#include "fountain.hpp"
#include "world.hpp"	// getGround


const OClass Fountain::oclass(FOUNTAIN_TYPE, "Fountain", Fountain::creator);


/* creation from a file */
WObject * Fountain::creator(char *l)
{
  return new Fountain(l);
}

void Fountain::defaults()
{
  system = FOUNTAIN;
  number = DEF_NUM;
  flow = DEF_FLOW;
  speed = DEF_SPEED;
  points = true;
  pt_size = DEF_PTSIZE;
  ground = World::current()->getGround();
  for (int i=0; i<3; i++) color[i] = 1;	// white
}

void Fountain::parser(char *l)
{
  defaults();
  l = tokenize(l);
  begin_while_parse(l) {
    l = parse()->parseAttributes(l, this);
    if (!l) break;
    else if (! stringcmp(l, "number")) l = parse()->parseUInt16(l, &number, "number");
    else if (! stringcmp(l, "flow"))   l = parse()->parseFloat(l, &flow, "flow");
    else if (! stringcmp(l, "speed"))  l = parse()->parseFloat(l, &speed, "speed");
    else if (! stringcmp(l, "ground")) l = parse()->parseFloat(l, &ground, "ground");
    else if (! stringcmp(l, "color"))  l = parse()->parseVector3f(l, color, "color");
    else if (! stringcmp(l, "size")) {
      l = parse()->parseUInt8(l, &pt_size, "size");
      if (pt_size <= 0) pt_size = 1;
    }
  }
  end_while_parse(l);
}

Fountain::Fountain(char *l)
{
  parser(l);
  behavior();
  inits();
}

void Fountain::pause(Fountain *fountain, void *d, time_t s, time_t u)
{
  if (fountain->state == ACTIVE) fountain->state = INACTIVE;
  else fountain->state = ACTIVE;
}

void Fountain::funcs()
{
  setActionFunc(FOUNTAIN_TYPE, 0, WO_ACTION pause, "Switch");
}
