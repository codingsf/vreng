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
#ifndef INITOBJ_HPP
#define INITOBJ_HPP

#include "objects.hpp"	// OBJECTSNUMBER
#include "wobject.hpp"	// WObject

#define UNKNOWN_TYPE	-1


/**
 * GeneralInitList struct
 */
struct GeneralInitList {
  void (*initfunc) (void);	///< init function
};

/**
 * PropertyFuncList struct
 */
struct PropertyFuncList {
  void (*method) (WObject *po, class Payload *pp);
};
#define WO_PAYLOAD  (void (*)(WObject *po, class Payload *pp))

/**
 * GeneralActionList struct
 */
struct GeneralActionList {
  char name[ACTIONNAME_LEN];
  void (*method) (WObject *po, void *d, time_t s, time_t u);
};
#define WO_ACTION (void (*)(WObject *po, void *d, time_t s, time_t u))


/* external functions */

void setActionFunc(int type, uint8_t action, void (*method)(WObject *o, void *d, time_t s, time_t u), const char *action_name);
bool isAction(int type, uint8_t action);
void doAction(int type, uint8_t action, WObject *o, void *d, time_t s, time_t u);
bool isActionName(int type, uint8_t action);
int indexAction(int type, const char *name);
void copyActionName(int type, uint8_t action, char *dest);
char * getActionName(int type, uint8_t action);
void * getActionMethod(int type, uint8_t action);

void setMaxLastings(int type, float maxlast);
float getMaxLastings(int type);

void setPropertiesnumber(int type, uint8_t nbprop);
uint8_t getPropertiesnumber(int type);

void getPropertyFunc(int type, uint8_t prop, void (*getprop_method)(WObject *o, Payload *p));
void putPropertyFunc(int type, uint8_t prop, void (*putprop_method)(WObject *o, Payload *p));
bool isGetPropertyFunc(int type, uint8_t prop);
bool isPutPropertyFunc(int type, uint8_t prop);
void runGetPropertyFunc(int type, uint8_t prop, WObject *o, Payload *pp);
void runPutPropertyFunc(int type, uint8_t prop, WObject *o, Payload *pp);

#endif
