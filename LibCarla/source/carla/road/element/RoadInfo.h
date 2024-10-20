// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfoVisitor.h"// ����RoadInfoVisitor��Ķ��壬���ڷ�����ģʽ��
#include "carla/NonCopyable.h"// ����NonCopyable��Ķ��壬���ڽ�ֹ����������

#include <map>// �ṩmap�����Ķ��塣
#include <string>// �ṩstring��Ķ��塣
#include <vector>// �ṩvector�����Ķ��塣
//ʹ��carla�����ռ��µ�road��element�������ռ䡣
namespace carla {
    namespace road {
        namespace element {
            // ����RoadInfo�࣬���̳���NonCopyable��˽�м̳У������ڽ�ֹ����������
            class RoadInfo : private NonCopyable {
            public:
                // ����һ��������������ȷ���������ܹ���ȷ������
                virtual ~RoadInfo() = default;
                // ����һ�����麯��AcceptVisitor������һ��RoadInfoVisitor������Ϊ������
                // ���Ƿ�����ģʽ��ʵ�֣������ⲿͨ�������߶������RoadInfo���ڲ�״̬��
                virtual void AcceptVisitor(RoadInfoVisitor&) = 0;
                /// ��ȡ�ӵ�·��ʼλ�õľ��롣
                /// Distance from road's start location.
                double GetDistance() const {
                    return _s;// ����˽�г�Ա����_s��ֵ��
                }

            protected:
                // �����ܱ����Ĺ��캯�������������๹��RoadInfo����
                // ����Ĭ�ϲ���0.0����ʾĬ�Ͼ���Ϊ0��
                RoadInfo(double distance = 0.0) : _s(distance) {}

            private:
                // ˽�г�Ա����_s���洢�ӵ�·��ʼλ�õľ��롣
                double _s;
            };

        } // namespace element
    } // namespace road
} // namespace carla
