// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <string>
// ���� carla �����ռ䣬����������֯�� "Carla" ��ص�һϵ���ࡢ�����ȴ���ṹ��ʹ���ж����������򣬱���������ͻ
namespace carla {
// �� carla �����ռ����ٶ��� ros2 �����ռ�
namespace ros2 {
    // ���� CarlaSubscriber �࣬�������Ʋ��������������� ROS2 �����¶��� Carla �����Ϣ�Ļ���
  class CarlaSubscriber {
    public:
        // ��ȡ frame_id��֡ ID���ַ����ĳ����ã�ͨ���� ROS ��֡ ID ���ڱ�ʶ���������Ĳο�����ϵ����Ϣ���ⲿ��ͨ���˺�����ȡ�ó�Ա������ֵ���������޸���
      const std::string& frame_id() const { return _frame_id; }
      // ��ȡ name�����ƣ��ַ����ĳ����ã�������ƿ������ڱ�ʶ�ö����ߵ��ض����ƣ��������ֲ�ͬ�Ķ��Ķ����
      const std::string& name() const { return _name; }
      // ��ȡ parent����������أ��ַ����ĳ����ã��������ڱ�ʾ�ö����������ĸ���������߶���������Ϣ
      const std::string& parent() const { return _parent; }
      // ���� frame_id ��Ա������ͨ����ֵ���ý���һ���µ��ַ�������ʹ�� std::move ����Դ����Ȩת�Ƹ���Ա���������ڸ���֡ ID �����Ϣ
      void frame_id(std::string&& frame_id) { _frame_id = std::move(frame_id); }
      // ���� name ��Ա������ԭ��ͬ frame_id �����ú��������ڸ��¶�����������Ϣ
      void name(std::string&& name) { _name = std::move(name); }
      // ���� parent ��Ա���������ڸ��¸����������Ϣ
      void parent(std::string&& parent) { _parent = std::move(parent); }

      virtual const char* type() const = 0;

    public:
      CarlaSubscriber() = default;
      // ������������Ĭ��ʵ�֣��������������������ʱ��ȷ�ؽ����ڴ������������ز���
      virtual ~CarlaSubscriber() = default;

    protected:
        // �洢֡ ID ���ַ�����Ա��������ʼ��Ϊ���ַ��������ڱ�����ö�������ص����ݵĲο�����ϵ�ȱ�ʶ��Ϣ
      std::string _frame_id = "";
      // �洢���������Ƶ��ַ�����Ա��������ʼ��Ϊ���ַ������������ֲ�ͬ�Ķ�����
      std::string _name = "";
      // �洢�����������Ϣ���ַ�����Ա��������ʼ��Ϊ���ַ������������ڱ�ʾ�������ϼ���������
      std::string _parent = "";
  };
}
}
