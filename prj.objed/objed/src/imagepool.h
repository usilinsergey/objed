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
#ifndef IMAGEPOOL_H_INCLUDED
#define IMAGEPOOL_H_INCLUDED

#include <objed/objed.h>

#include <string>
#include <vector>
#include <map>

namespace objed
{
  class ImagePoolItem
  {
  public:
    ImagePoolItem();
    virtual ~ImagePoolItem();

  private:
    ImagePoolItem(const ImagePoolItem &);
    ImagePoolItem &operator=(const ImagePoolItem &);

  public:
    IplImage * image(int width, int height, int channels, int depth);
    IplImage * image() const;

  private:
    IplImage *regionImage, *sourceImage;
  };

  class ImagePoolImpl : public ImagePool
  {
  public:
    ImagePoolImpl();
    virtual ~ImagePoolImpl();

  private:
    ImagePoolImpl(const ImagePoolImpl &);
    ImagePoolImpl &operator=(const ImagePoolImpl &);

  public:
    virtual bool update(IplImage *image);
    virtual IplImage *integral(const std::string &id);
    virtual IplImage *image(const std::string &id);
    virtual IplImage *base();

    virtual std::vector<std::string> imageNames() const;
    virtual std::vector<std::string> integralNames() const;

  private:
    ImagePoolItem *baseItem;
    std::map<std::string, ImagePoolItem *> imageItems;
    std::map<std::string, ImagePoolItem *> integralItems;
  };
}

#endif  // IMAGEPOOL_H_INCLUDED