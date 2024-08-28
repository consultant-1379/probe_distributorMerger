
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_ietf_jgss_GSSContext__
#define __org_ietf_jgss_GSSContext__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace org
  {
    namespace ietf
    {
      namespace jgss
      {
          class ChannelBinding;
          class GSSContext;
          class GSSCredential;
          class GSSName;
          class MessageProp;
          class Oid;
      }
    }
  }
}

class org::ietf::jgss::GSSContext : public ::java::lang::Object
{

public:
  virtual JArray< jbyte > * initSecContext(JArray< jbyte > *, jint, jint) = 0;
  virtual jint initSecContext(::java::io::InputStream *, ::java::io::OutputStream *) = 0;
  virtual JArray< jbyte > * acceptSecContext(JArray< jbyte > *, jint, jint) = 0;
  virtual void acceptSecContext(::java::io::InputStream *, ::java::io::OutputStream *) = 0;
  virtual jboolean isEstablished() = 0;
  virtual void dispose() = 0;
  virtual jint getWrapSizeLimit(jint, jboolean, jint) = 0;
  virtual JArray< jbyte > * wrap(JArray< jbyte > *, jint, jint, ::org::ietf::jgss::MessageProp *) = 0;
  virtual void wrap(::java::io::InputStream *, ::java::io::OutputStream *, ::org::ietf::jgss::MessageProp *) = 0;
  virtual JArray< jbyte > * unwrap(JArray< jbyte > *, jint, jint, ::org::ietf::jgss::MessageProp *) = 0;
  virtual void unwrap(::java::io::InputStream *, ::java::io::OutputStream *, ::org::ietf::jgss::MessageProp *) = 0;
  virtual JArray< jbyte > * getMIC(JArray< jbyte > *, jint, jint, ::org::ietf::jgss::MessageProp *) = 0;
  virtual void getMIC(::java::io::InputStream *, ::java::io::OutputStream *, ::org::ietf::jgss::MessageProp *) = 0;
  virtual void verifyMIC(JArray< jbyte > *, jint, jint, JArray< jbyte > *, jint, jint, ::org::ietf::jgss::MessageProp *) = 0;
  virtual void verifyMIC(::java::io::InputStream *, ::java::io::InputStream *, ::org::ietf::jgss::MessageProp *) = 0;
  virtual JArray< jbyte > * export$() = 0;
  virtual void requestMutualAuth(jboolean) = 0;
  virtual void requestReplayDet(jboolean) = 0;
  virtual void requestSequenceDet(jboolean) = 0;
  virtual void requestCredDeleg(jboolean) = 0;
  virtual void requestAnonymity(jboolean) = 0;
  virtual void requestConf(jboolean) = 0;
  virtual void requestInteg(jboolean) = 0;
  virtual void requestLifetime(jint) = 0;
  virtual void setChannelBinding(::org::ietf::jgss::ChannelBinding *) = 0;
  virtual jboolean getCredDelegState() = 0;
  virtual jboolean getMutualAuthState() = 0;
  virtual jboolean getReplayDetState() = 0;
  virtual jboolean getSequenceDetState() = 0;
  virtual jboolean getAnonymityState() = 0;
  virtual jboolean isTransferable() = 0;
  virtual jboolean isProtReady() = 0;
  virtual jboolean getConfState() = 0;
  virtual jboolean getIntegState() = 0;
  virtual jint getLifetime() = 0;
  virtual ::org::ietf::jgss::GSSName * getSrcName() = 0;
  virtual ::org::ietf::jgss::GSSName * getTargName() = 0;
  virtual ::org::ietf::jgss::Oid * getMech() = 0;
  virtual ::org::ietf::jgss::GSSCredential * getDelegCred() = 0;
  virtual jboolean isInitiator() = 0;
  static const jint DEFAULT_LIFETIME = 0;
  static const jint INDEFINITE_LIFETIME = 2147483647;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __org_ietf_jgss_GSSContext__
