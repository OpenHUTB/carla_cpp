// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once 
///@brief pugi XML������������ռ�
namespace pugi {
    ///@class xml_document
    ///@brief ��ʾһ��XML�ĵ�
  class xml_document; 
} // namespace pugi
///@brief Carlaģ�����������ռ�
namespace carla {
 ///@brief Carla�е�·��ع��ܵ������ռ�
namespace road {
    ///@class MapBuilder
    ///@brief ������Carlaģ�����й�����ͼ����
  class MapBuilder;
} // namespace road
///@brief ����OpenDrive��ʽ��ͼ���ݵ������ռ�
namespace opendrive {
namespace parser {
///@class ControllerParser
///@brief ���ڽ���Opendrive���������ݵ���
  class ControllerParser {
  public:
      ///@brief ����XML�ĵ���ʹ���ṩ��MapBuilder������ͼ
      ///@param xml Ҫ������XML�ĵ��ĳ�������
      ///@param map_builder �����ڹ�����ͼ��MapBuilder��������á�
    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
