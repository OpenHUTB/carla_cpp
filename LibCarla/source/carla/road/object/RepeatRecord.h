// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // ȷ�����ͷ�ļ���һ�����뵥Ԫ��ֻ������һ�� 

#include "carla/NonCopyable.h"  // ����carla����е�NonCopyable�࣬���ڽ�ֹ�������� 
#include <string>  // ������׼���е�string�� 
#include <vector>  // ������׼����vector�࣬���ڴ���̬���� 
#include "carla/road/RoadTypes.h"  // ����carla�����roadģ���·�Ͷ��� 

namespace carla {
namespace road {
namespace object {
  // RepeatRecord�ඨ�壬���̳���MovableNonCopyable,����˽�м̳���NonCopyable,  
  // ��ζ��RepeatRecord��Ҳ���ܱ��������������ƶ������MovableNonCopyable��ʵ�����ƶ����壩 
  // ����౻���Ϊֻ����˽�г�Ա�������䲻���㱻�ⲿֱ��ʹ�� 
  class RepeatRecord : private MovableNonCopyable {
  public:
    // Ĭ�Ϲ��캯������ִ���κβ��� 
    RepeatRecord() {}

  private:

    ObjId _id;
  };

} // object
} // road
} // carla
