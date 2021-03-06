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
#include "str.hpp"	// mystrcasecmp
#include "netobj.hpp"	// Noid
#include "initobj.hpp"	// OBJECTSNUMBER
#include "oclass.hpp"

using namespace std;

#define STL 0

#if STL
vector< OClass* > OClass::otable(OBJECTSNUMBER, NULL);
//vector< OClass* > OClass::otable(1, NULL);
#else
OClass** OClass::otable = NULL;
uint16_t OClass::otable_size = 0;
#endif

// local
static const OClass end(0, "End", NULL);


/** OClass constructor */
OClass::OClass(uint8_t _type_id, const char* _type_name,
	       WCreator _creator, WReplicator _replicator, WBuiltin _builtin) :
  type_id(_type_id), type_name(_type_name), creator(_creator), replicator(_replicator), builtin(_builtin)
{
#if STL
  if (type_id < otable.size()) otable.at(type_id) = this;
  else {
    for (uint32_t i=1; i < otable.size(); i++) otable.push_back((OClass *) NULL);
    //for (uint32_t i=1; i < otable.size(); i++) otable.at(i) = (OClass *) NULL;
    otable.push_back(this);
  }
  //error("otable[%d] %s size=%d", type_id, type_name, otable.size());
#else
  if (type_id < otable_size) otable[type_id] = this;
  else {
    if (! (otable = (OClass**) realloc(otable, sizeof(OClass *) * (type_id+1))))
      fatal("can't realloc otable");
    for (int i=otable_size; i < type_id + 1; i++) otable[i] = (OClass *) NULL;
    otable_size = type_id + 1;
    otable[type_id] = this;
  }
  //error("otable[%d] %s size=%d", type_id, type_name, otable_size);
#endif
}

const OClass * OClass::getOClass(const char *type_name)
{
#if STL
  for (uint32_t i=1; i < otable.size(); i++) {
    if (otable.at(i)) {
      if (otable.at(i)->type_name) {
        if (type_name)
          if (! mystrcasecmp(type_name, otable.at(i)->type_name)) return otable.at(i);
      }
      else
        error("otable[%d]->type_name=%s", i, otable.at(i)->type_name);
    }
#else
  for (int i=1; i < otable_size; i++) {
    if (otable[i]) {
      if (otable[i]->type_name) {
        if (type_name)
          if (! mystrcasecmp(type_name, otable[i]->type_name)) return otable[i];
      }
      else
        error("otable[%d]->type_name=%s", i, otable[i]->type_name);
    }
#endif
  }
  error("type_name=%s not found, please upgrade VREng!", type_name);
  return NULL;
}

const OClass * OClass::getOClass(uint8_t type_id)
{
#if STL
  if (isValidType(type_id)) return otable[type_id];
#else
  if (type_id < otable_size) return otable[type_id];
#endif
  error("getOClass: type_id=%d out of bounds", type_id); dumpTable();
  return NULL;
}

WObject * OClass::creatorInstance(uint8_t type_id, char *l)
{
  if (isValidType(type_id)) return otable[type_id]->creator(l);
  error("creatorInstance: type_id=%d out of bounds", type_id); dumpTable();
  return NULL;
}

void OClass::builtinInstance(uint8_t type_id)
{
  if (isValidType(type_id)) otable[type_id]->builtin();
  else error("builtinInstance: type_id=%d out of bounds", type_id);
}

WObject * OClass::replicatorInstance(uint8_t type_id, Noid noid, Payload *pp)
{
  if (isValidType(type_id)) return otable[type_id]->replicator(type_id, noid, pp);
  error("replicatorInstance: type_id=%d out of bounds", type_id); dumpTable();
  return NULL;
}

void OClass::dumpTable()
{
#if STL
  for (uint32_t i=1; i < otable.size(); i++)
#else
  for (int i=1; i < otable_size; i++)
#endif
    printf("%02d: %p %02d %s\n", i, otable[i], otable[i]->type_id, otable[i]->type_name);
}

bool OClass::isValidType(int type_id)
{
  return (type_id > 0 && type_id <= OBJECTSNUMBER);
}
