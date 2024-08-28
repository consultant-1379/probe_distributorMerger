
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_image_BufferStrategy__
#define __java_awt_image_BufferStrategy__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class BufferCapabilities;
        class Graphics;
      namespace image
      {
          class BufferStrategy;
      }
    }
  }
}

class java::awt::image::BufferStrategy : public ::java::lang::Object
{

public:
  BufferStrategy();
  virtual ::java::awt::BufferCapabilities * getCapabilities() = 0;
  virtual ::java::awt::Graphics * getDrawGraphics() = 0;
  virtual jboolean contentsLost() = 0;
  virtual jboolean contentsRestored() = 0;
  virtual void show() = 0;
  static ::java::lang::Class class$;
};

#endif // __java_awt_image_BufferStrategy__
