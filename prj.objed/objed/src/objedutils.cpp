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

#include <objed/objedutils.h>

#include <algorithm>
#include <cstring>

namespace objed
{
  template<class T> bool compareRectsLess(const Rect<T> &r1, const Rect<T> &r2)
  {
    return r1.width * r1.height < r2.width * r2.height;
  }

  template<class T> bool compareRectsGreater(const Rect<T> &r1, const Rect<T> &r2)
  {
    return r1.width * r1.height > r2.width * r2.height;
  }

  template<class T> Json::Value rectToData(const Rect<T> &rect)
  {
    Json::Value data;
    data["x"] = rect.x;
    data["y"] = rect.y;
    data["width"] = rect.width;
    data["height"] = rect.height;
    return data;
  }

  template Json::Value rectToData<int>(const Rect<int> &rect);

  template Json::Value rectToData<double>(const Rect<double> &rect);

  template<class T> Rect<T> dataToRect(const Json::Value &data)
  {
    return Rect<T>();
  }

  template<> Rect<int> dataToRect(const Json::Value &data)
  {
    return Rect<int>(data["x"].asInt(), data["y"].asInt(), 
      data["width"].asInt(), data["height"].asInt());
  }

  template<> Rect<double> dataToRect(const Json::Value &data)
  {
    return Rect<double>(data["x"].asDouble(), data["y"].asDouble(), 
      data["width"].asDouble(), data["height"].asDouble());
  }

  template<class T> Json::Value intervalToData(const Interval<T> &interval)
  {
    Json::Value data;
    data["min"] = interval.mn;
    data["max"] = interval.mx;
    return data;
  }

  template Json::Value intervalToData(const Interval<int> &interval);
  
  template Json::Value intervalToData(const Interval<double> &interval);

  template<class T> Interval<T> dataToInterval(const Json::Value &data)
  {
    return Interval<T>();
  }

  template<> Interval<int> dataToInterval(const Json::Value &data)
  {
    return Interval<int>(data["min"].asInt(), data["max"].asInt());
  }

  template<> Interval<double> dataToInterval(const Json::Value &data)
  {
    return Interval<double>(data["min"].asDouble(), data["max"].asDouble());
  }

  template<class T> std::vector<Cluster<T> > cluster(const std::vector<Rect<T> > &rectList, double overlap)
  {
    std::vector<Cluster<T> > clusterList;
    std::vector<Rect<T> > intClusterList;

    if (overlap >= 1.0)
    {
      for (size_t iRect = 0; iRect < rectList.size(); iRect++)
        clusterList.push_back(Cluster<T>(rectList[iRect], 1));
      return clusterList;
    }

    DetectionList::const_iterator itDet;
    for (size_t iRect = 0; iRect < rectList.size(); iRect++)
    {
      const Rect<T> &rect = rectList[iRect];
      bool rectClustered = false;

      for (int iClust = clusterList.size() - 1; iClust >= 0 && rectClustered == false; iClust--)
      {
        Cluster<T> &cluster = clusterList[iClust];
        Rect<T> &intCluster = intClusterList[iClust];

        Rect<T> intersection = intersected(cluster, rect);
        T intersectionArea = intersection.width * intersection.height;
        T maxArea = std::max(cluster.width * cluster.height, rect.width * rect.height);
        
        if (intersectionArea > maxArea * overlap)
        {
          cluster.power++;
          cluster.x = (intCluster.x += rect.x) / cluster.power;
          cluster.y = (intCluster.y += rect.y) / cluster.power;
          cluster.width = (intCluster.width += rect.width) / cluster.power;
          cluster.height = (intCluster.height += rect.height) / cluster.power;
          rectClustered = true;
        }
      }

      if (rectClustered == false)
      {
        clusterList.push_back(Cluster<T>(rect, 1));
        intClusterList.push_back(rect);
      }
    }

    return clusterList;
  }

  template std::vector<Cluster<int> > cluster(const std::vector<Rect<int> > &rectList, double maxDistance);

  template std::vector<Cluster<double> > cluster(const std::vector<Rect<double> > &rectList, double maxDistance);

  template<class T> std::vector<Cluster<T> > cluster(const std::vector<Cluster<T> > &clusterList, double overlap)
  {
    std::vector<Cluster<T> > outClusterList;
    std::vector<Rect<T> > intOutClusterList;

    if (overlap >= 1.0)
      return clusterList;

    DetectionList::const_iterator itDet;
    for (size_t iClust = 0; iClust < clusterList.size(); iClust++)
    {
      const Cluster<T> &cluster = clusterList[iClust];
      bool clusterClustered = false;

      for (int iOutClust = outClusterList.size() - 1; iOutClust >= 0 && clusterClustered == false; iOutClust--)
      {
        Cluster<T> &outCluster = outClusterList[iOutClust];
        Rect<T> &intOutCluster = intOutClusterList[iOutClust];

        Rect<T> intersection = intersected(outCluster, cluster);
        T intersectionArea = intersection.width * intersection.height;
        T maxArea = std::max(outCluster.width * outCluster.height, cluster.width * cluster.height);

        if (intersectionArea > maxArea * overlap)
        {
          outCluster.power += cluster.power;
          outCluster.x = (intOutCluster.x += cluster.x * cluster.power) / outCluster.power;
          outCluster.y = (intOutCluster.y += cluster.y * cluster.power) / outCluster.power;
          outCluster.width = (intOutCluster.width += cluster.width * cluster.power) / outCluster.power;
          outCluster.height = (intOutCluster.height += cluster.height * cluster.power) / outCluster.power;
          clusterClustered = true;
        }
      }

      if (clusterClustered == false)
      {
        outClusterList.push_back(cluster);
        intOutClusterList.push_back(Rect<T>(cluster.x * cluster.power, cluster.y * cluster.power,
          cluster.width * cluster.power, cluster.height * cluster.power));
      }
    }

    return outClusterList;
  }

  template std::vector<Cluster<int> > cluster(const std::vector<Cluster<int> > &clusterList, double maxDistance);

  template std::vector<Cluster<double> > cluster(const std::vector<Cluster<double> > &clusterList, double maxDistance);

  template<class T> std::vector<Cluster<T> > mergeIncluded(const std::vector<Cluster<T> > &clusterList)
  {
    std::vector<Cluster<T> > oldClusterList = clusterList;
    std::sort(oldClusterList.begin(), oldClusterList.end(), compareRectsGreater<T>);

    std::vector<Cluster<T> > newClusterList;
    for (typename std::vector<Cluster<T> >::iterator itOld = oldClusterList.begin(); itOld != oldClusterList.end(); ++itOld)
    {
      bool isIncluded = false;
      const Cluster<T> &oldCluster = *itOld;

      for (typename std::vector<Cluster<T> >::iterator itNew = newClusterList.begin(); itNew != newClusterList.end(); ++itNew)
      {
        Cluster<T> &newCluster = *itNew;
        Rect<T> intersectedRect = intersected(newCluster, oldCluster);
        if (memcmp(&intersectedRect, &oldCluster, sizeof(intersectedRect)) == 0)
        {
          newCluster.power += oldCluster.power;
          isIncluded = true;
        }
      }

      if (isIncluded == false)
        newClusterList.push_back(oldCluster);
    }

    return newClusterList;
  }

  template std::vector<Cluster<int> > mergeIncluded(const std::vector<Cluster<int> > &clusterList);

  template std::vector<Cluster<double> > mergeIncluded(const std::vector<Cluster<double> > &clusterList);
}
