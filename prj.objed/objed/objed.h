/*
Copyright (c) 2011-2013, Sergey Usilin. All rights reserved.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of copyright holders.
*/

#pragma once
#ifndef OBJED_H_INCLUDED
#define OBJED_H_INCLUDED

#include <objed/simplest.h>
#include <json-cpp/json.h>

#include <opencv2/core.hpp>

namespace objed
{
  class ImagePool
  {
  public:
    virtual bool update(IplImage *image) = 0;
    virtual IplImage * integral(const std::string &id) = 0;
    virtual IplImage * image(const std::string &id) = 0;
    virtual IplImage * base() = 0;

    virtual std::vector<std::string> imageNames() const = 0;
    virtual std::vector<std::string> integralNames() const = 0;

  public:
    virtual ~ImagePool() {};

  public:
    static ImagePool * create();
    static void destroy(ImagePool *&imagePool);
  };

  class DebugInfo
  {
  public:
    std::map<int, int> int_data;
  };

  class Classifier
  {
  public:
    static const int DBG_SC_COUNT = 1;

  public:
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual bool prepare(ImagePool *imagePool) = 0;
    virtual bool evaluate(float *result, int x, int y, DebugInfo *debugInfo = 0) const = 0;
    virtual Json::Value serialize() const = 0;
    virtual std::string type() const = 0;
    virtual Classifier * clone() const = 0;

  public:
    typedef Classifier * (CreateFn)(const Json::Value &);
    typedef void (DestroyFn)(Classifier *&);

  public:
    virtual ~Classifier() {};

  public:
    static Classifier * create(const std::string &path, const std::string &workDir = "");
    static Classifier * create(const Json::Value &data, const std::string &workDir = "");
    static void destroy(Classifier *&classifier);
  };

  class Detector
  {
  public:
    static const int DBG_TOTAL_SC_COUNT    = 1001;
    static const int DBG_MIN_SC_COUNT      = 1002;
    static const int DBG_MAX_SC_COUNT      = 1003;
    static const int DBG_EVALUATION_COUNT  = 1004;

  public:
    virtual DetectionList detect(IplImage *image, DebugInfo *debugInfo = 0) = 0;
    
    virtual std::string type() const = 0;
    virtual Detector * clone() const = 0;

    virtual void setImagePool(objed::ImagePool *extImagePool) = 0;
    virtual void resetImagePool() = 0;

  public:
    typedef Detector * (CreateFn)(const Json::Value &);
    typedef void (DestroyFn)(Detector *&);

  public:
    virtual ~Detector() {};

  public:
    static Detector * create(const std::string &path, const std::string &workDir = "");
    static Detector * create(const Json::Value &data, const std::string &workDir = "");
    static void destroy(Detector *&detector);
  };
}


#if defined _MSC_VER && defined objed_EXPORTS
#  define OBJED_API __declspec(dllexport)
#elif defined _MSC_VER && defined objedocv_EXPORTS 
#  define OBJED_API __declspec(dllexport)
#else
#  define OBJED_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

  OBJED_API objed::ImagePool * CreateImagePool();
  OBJED_API void DestroyImagePool(objed::ImagePool *imagePool);

  OBJED_API objed::Classifier * CreateClassifierFromFile(const char *path);
  OBJED_API objed::Classifier * CreateClassifierFromJson(const char *data);
  OBJED_API void DestroyClassifier(objed::Classifier *classifier);

  OBJED_API objed::Detector * CreateDetectorFromFile(const char *path);
  OBJED_API objed::Detector * CreateDetectorFromJson(const char *data);
  OBJED_API void DestroyDetector(objed::Detector *detector);

#ifdef __cplusplus
}
#endif

#endif  // OBJED_H_INCLUDED