//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2015 Qore Technologies
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------
///
/// \file
/// \brief TODO file description
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_INVOCATIONHANDLER_H_
#define QORE_JNI_INVOCATIONHANDLER_H_

#include <memory>
#include "Object.h"
#include "Dispatcher.h"

extern QoreClass* QC_INVOCATIONHANDLER;
extern qore_classid_t CID_INVOCATIONHANDLER;

namespace jni {

class InvocationHandler : public ObjectBase {

public:
   InvocationHandler(std::unique_ptr<Dispatcher> dispatcher);
   InvocationHandler(const ResolvedCallReferenceNode *callback);

   jobject getJavaObject() const override {
      return handler;
   }

   void destroy();

private:
   GlobalReference<jobject> handler;
};

} // namespace jni

#endif // QORE_JNI_INVOCATIONHANDLER_H_
