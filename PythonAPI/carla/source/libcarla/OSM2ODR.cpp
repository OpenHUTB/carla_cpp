// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 引入OSM2ODR.h头文件，该文件位于SUMO源码中的Build/osm2odr-source/src/目录下，
// 推测这里面包含了与将OpenStreetMap地图转换为OpenDRIVE格式相关的一些类型、函数等声明内容
#include <OSM2ODR.h>  

// 定义一个空类OSM2ODR，这里可能是用于模拟PythonAPI中的命名空间概念，
// 虽然类体为空，但通过它可以在C++代码里营造出类似Python中命名空间的组织结构，方便后续代码对相关功能进行分组管理
class OSM2ODR {};

namespace osm2odr {
    // 定义一个重载的输出流操作符<<，用于将OSM2ODRSettings类型的对象以特定格式输出到流中。
    // 这样可以方便地在控制台或者其他支持输出流的地方直观地查看OSM2ODRSettings对象包含的各种设置参数信息。
    std::ostream &operator<<(std::ostream &out, const OSM2ODRSettings &settings) {
        // 按照指定格式将OSM2ODRSettings对象中的各个属性依次输出到输出流out中，
        // 例如输出use_offsets属性时，根据其布尔值输出对应的"true"或"false"字符串表示
        out << "Osm2odrSettings(use_offsets=" << (settings.use_offsets? "true" : "false")
            << ", offset_x=" << settings.offset_x << ", offset_y=" << settings.offset_y
            << ", default_lane_width=" << settings.default_lane_width
            << ", elevation_layer_height=" << settings.elevation_layer_height << ")";
        return out;
    }

    // 定义一个名为SetOsmWayTypes的函数，用于设置OSM2ODRSettings对象中的osm_highways_types成员变量。
    // 它接收一个OSM2ODRSettings对象的引用self以及一个boost::python::list类型的input参数，
    // 函数内部会将Python列表类型的input转换为C++的std::vector<std::string>类型，并赋值给self.osm_highways_types成员。
    void SetOsmWayTypes(OSM2ODRSettings& self, boost::python::list input) {
        self.osm_highways_types = PythonLitstToVector<std::string>(input);
    }

    // 与SetOsmWayTypes函数类似，定义SetTLExcludedWayTypes函数，用于设置OSM2ODRSettings对象中的tl_excluded_highways_types成员变量。
    // 同样接收OSM2ODRSettings对象引用和Python列表类型的参数，将输入的Python列表转换为C++的字符串向量后赋值给对应的成员变量。
    void SetTLExcludedWayTypes(OSM2ODRSettings& self, boost::python::list input) {
        self.tl_excluded_highways_types = PythonLitstToVector<std::string>(input);
    }
}

// 定义一个名为export_osm2odr的函数，从函数名推测其功能可能是将与osm2odr相关的功能、类型等导出，
// 以便在其他模块（可能是Python绑定相关的模块）中使用，常用于将C++代码与Python进行交互的场景中。
void export_osm2odr() {
    using namespace osm2odr;
    using namespace boost::python;

    // 使用boost::python库的class_模板来定义一个Python可访问的类，名为"Osm2OdrSettings"，
    // 对应C++中的OSM2ODRSettings类型，通过init<>()指定其默认构造函数用于初始化对象。
    class_<OSM2ODRSettings>("Osm2OdrSettings", init<>())
        // 为Python中的类添加属性"use_offsets"，并关联到C++中OSM2ODRSettings类的use_offsets成员变量，
        // 通过这种方式，Python代码可以直接访问和修改该属性，并且读写操作都绑定到了对应的C++成员变量上。
       .add_property("use_offsets", &OSM2ODRSettings::use_offsets, &OSM2ODRSettings::use_offsets)
       .add_property("offset_x", &OSM2ODRSettings::offset_x, &OSM2ODRSettings::offset_x)
       .add_property("offset_y", &OSM2ODRSettings::offset_y, &OSM2ODRSettings::offset_y)
       .add_property("default_lane_width", &OSM2ODRSettings::default_lane_width, &OSM2ODRSettings::default_lane_width)
       .add_property("elevation_layer_height", &OSM2ODRSettings::elevation_layer_height, &OSM2ODRSettings::elevation_layer_height)
       .add_property("proj_string", &OSM2ODRSettings::proj_string, &OSM2ODRSettings::proj_string)
       .add_property("center_map", &OSM2ODRSettings::center_map, &OSM2ODRSettings::center_map)
       .add_property("generate_traffic_lights", &OSM2ODRSettings::generate_traffic_lights, &OSM2ODRSettings::generate_traffic_lights)
       .add_property("all_junctions_with_traffic_lights", &OSM2ODRSettings::all_junctions_traffic_lights, &OSM2ODRSettings::all_junctions_traffic_lights)
        // 为Python类添加名为"set_osm_way_types"的方法，关联到C++中的SetOsmWayTypes函数，
        // 使得在Python中可以调用该方法来设置对应的成员变量，参数"way_types"用于传递Python列表类型的数据。
       .def("set_osm_way_types", &SetOsmWayTypes, arg("way_types"))
        // 类似地，添加"set_traffic_light_excluded_way_types"方法，关联到SetTLExcludedWayTypes函数，
        // 用于在Python中操作对应C++对象的另一个成员变量相关设置。
       .def("set_traffic_light_excluded_way_types", &SetTLExcludedWayTypes, arg("way_types"))
        // 为Python类定义一个特殊的方法，用于将对象转换为字符串表示形式，这里调用了之前定义的重载输出流操作符<<来实现该功能，
        // 使得在Python中使用str()函数对该类对象进行操作时能得到合适的字符串描述。
       .def(self_ns::str(self_ns::self))
    ;

    // 再次使用boost::python库的class_模板定义一个Python可访问的类，名为"Osm2Odr"，对应C++中的OSM2ODR类型（虽然OSM2ODR类体为空，但可作为一种组织功能的标识）。
    // no_init表示该类在Python中不使用默认构造函数进行初始化，可能需要通过其他静态方法等方式来创建实例或者使用相关功能。
    class_<OSM2ODR>("Osm2Odr", no_init)
        // 为Python类添加名为"convert"的静态方法，关联到C++中的ConvertOSMToOpenDRIVE函数，
        // 用于在Python中调用该方法实现将OpenStreetMap文件转换为OpenDRIVE格式的功能，
        // 参数"osm_file"表示要转换的OpenStreetMap文件路径，"settings"参数有默认值，默认使用OSM2ODRSettings类型的默认构造对象。
       .def("convert", &ConvertOSMToOpenDRIVE, (arg("osm_file"), arg("settings") = OSM2ODRSettings()))
        // 明确指定"convert"方法为静态方法，符合在Python中调用该方法时不需要先实例化类对象的预期行为。
       .staticmethod("convert")
    ;
}
