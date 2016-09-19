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
/// \brief Contains various definitions.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_DEFS_H_
#define QORE_JNI_DEFS_H_

#include <memory>
#include <qore/Qore.h>
#include <jni.h>

namespace jni {

/**
 * \brief Logging level used by the JNI module in calls to printd().
 */
constexpr int LogLevel = 1;

/**
 * \brief Base class for exceptions.
 */
class Exception {

public:
   /**
    * \brief Default virtual destructor.
    */
   virtual ~Exception() = default;

   /**
    * \brief Raises the corresponding Qore exception in the ExceptionSink.
    * \param xsink the exception sink
    */
   virtual void convert(ExceptionSink *xsink) = 0;

   Exception(Exception &&) = default;
   Exception &operator=(Exception &&) = default;

protected:
   /**
    * \brief Default constructor.
    */
   Exception() = default;

private:
   Exception(const Exception &) = delete;
   Exception &operator=(const Exception &) = delete;
};

/**
 * \brief An exception thrown when the current thread cannot be attached to the JVM.
 */
class UnableToAttachException : public Exception {

public:
   /**
    * \brief Constructor.
    * \param err the error code
    */
   UnableToAttachException(jint err) : err(err) {
      printd(LogLevel, "JNI - error attaching thread %d, err: %d\n", gettid(), err);
   }

   void convert(ExceptionSink *xsink) override {
      xsink->raiseException("JNI-ERROR", "Unable to attach thread to the JVM, error %d", err);
   }

private:
   jint err;
};

/**
 * \brief An exception thrown when the current thread cannot be registered with Qore.
 */
class UnableToRegisterException : public Exception {

public:
   /**
    * \brief Constructor.
    */
   UnableToRegisterException() {
      printd(LogLevel, "JNI - error registering thread %ld with Qore\n", pthread_self());
   }

   void convert(ExceptionSink *xsink) override {
      xsink->raiseException("JNI-ERROR", "Unable to register thread with Qore");
   }
};

/**
 * \brief A C++ exception representing a Java exception.
 */
class JavaException : public Exception {

public:
   /**
    * \brief Constructor.
    */
   JavaException() {
   }

   void convert(ExceptionSink *xsink) override;
};

/**
 * \brief Basic exception with a simple string messsage.
 */
class BasicException : public Exception {

public:
   /**
    * \brief Constructor.
    * \param message the exception message
    */
   BasicException(std::string message) : message(std::move(message)) {
   }

   void convert(ExceptionSink *xsink) override {
      xsink->raiseException("JNI-ERROR", message.c_str());
   }

private:
   std::string message;
};

/**
 * \brief A C++ wrapper for an exception raised in the ExceptionSink.
 */
class XsinkException : public Exception {

public:
   /**
    * \brief Constructor.
    * \param xsink the exception sink with an exception
    */
   XsinkException(ExceptionSink &xsink) : sink(new ExceptionSink()) {
      sink->assimilate(xsink);
   }

   void convert(ExceptionSink *xsink) override {
      xsink->assimilate(*sink);
   }

private:
   std::unique_ptr<ExceptionSink> sink;
};

class QoreThreadAttacher {

public:
   QoreThreadAttacher() : attached(false) {
   }

   ~QoreThreadAttacher() {
      if (attached) {
         printd(LogLevel, "Detaching thread %ld from Qore\n", pthread_self());
         q_deregister_foreign_thread();
      }
   }

   void attach() {
      int rc = q_register_foreign_thread();
      if (rc == QFT_OK) {
         attached = true;
         printd(LogLevel, "Thread %ld attached to Qore\n", pthread_self());
      } else if (rc != QFT_REGISTERED) {
         throw UnableToRegisterException();
      }
   }

private:
   bool attached;
};

extern thread_local QoreThreadAttacher qoreThreadAttacher;

} // namespace jni

#endif // QORE_JNI_DEFS_H_
