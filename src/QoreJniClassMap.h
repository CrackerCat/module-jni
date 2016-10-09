/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  QoreJniClassMap.h

  Qore Programming Language JNI Module

  Copyright (C) 2016 Qore Technologies, s.r.o.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _QORE_JNI_QOREJAVACLASSMAP_H

#define _QORE_JNI_QOREJAVACLASSMAP_H

DLLLOCAL void init_jni_functions(QoreNamespace& ns);
DLLLOCAL QoreClass* initObjectClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initArrayClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initThrowableClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initClassClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initFieldClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initStaticFieldClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initMethodClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initStaticMethodClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initConstructorClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initInvocationHandlerClass(QoreNamespace& ns);

DLLLOCAL QoreClass* jni_class_handler(QoreNamespace* ns, const char* cname);

class QoreJniClassMap {
protected:
   // map of java class names to QoreClass objects
   typedef std::map<std::string, QoreClass*> jcmap_t;
   // map of java class objects to QoreClass objects
   //typedef std::map<jclass, QoreClass*> jcpmap_t;

   // parent namespace for jni module functionality
   QoreNamespace default_jns;
   jcmap_t jcmap;
   //jcpmap_t jcpmap;
   bool init_done;

   /*
   DLLLOCAL void add_intern(const char* name, java::lang::Class* jc, QoreClass* qc) {
      //printd(0, "QoreJniClassMap::add_intern() name=%s jc=%p qc=%p (%s)\n", name, jc, qc, qc->getName());

      assert(jcmap.find(name) == jcmap.end());
      jcmap[name] = qc;

      //assert(jcpmap.find(jc) == jcpmap.end());
      //jcpmap[jc] = qc;
   }

   DLLLOCAL int getArgTypes(type_vec_t& argTypeInfo, JArray<jclass> *params);

   DLLLOCAL void doConstructors(QoreClass& qc, java::lang::Class *jc, ExceptionSink *xsink = 0);
   DLLLOCAL void doMethods(QoreClass& qc, java::lang::Class *jc, ExceptionSink *xsink = 0);
   DLLLOCAL void doFields(QoreClass& qc, java::lang::Class *jc, ExceptionSink *xsink = 0);

   DLLLOCAL void populateQoreClass(QoreClass& qc, java::lang::Class *jc, ExceptionSink *xsink = 0);
   DLLLOCAL void addQoreClass();

   DLLLOCAL void addSuperClass(QoreClass& qc, java::lang::Class *jsc);
   */

public:
   mutable QoreThreadLock m;

   DLLLOCAL QoreJniClassMap() : default_jns("Jni"), init_done(false) {
   }

   DLLLOCAL void init();

   /*
   DLLLOCAL QoreClass* createQoreClass(QoreNamespace &jns, const char *name, java::lang::Class *jc, ExceptionSink *xsink = 0);

   DLLLOCAL QoreClass* find(java::lang::Class *jc) const {
      jcpmap_t::const_iterator i = jcpmap.find(jc);
      return i == jcpmap.end() ? 0 : i->second;
   }

   DLLLOCAL QoreClass* findCreate(java::lang::Class *jc) {
      QoreClass *qc = find(jc);
      if (qc)
         return qc;
      QoreString cname;
      getQoreString(jc->getName(), cname);
      return createQoreClass(default_jns, cname.getBuffer(), jc);
   }

   DLLLOCAL const QoreTypeInfo* getQoreType(java::lang::Class* jc, bool& err);
   */

   DLLLOCAL void initDone() {
      assert(!init_done);
      init_done = true;
   }

   DLLLOCAL QoreNamespace& getRootNS() {
      return default_jns;
   }

   /*
   DLLLOCAL java::lang::Object *toJava(java::lang::Class *jc, const AbstractQoreNode *n, ExceptionSink *xsink);
   DLLLOCAL const QoreTypeInfo *toTypeInfo(java::lang::Class *jc);
   DLLLOCAL AbstractQoreNode *toQore(java::lang::Object *jobj, ExceptionSink *xsink);
   DLLLOCAL QoreClass *loadClass(QoreNamespace &jns, java::lang::ClassLoader *loader, const char *cstr, java::lang::String *jstr = 0, ExceptionSink *xsink = 0);
   */
};

#endif
