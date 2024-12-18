// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/image/ImageConverter.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/pointcloud/PointCloudIO.h>
#include <carla/sensor/SensorData.h>
#include <carla/sensor/data/CollisionEvent.h>
#include <carla/sensor/data/IMUMeasurement.h>
#include <carla/sensor/data/ObstacleDetectionEvent.h>
#include <carla/sensor/data/Image.h>
#include <carla/sensor/data/LaneInvasionEvent.h>
#include <carla/sensor/data/LidarMeasurement.h>
#include <carla/sensor/data/SemanticLidarMeasurement.h>
#include <carla/sensor/data/GnssMeasurement.h>
#include <carla/sensor/data/RadarMeasurement.h>
#include <carla/sensor/data/DVSEventArray.h>
#include <carla/sensor/data/V2XEvent.h>
#include <carla/sensor/data/V2XData.h>
#include <carla/sensor/data/LibITS.h>

#include <carla/sensor/data/RadarData.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ostream>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <thread>

namespace carla {
namespace sensor {
namespace data {

  std::ostream &operator<<(std::ostream &out, const Image &image) {
    out << "Image(frame=" << std::to_string(image.GetFrame())
        << ", timestamp=" << std::to_string(image.GetTimestamp())
        << ", size=" << std::to_string(image.GetWidth()) << 'x' << std::to_string(image.GetHeight())
        << ')';
    return out;
  }

// 为Image类型重载输出流运算符，允许Image对象可以直接输出到标准输出流（如cout）中。
// 输出内容包括帧编号、时间戳、图像宽度和高度。  
 
  std::ostream &operator<<(std::ostream &out, const OpticalFlowImage &image) {
    out << "OpticalFlowImage(frame=" << std::to_string(image.GetFrame())
        << ", timestamp=" << std::to_string(image.GetTimestamp())
        << ", size=" << std::to_string(image.GetWidth()) << 'x' << std::to_string(image.GetHeight())
        << ')';
    return out;
  }

// 为OpticalFlowImage类型重载输出流运算符，功能类似Image类型，用于输出光学流图像的信息。

  std::ostream &operator<<(std::ostream &out, const LidarMeasurement &meas) {
    out << "LidarMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", number_of_points=" << std::to_string(meas.size())
        << ')';
    return out;
  }

// 为LidarMeasurement类型重载输出流运算符，输出激光雷达测量的信息，包括帧编号、时间戳和点数量。

  std::ostream &operator<<(std::ostream &out, const SemanticLidarMeasurement &meas) {
    out << "SemanticLidarMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", number_of_points=" << std::to_string(meas.size())
        << ')';
    return out;
  }

// 为SemanticLidarMeasurement类型重载输出流运算符，输出语义激光雷达测量的信息，类似LidarMeasurement。

  std::ostream &operator<<(std::ostream &out, const CollisionEvent &meas) {
    out << "CollisionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", other_actor=" << meas.GetOtherActor()
        << ')';
    return out;
  }

// 为CollisionEvent类型重载输出流运算符，输出碰撞事件的信息，包括帧编号、时间戳和碰撞的其他参与者。

  std::ostream &operator<<(std::ostream &out, const ObstacleDetectionEvent &meas) {
    out << "ObstacleDetectionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", other_actor=" << meas.GetOtherActor()
        << ')';
    return out;
  }

// 为ObstacleDetectionEvent类型重载输出流运算符，输出障碍物检测事件的信息，类似CollisionEvent。

  std::ostream &operator<<(std::ostream &out, const LaneInvasionEvent &meas) {
    out << "LaneInvasionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ')';
    return out;
  }

// 为LaneInvasionEvent类型重载输出流运算符，输出车道入侵事件的信息，包括帧编号和时间戳。

  std::ostream &operator<<(std::ostream &out, const GnssMeasurement &meas) {
    out << "GnssMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", lat=" << std::to_string(meas.GetLatitude())
        << ", lon=" << std::to_string(meas.GetLongitude())
        << ", alt=" << std::to_string(meas.GetAltitude())
        << ')';
    return out;
  }

// 为GnssMeasurement类型重载输出流运算符，输出GNSS测量信息，包括帧编号、时间戳、纬度、经度和海拔。

  std::ostream &operator<<(std::ostream &out, const IMUMeasurement &meas) {
    out << "IMUMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", accelerometer=" << meas.GetAccelerometer()
        << ", gyroscope=" << meas.GetGyroscope()
        << ", compass=" << std::to_string(meas.GetCompass())
        << ')';
    return out;
  }

// 为IMUMeasurement类型重载输出流运算符，输出IMU测量信息，包括帧编号、时间戳、加速度计、陀螺仪和指南针数据。

  std::ostream &operator<<(std::ostream &out, const RadarMeasurement &meas) {
    out << "RadarMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", point_count=" << std::to_string(meas.GetDetectionAmount())
        << ')';
    return out;
  }

// 为RadarMeasurement类型重载输出流运算符，输出雷达测量信息，包括帧编号、时间戳和检测到的点数。

  std::ostream &operator<<(std::ostream &out, const DVSEvent &event) {
    out << "Event(x=" << std::to_string(event.x)
        << ", y=" << std::to_string(event.y)
        << ", t=" << std::to_string(event.t)
        << ", pol=" << std::to_string(event.pol) << ')';
    return out;
  }

// 为DVSEvent类型重载输出流运算符，输出动态视觉传感器事件的信息，包括位置坐标、时间和极性。

  std::ostream &operator<<(std::ostream &out, const DVSEventArray &events) {
    out << "EventArray(frame=" << std::to_string(events.GetFrame())
        << ", timestamp=" << std::to_string(events.GetTimestamp())
        << ", dimensions=" << std::to_string(events.GetWidth()) << 'x' << std::to_string(events.GetHeight())
        << ", number_of_events=" << std::to_string(events.size())
        << ')';
    return out;
  }

// 为DVSEventArray类型重载输出流运算符，输出动态视觉传感器事件数组的信息，包括帧编号、时间戳、尺寸和事件数量。

  std::ostream &operator<<(std::ostream &out, const RadarDetection &det) {
    out << "RadarDetection(velocity=" << std::to_string(det.velocity)
        << ", azimuth=" << std::to_string(det.azimuth)
        << ", altitude=" << std::to_string(det.altitude)
        << ", depth=" << std::to_string(det.depth)
        << ')';
    return out;
  }

// 为RadarDetection类型重载输出流运算符，输出雷达检测到的物体的信息，包括速度、方位角、高度和深度。

  std::ostream &operator<<(std::ostream &out, const LidarDetection &det) {
    out << "LidarDetection(x=" << std::to_string(det.point.x)
        << ", y=" << std::to_string(det.point.y)
        << ", z=" << std::to_string(det.point.z)
        << ", intensity=" << std::to_string(det.intensity)
        << ')';
    return out;
  }

// 为LidarDetection类型重载输出流运算符，输出激光雷达检测到的点的信息，包括坐标和强度。

  std::ostream &operator<<(std::ostream &out, const SemanticLidarDetection &det) {
    out << "SemanticLidarDetection(x=" << std::to_string(det.point.x)
        << ", y=" << std::to_string(det.point.y)
        << ", z=" << std::to_string(det.point.z)
        << ", cos_inc_angle=" << std::to_string(det.cos_inc_angle)
        << ", object_idx=" << std::to_string(det.object_idx)
        << ", object_tag=" << std::to_string(det.object_tag)
        << ')';
    return out;
  }

// 为SemanticLidarDetection类型重载输出流运算符，输出语义激光雷达检测到的点的信息，包括坐标、余弦入射角和对象索引及标签。

  std::ostream &operator<<(std::ostream &out, const CAMEvent &data) {
    out << "CAMEvent(frame=" << std::to_string(data.GetFrame())
        << ", timestamp=" << std::to_string(data.GetTimestamp())
        << ", message_count=" << std::to_string(data.GetMessageCount())
        << ')';
    return out;
  }
// 为CAMEvent类型重载输出流运算符，输出CAMEvent（可能是指某种与通信相关的事件）的信息，包括帧编号、时间戳和消息数量。

std::ostream &operator<<(std::ostream &out, const CustomV2XEvent &data) {  
    out << "CustomV2XEvent(frame=" << std::to_string(data.GetFrame()) // 打印帧号  
        << ", timestamp=" << std::to_string(data.GetTimestamp())      // 打印时间戳  
        << ", message_count=" << std::to_string(data.GetMessageCount()) // 打印消息数量  
        << ')';  
    return out; // 返回输出流对象，支持链式调用  
}  
  
// 为CAMData类型重载<<运算符  
std::ostream &operator<<(std::ostream &out, const CAMData &data) {  
    out << "CAMData(power=" << std::to_string(data.Power) // 打印功率  
        << ", stationId=" << std::to_string(data.Message.header.stationID) // 打印站点ID  
        << ", messageId=" << std::to_string(data.Message.header.messageID) // 打印消息ID  
        << ')';  
    return out;  
}  
  
// 为CustomV2XData类型重载<<运算符，其实现与CAMData类似  
std::ostream &operator<<(std::ostream &out, const CustomV2XData &data) {  
    out << "CustomV2XData(power=" << std::to_string(data.Power)  
        << ", stationId=" << std::to_string(data.Message.header.stationID)  
        << ", messageId=" << std::to_string(data.Message.header.messageID)  
        << ')';  
    return out;  
}  
  
} // namespace s11n  
} // namespace sensor  
} // namespace carla  
  
// 定义一个枚举类EColorConverter，用于表示不同的颜色转换器类型  
enum class EColorConverter {  
  Raw, // 原始数据  
  Depth, // 深度图  
  LogarithmicDepth, // 对数深度图  
  CityScapesPalette // CityScapes调色板  
};  
  
// 模板函数GetRawDataAsBuffer，用于将T类型的数据转换为Python的内存视图对象  
template <typename T>  
static auto GetRawDataAsBuffer(T &self) {  
    // 将self的数据部分转换为unsigned char*类型  
    auto *data = reinterpret_cast<unsigned char *>(self.data());  
    // 计算数据的大小  
    auto size = static_cast<Py_ssize_t>(sizeof(typename T::value_type) * self.size());  
    // 根据Python版本创建内存视图对象  
#if PY_MAJOR_VERSION >= 3  
    auto *ptr = PyMemoryView_FromMemory(reinterpret_cast<char *>(data), size, PyBUF_READ);  
#else  
    auto *ptr = PyBuffer_FromMemory(data, size);  
#endif  
    // 返回boost::python::object对象，封装了内存视图  
    return boost::python::object(boost::python::handle<>(ptr));  
}  
  
// 模板函数ConvertImage，用于根据指定的颜色转换器类型转换图像数据  
template <typename T>  
static void ConvertImage(T &self, EColorConverter cc) {  
    // 释放全局解释器锁，以便在C++代码中执行耗时操作时不会阻塞Python线程  
    carla::PythonUtil::ReleaseGIL unlock;  
    // 使用carla::image命名空间  
    using namespace carla::image;  
    // 创建ImageView对象，用于操作图像数据  
    auto view = ImageView::MakeView(self);  
    // 根据颜色转换器类型执行相应的转换  
    switch (cc) {  
        case EColorConverter::Depth:  
            ImageConverter::ConvertInPlace(view, ColorConverter::Depth());  
            break;  
        case EColorConverter::LogarithmicDepth:  
            ImageConverter::ConvertInPlace(view, ColorConverter::LogarithmicDepth());  
            break;  
        case EColorConverter::CityScapesPalette:  
            ImageConverter::ConvertInPlace(view, ColorConverter::CityScapesPalette());  
            break;  
        case EColorConverter::Raw:  
            break; // 忽略原始数据，不进行转换  
        default:  
            throw std::invalid_argument("invalid color converter!"); // 抛出异常，表示无效的颜色转换器  
    }  
}  
  
// FakeImage类，继承自std::vector<uint8_t>，用于表示从光学流转换为颜色后的图像  
class FakeImage : public std::vector<uint8_t> {  
  public:  
    unsigned int Width = 0; // 图像宽度  
    unsigned int Height = 0; // 图像高度  
    float FOV = 0; // 视野角度  
};  
  
// ColorCodedFlow函数，用于将光学流图像转换为RGB图像  
static FakeImage ColorCodedFlow(  
    carla::sensor::data::OpticalFlowImage& image) {  
    // 使用命名空间别名简化代码  
    namespace bp = boost::python;  
    namespace csd = carla::sensor::data;  
    // 定义常量π和弧度转角度的系数  
    constexpr float pi = 3.1415f;  
    constexpr float rad2ang = 360.f/(2.f*pi);  
    // 创建FakeImage对象，用于存储转换后的图像数据  
    FakeImage result;  
    // 设置图像的宽度、高度和视野角度  
    result.Width = image.GetWidth();  
    result.Height = image.GetHeight();  
    result.FOV = image.GetFOVAngle();  
    // 调整result的大小，以适应RGB图像的数据量（每个像素4个字节）  
    result.resize(image.GetHeight()*image.GetWidth()* 4);

  // Lambda函数，用于批量处理像素（计算光流数据并映射为颜色）
  auto command = [&] (size_t min_index, size_t max_index) {
    // 遍历图像像素的索引，从 min_index 到 max_index
    for (size_t index = min_index; index < max_index; index++) {
      const csd::OpticalFlowPixel& pixel = image[index];
      // 提取光流的 x 和 y 分量，表示在 x 和 y 方向上的流动
      float vx = pixel.x;
      float vy = pixel.y;

       // 计算光流向量的角度，角度范围为 [0, 360] 度
      float angle = 180.f + std::atan2(vy, vx)*rad2ang;
      if (angle < 0) angle = 360.f + angle;
      angle = std::fmod(angle, 360.f);

      // 计算光流向量的模（即流动的大小）
      float norm = std::sqrt(vx*vx + vy*vy);
      const float shift = 0.999f;
      const float a = 1.f/std::log(0.1f + shift);
      float intensity = carla::geom::Math::Clamp(a*std::log(norm + shift), 0.f, 1.f);

      // 将角度（Hue）赋值给 H，饱和度（S）设为 1，亮度（V）使用计算得到的强度值
      float& H = angle;
      float  S = 1.f;
      float V = intensity;
      float H_60 = H*(1.f/60.f);

      // 计算 HSV 转换到 RGB 的中间值
      float C = V * S;
      float X = C*(1.f - std::abs(std::fmod(H_60, 2.f) - 1.f));
      float m = V - C;

      // 初始化 RGB 分量
      float r = 0,g = 0,b = 0;
      // 根据 H 值的范围确定 RGB 分量的值
      unsigned int angle_case = static_cast<unsigned int>(H_60);
      switch (angle_case) {
      case 0:
        r = C;
        g = X;
        b = 0;
        break;
      case 1:
        r = X;
        g = C;
        b = 0;
        break;
      case 2:
        r = 0;
        g = C;
        b = X;
        break;
      case 3:
        r = 0;
        g = X;
        b = C;
        break;
      case 4:
        r = X;
        g = 0;
        b = C;
        break;
      case 5:
        r = C;
        g = 0;
        b = X;
        break;
      default:
        r = 1;
        g = 1;
        b = 1;
        break;
      }

      // 将 RGB 分量调整并映射到 [0, 255] 范围，并保存到结果数组中（假设使用 RGBA 格式，A 为 0）
      uint8_t R = static_cast<uint8_t>((r+m)*255.f);
      uint8_t G = static_cast<uint8_t>((g+m)*255.f);
      uint8_t B = static_cast<uint8_t>((b+m)*255.f);
      // 将计算得到的 RGB 值存储到结果数组中（以 RGBA 格式存储）
      result[4*index] = B;
      result[4*index + 1] = G;
      result[4*index + 2] = R;
      result[4*index + 3] = 0;
    }
  };
  // 计算可用线程数，至少为 8 个线程，或者根据硬件的核心数决定
  size_t num_threads = std::max(8u, std::thread::hardware_concurrency());
  // 计算每个线程处理的像素数量（将图像大小均分给多个线程）
  size_t batch_size = image.size() / num_threads;
  // 创建一个线程指针数组，用于存储线程对象（多一个线程是为了保存线程集合的大小）
  std::vector<std::thread*> t(num_threads+1);

  // 创建 num_threads 个线程，每个线程负责处理不同的图像数据批次
  for(size_t n = 0; n < num_threads; n++) {
    // 为每个线程分配任务，处理的范围是从 n * batch_size 到 (n+1) * batch_size
    t[n] = new std::thread(command, n * batch_size, (n+1) * batch_size);
  }
  t[num_threads] = new std::thread(command, num_threads * batch_size, image.size());

  for(size_t n = 0; n <= num_threads; n++) {
    if(t[n]->joinable()){
      t[n]->join();
    }
    delete t[n];
  }
  return result;
}
// 定义一个保存图像到磁盘的模板函数
template <typename T>
static std::string SaveImageToDisk(T &self, std::string path, EColorConverter cc) {
  // 释放 Python GIL（全局解释器锁），以便在 C++ 中执行多线程操作
  carla::PythonUtil::ReleaseGIL unlock;
  using namespace carla::image;
  // 将图像数据转换为图像视图
  auto view = ImageView::MakeView(self);
  // 根据不同的颜色转换类型进行不同的处理
  switch (cc) {
    case EColorConverter::Raw:
      return ImageIO::WriteView(
          std::move(path),
          view);
    case EColorConverter::Depth:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::Depth()));
    case EColorConverter::LogarithmicDepth:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::LogarithmicDepth()));
    case EColorConverter::CityScapesPalette:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::CityScapesPalette()));
    default:
      throw std::invalid_argument("invalid color converter!");
  }
}

template <typename T>
static std::string SavePointCloudToDisk(T &self, std::string path) {
  carla::PythonUtil::ReleaseGIL unlock;
  return carla::pointcloud::PointCloudIO::SaveToDisk(std::move(path), self.begin(), self.end());
}

static boost::python::dict GetCAMData(const carla::sensor::data::CAMData message)
{
    boost::python::dict myDict;
    try
    {
        myDict["Power"] = message.Power;
        myDict["Message"] = GetCAMMessage(message);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return myDict;
}


static boost::python::dict GetCustomV2XData(const carla::sensor::data::CustomV2XData message)
{
    boost::python::dict myDict;
    try
    {
        myDict["Power"] = message.Power;
        myDict["Message"] = GetCustomV2XMessage(message);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return myDict;
}
/**********************************************************************************************/
void export_sensor_data() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::rpc;
  namespace cs = carla::sensor;
  namespace csd = carla::sensor::data;
  namespace css = carla::sensor::s11n;

  // Fake image returned from optical flow to color conversion
  // fakes the regular image object. Only used for visual purposes
  class_<FakeImage>("FakeImage", no_init)
      .def(vector_indexing_suite<std::vector<uint8_t>>())
      .add_property("width", &FakeImage::Width)
      .add_property("height", &FakeImage::Height)
      .add_property("fov", &FakeImage::FOV)
      .add_property("raw_data", &GetRawDataAsBuffer<FakeImage>);

  class_<cs::SensorData, boost::noncopyable, boost::shared_ptr<cs::SensorData>>("SensorData", no_init)
    .add_property("frame", &cs::SensorData::GetFrame)
    .add_property("frame_number", &cs::SensorData::GetFrame) // deprecated.
    .add_property("timestamp", &cs::SensorData::GetTimestamp)
    .add_property("transform", CALL_RETURNING_COPY(cs::SensorData, GetSensorTransform))
  ;

  enum_<EColorConverter>("ColorConverter")
    .value("Raw", EColorConverter::Raw)
    .value("Depth", EColorConverter::Depth)
    .value("LogarithmicDepth", EColorConverter::LogarithmicDepth)
    .value("CityScapesPalette", EColorConverter::CityScapesPalette)
  ;

  // The values here should match the ones in the enum EGBufferTextureID,
  // from the CARLA fork of Unreal Engine (Renderer/Public/GBufferView.h).
  enum_<int>("GBufferTextureID")
    .value("SceneColor", 0)
    .value("SceneDepth", 1)
    .value("SceneStencil", 2)
    .value("GBufferA", 3)
    .value("GBufferB", 4)
    .value("GBufferC", 5)
    .value("GBufferD", 6)
    .value("GBufferE", 7)
    .value("GBufferF", 8)
    .value("Velocity", 9)
    .value("SSAO", 10)
    .value("CustomDepth", 11)
    .value("CustomStencil", 12)
  ;

  class_<csd::Image, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::Image>>("Image", no_init)
    .add_property("width", &csd::Image::GetWidth)
    .add_property("height", &csd::Image::GetHeight)
    .add_property("fov", &csd::Image::GetFOVAngle)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::Image>)
    .def("convert", &ConvertImage<csd::Image>, (arg("color_converter")))
    .def("save_to_disk", &SaveImageToDisk<csd::Image>, (arg("path"), arg("color_converter")=EColorConverter::Raw))
    .def("__len__", &csd::Image::size)
    .def("__iter__", iterator<csd::Image>())
    .def("__getitem__", +[](const csd::Image &self, size_t pos) -> csd::Color {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::Image &self, size_t pos, csd::Color color) {
      self.at(pos) = color;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::OpticalFlowImage, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::OpticalFlowImage>>("OpticalFlowImage", no_init)
    .add_property("width", &csd::OpticalFlowImage::GetWidth)
    .add_property("height", &csd::OpticalFlowImage::GetHeight)
    .add_property("fov", &csd::OpticalFlowImage::GetFOVAngle)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::OpticalFlowImage>)
    .def("get_color_coded_flow", &ColorCodedFlow)
    .def("__len__", &csd::OpticalFlowImage::size)
    .def("__iter__", iterator<csd::OpticalFlowImage>())
    .def("__getitem__", +[](const csd::OpticalFlowImage &self, size_t pos) -> csd::OpticalFlowPixel {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::OpticalFlowImage &self, size_t pos, csd::OpticalFlowPixel color) {
      self.at(pos) = color;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LidarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::LidarMeasurement>>("LidarMeasurement", no_init)
    .add_property("horizontal_angle", &csd::LidarMeasurement::GetHorizontalAngle)
    .add_property("channels", &csd::LidarMeasurement::GetChannelCount)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::LidarMeasurement>)
    .def("get_point_count", &csd::LidarMeasurement::GetPointCount, (arg("channel")))
    .def("save_to_disk", &SavePointCloudToDisk<csd::LidarMeasurement>, (arg("path")))
    .def("__len__", &csd::LidarMeasurement::size)
    .def("__iter__", iterator<csd::LidarMeasurement>())
    .def("__getitem__", +[](const csd::LidarMeasurement &self, size_t pos) -> csd::LidarDetection {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::LidarMeasurement &self, size_t pos, const csd::LidarDetection &detection) {
      self.at(pos) = detection;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::SemanticLidarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::SemanticLidarMeasurement>>("SemanticLidarMeasurement", no_init)
    .add_property("horizontal_angle", &csd::SemanticLidarMeasurement::GetHorizontalAngle)
    .add_property("channels", &csd::SemanticLidarMeasurement::GetChannelCount)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::SemanticLidarMeasurement>)
    .def("get_point_count", &csd::SemanticLidarMeasurement::GetPointCount, (arg("channel")))
    .def("save_to_disk", &SavePointCloudToDisk<csd::SemanticLidarMeasurement>, (arg("path")))
    .def("__len__", &csd::SemanticLidarMeasurement::size)
    .def("__iter__", iterator<csd::SemanticLidarMeasurement>())
    .def("__getitem__", +[](const csd::SemanticLidarMeasurement &self, size_t pos) -> csd::SemanticLidarDetection {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::SemanticLidarMeasurement &self, size_t pos, const csd::SemanticLidarDetection &detection) {
      self.at(pos) = detection;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::CollisionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::CollisionEvent>>("CollisionEvent", no_init)
    .add_property("actor", &csd::CollisionEvent::GetActor)
    .add_property("other_actor", &csd::CollisionEvent::GetOtherActor)
    .add_property("normal_impulse", CALL_RETURNING_COPY(csd::CollisionEvent, GetNormalImpulse))
    .def(self_ns::str(self_ns::self))
  ;

    class_<csd::ObstacleDetectionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::ObstacleDetectionEvent>>("ObstacleDetectionEvent", no_init)
    .add_property("actor", &csd::ObstacleDetectionEvent::GetActor)
    .add_property("other_actor", &csd::ObstacleDetectionEvent::GetOtherActor)
    .add_property("distance", CALL_RETURNING_COPY(csd::ObstacleDetectionEvent, GetDistance))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LaneInvasionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::LaneInvasionEvent>>("LaneInvasionEvent", no_init)
    .add_property("actor", &csd::LaneInvasionEvent::GetActor)
    .add_property("crossed_lane_markings", CALL_RETURNING_LIST(csd::LaneInvasionEvent, GetCrossedLaneMarkings))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::GnssMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::GnssMeasurement>>("GnssMeasurement", no_init)
    .add_property("latitude", &csd::GnssMeasurement::GetLatitude)
    .add_property("longitude", &csd::GnssMeasurement::GetLongitude)
    .add_property("altitude", &csd::GnssMeasurement::GetAltitude)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::IMUMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::IMUMeasurement>>("IMUMeasurement", no_init)
    .add_property("accelerometer", &csd::IMUMeasurement::GetAccelerometer)
    .add_property("gyroscope", &csd::IMUMeasurement::GetGyroscope)
    .add_property("compass", &csd::IMUMeasurement::GetCompass)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::RadarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::RadarMeasurement>>("RadarMeasurement", no_init)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::RadarMeasurement>)
    .def("get_detection_count", &csd::RadarMeasurement::GetDetectionAmount)
    .def("__len__", &csd::RadarMeasurement::size)
    .def("__iter__", iterator<csd::RadarMeasurement>())
    .def("__getitem__", +[](const csd::RadarMeasurement &self, size_t pos) -> csd::RadarDetection {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::RadarMeasurement &self, size_t pos, const csd::RadarDetection &detection) {
      self.at(pos) = detection;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::RadarDetection>("RadarDetection")
    .def_readwrite("velocity", &csd::RadarDetection::velocity)
    .def_readwrite("azimuth", &csd::RadarDetection::azimuth)
    .def_readwrite("altitude", &csd::RadarDetection::altitude)
    .def_readwrite("depth", &csd::RadarDetection::depth)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LidarDetection>("LidarDetection")
    .def_readwrite("point", &csd::LidarDetection::point)
    .def_readwrite("intensity", &csd::LidarDetection::intensity)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::SemanticLidarDetection>("SemanticLidarDetection")
    .def_readwrite("point", &csd::SemanticLidarDetection::point)
    .def_readwrite("cos_inc_angle", &csd::SemanticLidarDetection::cos_inc_angle)
    .def_readwrite("object_idx", &csd::SemanticLidarDetection::object_idx)
    .def_readwrite("object_tag", &csd::SemanticLidarDetection::object_tag)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::DVSEvent>("DVSEvent")
    .add_property("x", &csd::DVSEvent::x)
    .add_property("y", &csd::DVSEvent::y)
    .add_property("t", &csd::DVSEvent::t)
    .add_property("pol", &csd::DVSEvent::pol)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::DVSEventArray, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::DVSEventArray>>("DVSEventArray", no_init)
    .add_property("width", &csd::DVSEventArray::GetWidth)
    .add_property("height", &csd::DVSEventArray::GetHeight)
    .add_property("fov", &csd::DVSEventArray::GetFOVAngle)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::DVSEventArray>)
    .def("__len__", &csd::DVSEventArray::size)
    .def("__iter__", iterator<csd::DVSEventArray>())
    .def("__getitem__", +[](const csd::DVSEventArray &self, size_t pos) -> csd::DVSEvent {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::DVSEventArray &self, size_t pos, csd::DVSEvent event) {
      self.at(pos) = event;
    })
    .def("to_image", CALL_RETURNING_LIST(csd::DVSEventArray, ToImage))
    .def("to_array", CALL_RETURNING_LIST(csd::DVSEventArray, ToArray))
    .def("to_array_x", CALL_RETURNING_LIST(csd::DVSEventArray, ToArrayX))
    .def("to_array_y", CALL_RETURNING_LIST(csd::DVSEventArray, ToArrayY))
    .def("to_array_t", CALL_RETURNING_LIST(csd::DVSEventArray, ToArrayT))
    .def("to_array_pol", CALL_RETURNING_LIST(csd::DVSEventArray, ToArrayPol))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::CAMData>("CAMMessage")
    .def_readwrite("power", &csd::CAMData::Power)
    .def("get", GetCAMData)
    .def(self_ns::str(self_ns::self))
  ;

    class_<csd::CustomV2XData>("CustomV2XData")
    .def_readwrite("power", &csd::CustomV2XData::Power)
    .def("get", GetCustomV2XData)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::CAMEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::CAMEvent>>("CAMEvent", no_init)
    .def("get_message_count", &csd::CAMEvent::GetMessageCount)
    .def("__len__", &csd::CAMEvent::size)
    .def("__iter__", iterator<csd::CAMEvent>())
    .def("__getitem__", +[](const csd::CAMEvent &self, size_t pos) -> csd::CAMData {
      return self.at(pos);
    })
  ;

    class_<csd::CustomV2XEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::CustomV2XEvent>>("CustomV2XEvent", no_init)
    .def("get_message_count", &csd::CustomV2XEvent::GetMessageCount)
    .def("__len__", &csd::CustomV2XEvent::size)
    .def("__iter__", iterator<csd::CustomV2XEvent>())
    .def("__getitem__", +[](const csd::CustomV2XEvent &self, size_t pos) -> csd::CustomV2XData {
      return self.at(pos);
    })
  ;
}
