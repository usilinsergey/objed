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

#include <json-cpp/reader.h>

#include <algorithm>
#include <fstream>
#include <cassert>
#include <cstring>
#include <string>

#if defined _MSC_VER
#  undef NOMINMAX
#  define NOMINMAX
#  include <windows.h>
#else  // _MSC_VER
#  include <dlfcn.h>
#  include <unistd.h>
#endif // _MSC_VER

template<class T>
inline static void delete_ptr(T *&ptr)
{
  delete ptr; ptr = 0;
}

static std::string canonizePath(const std::string &path)
{
  std::string canonizedPath = path;
  for(size_t index = 0; index = canonizedPath.find("\\", index), index != std::string::npos;)
  {
    canonizedPath.replace(index, 1, "/");
    index++;
  }

  return canonizedPath;
}

static std::string canonizeDirPath(const std::string &dirPath)
{
  if (dirPath.empty() == true)
    return dirPath;

  std::string canonizedDirPath = canonizePath(dirPath);
  if (canonizedDirPath[canonizedDirPath.length() - 1] != '/')
    canonizedDirPath.append("/");

  return canonizedDirPath;
}

static std::string absolutePath(const std::string &name, const std::string &workDir)
{
#ifdef _MSC_VER
  if (name.find(":") != std::string::npos)
    return name;
#endif  // _MSC_VER

  return canonizeDirPath(workDir) + name;
}

static std::string dirPath(const std::string &path)
{
  int index = canonizePath(path).rfind("/");
  if (index != std::string::npos)
    return path.substr(0, index);
  return std::string();
}

static std::string fileName(const std::string &path)
{
  int index = canonizePath(path).rfind("/");
  if (index != std::string::npos)
    return path.substr(index + 1);
  return path;
}

static void * resolveFunction(const std::string &libraryName, const std::string &functionName)
{
#ifdef _MSC_VER
  HMODULE library = LoadLibraryA(libraryName.c_str());
  return GetProcAddress(library, functionName.c_str());
#else  // _MSC_VER
#ifdef __APPLE__
  void *filterLib = dlopen((libraryName + ".dylib").c_str(), RTLD_LAZY);;
#else  // __APPLE__
  void *filterLib = dlopen((libraryName + ".so").c_str(), RTLD_LAZY);
#endif // __APPLE__
  return dlsym(filterLib, functionName.c_str());
#endif // _MSC_VER
}

#include <objed/objed.h>

#include "imagepool.h"

#include "maxcl.h"
#include "linearcl.h"
#include "additivecl.h"
#include "cascadecl.h"
#include "parallelcl.h"
#include "treecl.h"
#include "haar1stumpcl.h"
#include "haar2stumpcl.h"
#include "haar3stumpcl.h"
#include "haar1pwcl.h"
#include "haar2pwcl.h"
#include "haar3pwcl.h"
#include "meancl.h"
#include "roicl.h"

#include "simpledet.h"
#include "yscaledet.h"
#include "lazydet.h"
#include "multidet.h"

objed::ImagePool * objed::ImagePool::create()
{
  return new ImagePoolImpl();
}

void objed::ImagePool::destroy(ImagePool *&imagePool)
{
  delete imagePool;
  imagePool = 0;
}

objed::Classifier * objed::Classifier::create(const std::string &path, const std::string &workDir)
{
  Json::Reader reader;
  Json::Value valueData;

  std::string absPath = absolutePath(path, workDir);
  std::ifstream ifs(absPath.c_str(), std::ios_base::in);
  if (ifs.is_open() == false)
    return 0;
  
  reader.parse(ifs, valueData);
  return create(valueData, dirPath(absPath));
}

objed::Classifier * objed::Classifier::create(const Json::Value &data, const std::string &workDir)
{
  if (data.empty() == false && data.isString() == true)
    return objed::Classifier::create(data.asString(), workDir);

  if (data.empty() == true || data.isObject() == false)
    return 0;

  std::string clType = data["type"].asString();

  if (clType == MaxClassifier::typeStatic())
    return new MaxClassifier(data);
  else if (clType == LinearClassifier::typeStatic())
    return new LinearClassifier(data);
  else if (clType == AdditiveClassifier::typeStatic())
    return new AdditiveClassifier(data);
  else if (clType == CascadeClassifier::typeStatic())
    return new CascadeClassifier(data);
  else if (clType == ParallelClassifier::typeStatic())
    return new ParallelClassifier(data);
  else if (clType == TreeClassifier::typeStatic())
    return new TreeClassifier(data);
  else if (clType == Haar1StumpClassifier::typeStatic())
    return new Haar1StumpClassifier(data);
  else if (clType == Haar2StumpClassifier::typeStatic())
    return new Haar2StumpClassifier(data);
  else if (clType == Haar3StumpClassifier::typeStatic())
    return new Haar3StumpClassifier(data);
  else if (clType == Haar1PwClassifier::typeStatic())
    return new Haar1PwClassifier(data);
  else if (clType == Haar2PwClassifier::typeStatic())
    return new Haar2PwClassifier(data);
  else if (clType == Haar3PwClassifier::typeStatic())
    return new Haar3PwClassifier(data);
  else if (clType == MeanClassifier::typeStatic())
    return new MeanClassifier(data);
  else if (clType == RoiClassifier::typeStatic())
    return new RoiClassifier(data);
  else
  {
    std::transform(clType.begin(), clType.end(), clType.begin(), ::tolower);
    CreateFn *createFn = (CreateFn *)resolveFunction(clType, clType + "_create");
    return createFn != 0 ? createFn(data) : 0;
  }

  return 0;
}

void objed::Classifier::destroy(Classifier *&classifier)
{
  if (classifier == 0)
    return;
  
  std::string clType = classifier->type();

  if (clType == MaxClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == LinearClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == AdditiveClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == CascadeClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == ParallelClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == TreeClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == Haar1StumpClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == Haar2StumpClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == Haar3StumpClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == Haar1PwClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == Haar2PwClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == Haar3PwClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == MeanClassifier::typeStatic())
    delete_ptr(classifier);
  else if (clType == RoiClassifier::typeStatic())
    delete_ptr(classifier);
  else 
  {
    std::transform(clType.begin(), clType.end(), clType.begin(), ::tolower);
    DestroyFn *destroyFn = (DestroyFn *)resolveFunction(clType, clType + "_destroy");
    if (destroyFn != 0) destroyFn(classifier);
  }
}

objed::Detector * objed::Detector::create(const std::string &path, const std::string &workDir)
{
  Json::Reader reader;
  Json::Value valueData;

  std::string absPath = absolutePath(path, workDir);
  std::ifstream ifs(absPath.c_str(), std::ios_base::in);
  if (ifs.is_open() == false)
    return 0;

  reader.parse(ifs, valueData);
  return create(valueData, dirPath(absPath));
}

objed::Detector * objed::Detector::create(const Json::Value &data, const std::string &workDir)
{
  if (data.empty() == false && data.isString() == true)
    return objed::Detector::create(data.asString(), workDir);

  if (data.empty() == true || data.isObject() == false)
    return 0;

  std::string detType = data["type"].asString();

  if (detType == SimpleDetector::typeStatic())
    return new SimpleDetector(data, workDir);
  else if (detType == YScaleDetector::typeStatic())
    return new YScaleDetector(data, workDir);
  else if (detType == LazyDetector::typeStatic())
    return new LazyDetector(data, workDir);
  else if (detType == MultiDetector::typeStatic())
    return new MultiDetector(data, workDir);
  else
  {
    std::transform(detType.begin(), detType.end(), detType.begin(), ::tolower);
    CreateFn *createFn = (CreateFn *)resolveFunction(detType, detType + "_create");
    return createFn != 0 ? createFn(data) : 0;
  }
}

void objed::Detector::destroy(Detector *&detector)
{
  if (detector == 0)
    return;

  std::string detType = detector->type();

  if (detType == SimpleDetector::typeStatic())
    delete_ptr(detector);
  else if (detType == YScaleDetector::typeStatic())
    delete_ptr(detector);
  else if (detType == LazyDetector::typeStatic())
    delete_ptr(detector);
  else if (detType == MultiDetector::typeStatic())
    delete_ptr(detector);
  else
  {
    std::transform(detType.begin(), detType.end(), detType.begin(), ::tolower);
    DestroyFn *destroyFn = (DestroyFn *)resolveFunction(detType, detType + "_destroy");
    if (destroyFn != 0) destroyFn(detector);
  }
}

OBJED_API objed::ImagePool * CreateImagePool()
{
  return objed::ImagePool::create();
}

OBJED_API void DestroyImagePool(objed::ImagePool *imagePool)
{
  objed::ImagePool::destroy(imagePool);
}

OBJED_API objed::Classifier * CreateClassifierFromFile(const char *path)
{
  if (path == 0 || strlen(path) == 0)
    return 0;

  return objed::Classifier::create(std::string(path));
}

OBJED_API objed::Classifier * CreateClassifierFromJson(const char *data)
{
  if (data == 0 || strlen(data) == 0)
    return 0;

  Json::Reader reader; Json::Value root;
  if (reader.parse(std::string(data), root, false) == false)
    return 0;

  return objed::Classifier::create(root);
}

OBJED_API void DestroyClassifier(objed::Classifier *classifier)
{
  objed::Classifier::destroy(classifier);
}

OBJED_API objed::Detector * CreateDetectorFromFile(const char *path)
{
  if (path == 0 || strlen(path) == 0)
    return 0;

  return objed::Detector::create(std::string(path));
}

OBJED_API objed::Detector * CreateDetectorFromJson(const char *data)
{
  if (data == 0 || strlen(data) == 0)
    return 0;

  Json::Reader reader; Json::Value root;
  if (reader.parse(std::string(data), root, false) == false)
    return 0;

  return objed::Detector::create(root);
}

OBJED_API void DestroyDetector(objed::Detector *detector)
{
  objed::Detector::destroy(detector);
}
