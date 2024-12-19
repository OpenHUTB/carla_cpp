#pragma once
#include <stdint.h>
#include <vector>
#include <cstring>
// ITSContainer类可能用于处理与智能交通系统（ITS）相关的数据结构和信息表示。
// 以下在类中定义的一系列枚举类型和类型别名，主要是为了规范和清晰地表示地理位置、方向、精度等相关信息的数据类型，
// 方便在整个程序中统一使用这些特定含义的数据表示方式，避免混乱和歧义。
class ITSContainer
{
    
public:

    typedef bool OptionalValueAvailable_t;


    /* Latitude Dependencies*/
// Latitude枚举类型用于定义表示纬度相关信息的特定取值，
// 这些取值具有明确的物理意义，对应不同精度或特殊状态下的纬度表示。   
    typedef enum Latitude
    {
// 表示北纬 1 微度，该值用于精确表示在地理坐标系中向北的微小纬度变化，单位为微度（百万分之一度）。
        Latitude_oneMicroDegreeNorth = 10,
// 表示南纬 1 微度，对应向南的微小纬度变化，单位同样为微度。
        Latitude_oneMicroDegreeSouth = -10,
// 表示纬度信息不可用的特定值，当无法获取准确纬度数据时，使用该值来标记
        Latitude_unavailable = 900000001
    } e_Latitude;

    /* Latitude */
// Latitude_t类型别名定义为long类型，用于在程序中统一表示纬度数据，
// 可以存储具体的纬度数值（如上述枚举中的微度表示值）或者表示不可用状态的特殊值。
    typedef long Latitude_t;

    /* Longitude Dependencies */
// Longitude枚举类型用于规定表示经度相关信息的特定取值，
// 每个取值对应不同精度或特殊情况的经度表示，与纬度类似，旨在清晰定义经度相关的不同状态。
    typedef enum Longitude
    {
// 表示东经 1 微度，用于体现地理坐标系中向东的微小经度变化，单位是微度（百万分之一度）。
        Longitude_oneMicroDegreeEast = 10,
 // 表示西经 1 微度，对应向西的微小经度变化，单位为微度。
        Longitude_oneMicroDegreeWest = -10,
 // 用于标记经度信息不可用的特殊值，当无法准确获取经度数据时，以此值表示该情况。
        Longitude_unavailable = 1800000001
    } e_Longitude;

    /* Longitude */
 // Longitude_t类型别名设定为long类型，用来在整个程序中统一承载经度数据，
// 可以存储实际的经度数值（如微度表示的具体值）或者不可用状态对应的特殊值。
    typedef long Longitude_t;

    /* SemiAxisLength Dependencies */
// SemiAxisLength枚举类型是为了明确表示半轴长度相关的特定取值，
// 可能用于描述如地球半轴等相关长度在特定情境下的不同表示，涵盖正常范围、超出范围以及不可用等情况。
    typedef enum SemiAxisLength
    { 
// 表示半轴长度为 1 厘米，是一种特定的长度度量单位下的标准表示值，用于符合该单位的半轴长度记录。
        SemiAxisLength_oneCentimeter = 1,
 // 表示半轴长度超出了正常的有效范围，当检测到半轴长度处于不合理的区间时，使用该值来标识。
        SemiAxisLength_outOfRange = 4094,
 // 当半轴长度信息无法获取时，用此值来表示该半轴长度处于不可用状态。
        SemiAxisLength_unavailable = 4095
    } e_SemiAxisLength;

    /* SemiAxisLength*/
// SemiAxisLength_t类型别名定义为long类型，以便在程序中统一处理半轴长度数据，
// 可以存放具体的合理长度值、超出范围的标识值或者不可用的特殊值。
    typedef long SemiAxisLength_t;

    /* HeadingValue Dependencies */
 // HeadingValue枚举类型用于定义航向值相关的特定取值，
// 这些取值基于 WGS84 坐标系（世界大地坐标系）下的基本方向角度，以特定的整数表示不同方向，方便在程序中统一引用。
    typedef enum HeadingValue
    {
// 表示航向为 WGS84 坐标系下的正北方向，对应角度值为 0 度，以特定的整数表示便于在代码中进行方向判断等操作。
        HeadingValue_wgs84North = 0,
// 代表航向为 WGS84 坐标系下的正东方向，对应角度值为 900，这里的数值可能是经过特定换算或按程序要求的表示形式。
        HeadingValue_wgs84East = 900,
// 表示航向为 WGS84 坐标系下的正南方向，对应角度值为 1800，用于明确正南方向的特定数值表示。
        HeadingValue_wgs84South = 1800,
 // 意味着航向为 WGS84 坐标系下的正西方向，对应角度值为 2700，方便在涉及方向的计算和判断中使用。
        HeadingValue_wgs84West = 2700,
 // 用于标记航向值不可用的情况，当无法获取准确的航向信息时，通过该值来体现这一状态。
        HeadingValue_unavailable = 3601
    } e_HeadingValue;

    /* HeadingValue */
 // HeadingValue_t类型别名定义为long类型，用于在程序中统一存储和操作航向值数据，
// 可以是具体的方向角度对应的表示值或者不可用状态对应的特殊值。
    typedef long HeadingValue_t;

     /* HeadingConfidence Dependencies */
// HeadingConfidence枚举类型旨在规定航向置信度相关的特定取值，
// 反映了对航向信息准确性的信任程度，涵盖了在不同精度范围内以及特殊情况（如超出范围、不可用）下的表示值。
    typedef enum HeadingConfidence {
 // 表示航向置信度为等于或在 0.1 度范围内，意味着对当前航向值的准确性有较高的信心，偏差较小。
        HeadingConfidence_equalOrWithinZeroPointOneDegree   = 1,
 // 意味着航向置信度为等于或在 1 度范围内，说明对航向值的准确性把握在相对稍大一点的偏差区间内。
        HeadingConfidence_equalOrWithinOneDegree    = 10,
 // 用于标记航向置信度超出了正常有效的取值范围，当出现不符合常规置信度定义的情况时，以此值标识。
        HeadingConfidence_outOfRange    = 126,
 // 当无法确定航向置信度，即该信息不可用时，使用此值来表示这一状态。
        HeadingConfidence_unavailable   = 127
    } e_HeadingConfidence;
 
    /* HeadingConfidence */
// HeadingConfidence_t类型别名定义为long类型，在程序中统一处理航向置信度数据，
// 可以存储具体的置信度表示值或者不可用等特殊情况对应的数值。
    typedef long HeadingConfidence_t;

    /* PosConfidenceEllipse*/
// PosConfidenceEllipse结构体用于表示位置置信椭圆相关的信息。
// 它包含了描述椭圆特征的几个关键元素，这些元素共同刻画了在一定置信水平下物体位置的不确定性范围及方向等信息。
    typedef struct PosConfidenceEllipse
    { 
// semiMajorConfidence表示置信椭圆的长半轴长度的置信相关信息，
// 其类型为SemiAxisLength_t，该类型在之前已定义，能够表示半轴长度的具体数值、超出范围情况或不可用状态等。
        SemiAxisLength_t semiMajorConfidence;
// semiMinorConfidence代表置信椭圆的短半轴长度的置信相关信息，同样使用SemiAxisLength_t类型，
// 用于体现短半轴长度方面的相关状态与数值。
        SemiAxisLength_t semiMinorConfidence;
// semiMajorOrientation用于指明置信椭圆长半轴的方向，以HeadingValue_t类型存储，
// 该类型能够表示如基于WGS84坐标系下的方向角度相关的特定值或者不可用状态等情况，以此确定长半轴在空间中的方位。
        HeadingValue_t semiMajorOrientation;
    } PosConfidenceEllipse_t;
    
    /* AltitudeValue Dependencies */
// AltitudeValue枚举类型主要用于定义高度相关的特定取值情况，
// 这些取值对应了不同的高度度量含义，比如相对于参考椭球面的高度、以厘米为单位的具体高度以及高度不可用的标识等，
// 在涉及到描述物体垂直位置信息时会用到这些不同的表示值。
    typedef enum AltitudeValue
    { 
// 表示物体的高度是相对于参考椭球面的高度，其值设为0，用于明确这种特定的高度参考基准情况。
        AltitudeValue_referenceEllipsoidSurface = 0,
 // 意味着物体的高度为1厘米，是一种以厘米为单位衡量高度的具体表示值，用于表示相对较小的垂直距离情况。
        AltitudeValue_oneCentimeter = 1,
 // 用于标记高度信息不可用的特殊值，当无法获取准确的高度数据时，通过该值来体现这种情况。
        AltitudeValue_unavailable = 800001
    } e_AltitudeValue;

    /* AltitudeValue */
// AltitudeValue_t类型别名定义为long类型，用于在程序中统一存储和操作高度值数据，
// 它可以存放AltitudeValue枚举中定义的各种高度相关的具体数值或者不可用状态对应的特殊值，方便在不同模块间以统一类型处理高度信息。
    typedef long AltitudeValue_t;

    /* AltitudeConfidence Dependencies */
    typedef enum AltitudeConfidence
    {
// 表示高度置信度对应的高度误差在 ±0.00001（单位需结合实际场景确定，可能是米等长度单位）范围内，意味着对高度值的准确性有很高的信心，误差极小。
        AltitudeConfidence_alt_000_01   = 0,
 // 意味着高度置信度对应的高度误差在 ±0.00002范围内，说明对高度值准确性的把握在稍大一点的误差区间内。
        AltitudeConfidence_alt_000_02   = 1,
 // 表示高度置信度对应的高度误差在 ±0.00005范围内，以此类推，不同取值对应不同的误差范围，体现不同的置信水平。
        AltitudeConfidence_alt_000_05   = 2,
        AltitudeConfidence_alt_000_10   = 3,
        AltitudeConfidence_alt_000_20   = 4,
        AltitudeConfidence_alt_000_50   = 5,
        AltitudeConfidence_alt_001_00   = 6,
        AltitudeConfidence_alt_002_00   = 7,
        AltitudeConfidence_alt_005_00   = 8,
        AltitudeConfidence_alt_010_00   = 9,
        AltitudeConfidence_alt_020_00   = 10,
        AltitudeConfidence_alt_050_00   = 11,
        AltitudeConfidence_alt_100_00   = 12,
        AltitudeConfidence_alt_200_00   = 13,
 // 用于标记高度置信度超出了正常有效的取值范围，当出现不符合常规置信度定义的情况时，以此值标识。
        AltitudeConfidence_outOfRange   = 14,
 // 当无法确定高度置信度，即该信息不可用时，使用此值来表示这一状态。
        AltitudeConfidence_unavailable  = 15
    }e_AltitudeConfidence;

    /* AltitudeConfidence */
// AltitudeConfidence_t类型别名定义为long类型，在程序中统一处理高度置信度数据，
// 可以存储AltitudeConfidence枚举中定义的具体置信度表示值或者不可用等特殊情况对应的数值，便于在整个代码中规范使用。
    typedef long AltitudeConfidence_t;

    /* Altitude */
// Altitude结构体用于综合表示高度相关的信息，将高度值本身以及对应的高度置信度结合在一起，
// 这样可以更全面地描述物体在垂直方向上的位置及其准确性情况。
    typedef struct Altitude
    {
// altitudeValue用于存储具体的高度数值，其类型为AltitudeValue_t，可包含如相对于参考面的高度、具体厘米数高度或不可用值等情况。
        AltitudeValue_t altitudeValue;
// altitudeConfidence存放对应的高度置信度信息，类型是AltitudeConfidence_t，反映了对该高度值准确性的信任程度。
        AltitudeConfidence_t altitudeConfidence;
    }Altitude_t;

    /* ReferencePosition */
// ReferencePosition结构体用于表示参考位置相关的综合信息，
// 它整合了水平位置（纬度、经度以及位置置信椭圆信息）和垂直位置（高度相关信息），
// 以此完整地描述一个物体在三维空间中的位置及其相关的不确定性和置信情况。
    typedef struct ReferencePosition
    {
 // latitude用于存储物体所在位置的纬度信息，其类型为Latitude_t，可表示具体的纬度数值或不可用状态等情况，之前已对该类型进行了定义。
        Latitude_t latitude;
// longitude用于存放物体所在位置的经度信息，类型是Longitude_t，同样能表示具体经度数值或不可用等情况，按照之前的定义使用。
        Longitude_t longitude;
 // positionConfidenceEllipse包含了位置置信椭圆的相关信息，用于刻画在水平面上物体位置的不确定性范围及长半轴方向等，
 // 是描述位置精度的一个重要组成部分，类型为之前定义的PosConfidenceEllipse_t结构体类型。
        PosConfidenceEllipse_t positionConfidenceEllipse;
 // altitude结构体变量用于承载物体的高度相关信息，包括具体高度值以及对应的高度置信度，全面描述了垂直方向上的位置情况，
// 类型为Altitude_t，是综合表示高度信息的结构体类型。
        Altitude_t altitude;
    } ReferencePosition_t;

    /* StationType Dependencies */
// StationType枚举类型用于定义各种交通站点（或交通参与者类型）的分类标识，
// 在智能交通系统相关的应用场景中，通过这些不同的枚举值来区分不同类型的交通实体，方便后续进行针对性的处理与分析。
    typedef enum StationType
    {
// 表示未知类型的交通站点或交通参与者，当无法明确具体类型时，使用该值作为默认或初始标识。
        StationType_unknown = 0,
 // 代表行人类型，用于标记交通场景中的行人相关信息，例如在收集交通数据或者进行交通流分析时区分行人相关的情况。
        StationType_pedestrian = 1,
 // 对应骑自行车的人，用于标识自行车骑行者这一交通参与者类型，有助于在交通管理、规划等应用中单独考虑自行车的相关情况。
        StationType_cyclist = 2,
  // 表示助力车类型，比如电动助力车等，用于区分这类特定的小型两轮交通工具使用者，在不同交通场景中有其独特的行为特点和处理方式。
        StationType_moped = 3,
 // 指代摩托车类型，用于明确交通中摩托车这一相对更高速、机动性较强的交通参与者，其行驶特性等与其他类型有明显区别。
        StationType_motorcycle = 4,
  // 代表乘用车类型，也就是常见的轿车等载人汽车，是道路交通中数量较多的一种交通参与者类型，常用于交通流量统计、路况分析等涉及汽车交通的场景。
        StationType_passengerCar = 5,
 // 用于标识公交车类型，公交车在交通系统中有固定线路、大容量载人等特点，在公共交通管理、调度等方面有专门的处理逻辑。
        StationType_bus = 6,
// 对应轻型卡车类型，区分载货量相对较小的载货汽车，在物流运输、道路荷载分析等应用场景中会单独考虑其影响。
        StationType_lightTruck = 7,
 // 表示重型卡车类型，与轻型卡车相比，重型卡车载货量大、车身较重，对道路的影响以及交通规则适用等方面都有不同之处，所以单独标识。
        StationType_heavyTruck = 8,
// 指代挂车类型，通常需要与牵引车配合使用，在交通运行、车辆追踪等方面其连接和行驶状态有独特的情况需要处理。
        StationType_trailer = 9,
 // 用于表示特殊车辆类型，例如工程抢险车、救护车等具有特殊用途和行驶规则的车辆，它们在交通系统中有特殊的通行权限和处理方式。
        StationType_specialVehicles = 10,
 // 代表有轨电车类型，有轨电车沿着固定轨道行驶，其运行规律、交通信号交互等与其他道路车辆有明显差异，需单独区分。
        StationType_tram = 11,
// 表示路边单元（Road Side Unit），这是智能交通系统中部署在道路路边的设备，用于实现如车辆与基础设施通信、交通信息采集等功能。
        StationType_roadSideUnit = 15        
    } e_StationType;

    /* StationType */
// StationType_t类型别名定义为long类型，用于在程序中统一存储和操作交通站点（或交通参与者类型）的标识数据，
// 可以存放StationType枚举中定义的各种具体类型对应的数值，方便在不同模块间以统一的数据类型进行相关类型的判断与处理。
    typedef long StationType_t;

    /* StationID*/
// StationID_t类型别名最初被定义为long类型，用于唯一标识交通站点（或特定交通参与者个体等），在整个交通系统相关的通信、数据管理等过程中，
// 通过这个唯一的标识符来区分不同的站点或个体，便于进行信息的准确关联和查询等操作。
// 不过此处也可考虑使用unsigned long类型（如下面被注释掉的那行代码所示），具体取决于实际应用场景中对标识符数值范围及特性的需求，
// 例如如果希望避免出现负数标识或者需要更大的正整数表示范围，unsigned long可能更合适。
// typedef unsigned long StationID_t;
    typedef long StationID_t;
    // typedef unsigned long StationID_t;

    /* Dependencies */
// protocolVersion枚举类型用于定义通信协议的版本标识，
// 在智能交通系统中，不同版本的协议可能在消息格式、功能支持等方面存在差异，通过这个枚举来明确当前使用的协议版本情况。
    typedef enum protocolVersion
    {
 // 表示当前正在使用的通信协议版本，用于在系统中统一标识当前所遵循的协议规范，方便进行版本兼容性检查等操作。
        protocolVersion_currentVersion = 1
    } e_protocolVersion;
// messageID枚举类型用于区分不同类型的消息，在智能交通系统的通信过程中，
// 各种消息承载着不同的信息，如车辆状态信息、交通路况信息等，通过该枚举可以清晰地识别每条消息的具体类型，便于进行针对性的处理。
    typedef enum messageID
    {
// 表示自定义类型的消息，当系统中存在用户自行定义格式和内容的消息时，使用该值来标识，方便与标准消息类型区分开。
        messageID_custom = 0,
 // 对应分散式环境通知消息（Decentralized Environmental Notification Message，DENM），这种消息常用于在交通场景中传播局部环境相关的事件信息。
        messageID_denm = 1,
  // 指代协同感知消息（Cooperative Awareness Message，CAM），主要用于车辆之间互相传递自身的状态信息，实现协同感知功能。
        messageID_cam = 2,
 // 代表兴趣点（Point of Interest，POI）消息，通常用于向交通参与者提供周边相关的兴趣点信息，比如附近的加油站、停车场等位置信息。
        messageID_poi = 3,
 // 对应信号相位和定时（Signal Phase and Timing，SPAT）消息，主要用于传递交通信号灯的相位、时长等定时信息，辅助车辆进行合理的通行决策。
        messageID_spat = 4,
 // 表示地图（MAP）消息，这类消息包含了交通相关的地图信息，如道路拓扑结构、限速标识位置等，帮助交通参与者更好地了解周边道路环境。
        messageID_map = 5,
  // 指代车内信息（In-Vehicle Information，IVI）消息，用于在车辆内部或者车辆与外界交互中传递如车内设备状态、多媒体信息等相关内容。
        messageID_ivi = 6,
  // 对应电动汽车远程服务请求（Electric Vehicle Remote Service Request，EV_RSR）消息，主要用于电动汽车与相关服务平台之间进行远程服务相关的通信。
        messageID_ev_rsr = 7
    } e_messageID;
// ItsPduHeader_t结构体用于构建智能交通系统中协议数据单元（Protocol Data Unit，PDU）的头部信息，
// 它包含了协议版本、消息ID以及发送该消息的站点（或交通参与者）的唯一标识符等关键信息，
// 这些信息对于接收方正确解析消息、判断消息来源及兼容性等方面起着重要作用。
    typedef struct ItsPduHeader
    {
 // 存储通信协议的版本号，其类型为long，通过该字段可以确定消息遵循的协议规范，便于进行版本相关的处理与兼容性检查。
        long protocolVersion;
 // 存放消息的具体类型标识，类型为long，接收方可以根据这个字段的值（对应messageID枚举中的不同取值）来确定如何进一步解析消息内容。
        long messageID;
 // 代表发送该消息的站点（或交通参与者）的唯一标识符，类型为StationID_t，用于区分不同来源的消息，方便消息的追踪与管理。
        StationID_t stationID;
    } ItsPduHeader_t;

    /* Heading */
// Heading_t结构体用于综合表示方向（航向）相关的信息，将方向的具体数值（如基于特定坐标系下的角度值）以及对该方向数值准确性的置信程度结合在一起，
// 这样在描述交通参与者的行驶方向或者其他涉及方向的场景时，能够更全面地传达相关信息，便于后续进行方向相关的计算与判断等操作。
    typedef struct Heading
    {
// headingValue用于存储具体的方向数值，其类型为HeadingValue_t，该类型之前已定义，能够表示如基于WGS84坐标系下的具体角度值或者不可用状态等情况。
        HeadingValue_t headingValue;
// headingConfidence存放对应的方向置信度信息，类型是HeadingConfidence_t，反映了对该方向数值准确性的信任程度，同样该类型在之前也已定义好。
        HeadingConfidence_t headingConfidence;
    } Heading_t;

    /* SpeedValue Dependencies */
 //以下定义了一个名为SpeedValue的枚举类型，用于表示速度相关的值。
 //枚举中的每个成员对应不同的速度状态或具体速度值。
    typedef enum SpeedValue
    {
        SpeedValue_standstill = 0, // 表示静止状态，速度值为0
        SpeedValue_oneCentimeterPerSec = 1,// 表示每秒移动1厘米的速度状态，对应值为1
        SpeedValue_unavailable = 16383// 表示速度不可用的状态，对应值为16383
    } e_SpeedValue;
// 定义SpeedValue_t类型为long类型，用于存储速度值相关的数据，可能在后续代码中作为变量类型来使用。
    /* SpeedValue */
    typedef long SpeedValue_t;

    /* SpeedConfidence Dependencies */
 //定义了一个名为SpeedConfidence的枚举类型，用于表示速度的置信度相关情况。
 //不同的枚举成员代表不同程度的速度置信范围或者特殊的置信状态。
    typedef enum SpeedConfidence
    {
        SpeedConfidence_equalOrWithInOneCentimerterPerSec = 1,// 表示速度的置信度在每秒正负1厘米范围内相等，对应值为1
        SpeedConfidence_equalOrWithinOneMeterPerSec = 100, // 表示速度的置信度在每秒正负1米范围内相等，对应值为100
        SpeedConfidence_outOfRange = 126, // 表示速度超出了有效范围，对应值为126
        SpeedConfidence_unavailable = 127 // 表示速度的置信度不可用，对应值为127
    } e_SpeedConfidence;
/ 定义SpeedConfidence_t类型为long类型，用于存储速度置信度相关的数据，方便在程序中作为变量类型进行操作。
    /* SpeedConfidence */
    typedef long SpeedConfidence_t;

    /* Speed */
 // 此结构体用于将速度值（SpeedValue_t类型）和速度置信度（SpeedConfidence_t类型）组合在一起，
 //方便在程序中作为一个整体来处理和传递速度相关的信息。
    typedef struct speed
    {
        SpeedValue_t speedValue; // 存储速度值的成员变量
        SpeedConfidence_t speedConfidence; // 存储速度置信度的成员变量
    } Speed_t;

    /* DriveDirection Dependencies */
 //定义了一个名为DriveDirection的枚举类型，用于表示驱动方向相关的情况。
 //枚举成员对应不同的行驶方向或者特殊的方向状态。
    typedef enum DriveDirection 
    {
        DriveDirection_forward  = 0, // 表示向前行驶的方向，对应值为0
        DriveDirection_backward = 1, // 表示向后行驶的方向，对应值为1
        DriveDirection_unavailable  = 2// 表示驱动方向不可用的状态，对应值为2
    } e_DriveDirection;
 // 定义DriveDirection_t类型为long类型，用于存储驱动方向相关的数据，在后续代码中可作为变量类型使用。
    /* DriveDirection */
    typedef long DriveDirection_t;

    /* VehicleLength Dependencies */
 //这里定义了一个名为VehicleLengthValue的枚举类型，用于表示车辆长度相关的值情况。
 //枚举中的各个成员对应着不同的车辆长度状态或者具体的长度表示值。
    typedef enum VehicleLengthValue
    {
        VehicleLengthValue_tenCentimeters = 1,// 表示车辆长度为10厘米的情况，对应枚举值为1
        VehicleLengthValue_outOfRange = 1022, // 表示车辆长度超出了有效测量范围，对应枚举值为1022
        VehicleLengthValue_unavailable = 1023// 表示车辆长度信息不可用，对应枚举值为1023
    } e_VehicleLengthValue;
// 定义VehicleLengthValue_t类型为long类型，后续可用于存储车辆长度值相关的数据，在程序中充当相应变量的类型。
    /* VehicleLengthValue */
    typedef long VehicleLengthValue_t;

    /* VehicleLengthConfidenceIndication Dependencies */
 //此部分定义了一个名为VehicleLengthConfidenceIndication的枚举类型，
 //用于体现对车辆长度信息的置信度指示情况，不同成员代表不同的关于车辆是否带有拖车以及拖车长度是否已知等相关置信状态。
    typedef enum VehicleLengthConfidenceIndication
    {
        VehicleLengthConfidenceIndication_noTrailerPresent = 0, // 表示车辆当前没有拖车的情况，对应枚举值为0
        VehicleLengthConfidenceIndication_trailerPresentWithKnownLength = 1, // 表示车辆带有拖车且拖车长度已知的情况，对应枚举值为1
        VehicleLengthConfidenceIndication_trailerPresentWithUnknownLength  = 2,// 表示车辆带有拖车但拖车长度未知的情况，对应枚举值为2
        VehicleLengthConfidenceIndication_trailerPresenceIsUnknown = 3, // 表示不确定车辆是否带有拖车的情况，对应枚举值为3
        VehicleLengthConfidenceIndication_unavailable = 4// 表示车辆长度置信度相关信息不可用的情况，对应枚举值为4
    } e_VehicleLengthConfidenceIndication;
// 定义VehicleLengthConfidenceIndication_t类型为long类型，用于存储车辆长度置信度指示相关的数据，便于在程序里作为变量类型操作。
    /* VehicleLengthConfidenceIndication */
    typedef long VehicleLengthConfidenceIndication_t;

    /* VehicleLength */
// 这个结构体将车辆长度值（VehicleLengthValue_t类型）和车辆长度置信度指示（VehicleLengthConfidenceIndication_t类型）结合在一起，
//使得在程序中可以把这两方面相关信息作为一个整体来处理、传递等，方便对车辆长度相关的综合情况进行表示。
    typedef struct VehicleLength
    {
        VehicleLengthValue_t vehicleLengthValue;// 用于存储车辆长度具体值的成员变量
        VehicleLengthConfidenceIndication_t vehicleLengthConfidenceIndication;// 用于存储车辆长度置信度指示情况的成员变量
    } VehicleLength_t;

    /* VehicleWidth Dependencies */
 //以下定义了一个名为VehicleWidth的枚举类型，用来表示车辆宽度相关的值状态。
 //枚举中的成员分别对应不同的车辆宽度情况，比如具体宽度数值或者特殊的宽度相关状态。
    typedef enum VehicleWidth
    {
        VehicleWidth_tenCentimeters = 1,// 表示车辆宽度为10厘米的情况，对应枚举值为1
        VehicleWidth_outOfRange = 61, // 表示车辆宽度超出了正常的有效范围，对应枚举值为61
        VehicleWidth_unavailable = 62 // 表示车辆宽度信息不可用，对应枚举值为62
    } e_VehicleWidth;
// 定义VehicleWidth_t类型为long类型，用于存储车辆宽度相关的数据，在程序后续操作中可以作为合适的变量类型来运用。
    /* VehicleWidth */
    typedef long VehicleWidth_t;

    /* LongitudinalAcceleration Dependencies */
 //此处定义了一个名为LongitudinalAcceletationValue的枚举类型，
 //该枚举用于表示车辆纵向加速度相关的值情况，涵盖了不同方向的加速度以及特殊的表示状态
    typedef enum LongitudinalAcceletationValue
    {
        LongitudinalAccelerationValue_pointOneMeterPerSecSquaredForward = 1, // 表示车辆纵向加速度为每秒平方0.1米向前的情况，对应枚举值为1
        LongitudinalAccelerationValue_pointOneMeterPerSecSquaredBackWard = -1, // 表示车辆纵向加速度为每秒平方0.1米向后的情况，对应枚举值为 -1
        LongitudinalAccelerationValue_unavailable = 161// 表示车辆纵向加速度信息不可用的情况，对应枚举值为161
    } e_LongitudinalAccelerationValue;
// 定义LongitudinalAccelerationValue_t类型为long类型，后续可用于存储纵向加速度值相关的数据，在程序中充当相应变量的类型。
    /* LongitudinalAcclerationValue */
    typedef long LongitudinalAccelerationValue_t;

    /* AccelerationConfidence Dependencies */ 
//这部分定义了一个名为AccelerationConfidence的枚举类型，
// 用于体现对加速度信息的置信度相关情况，不同枚举成员代表不同程度的置信范围或者特殊的置信状态。
    typedef enum AccelerationConfidence
    {
        AccelerationConfindence_pointOneMeterPerSecSquared = 1,// 表示加速度的置信度在每秒平方0.1米范围内，对应枚举值为1
        AccelerationConfidence_outOfRange = 101,// 表示加速度超出了有效范围，对应枚举值为101
        AccelerationConfidence_unavailable = 102 // 表示加速度置信度相关信息不可用，对应枚举值为102
    } e_AccelerationConfidence;
// 定义AccelerationConfidence_t类型为long类型，用于存储加速度置信度相关的数据，便于在程序里作为变量类型操作。
    /* AccelerationConfidence*/
    typedef long AccelerationConfidence_t;

    /* LongitudinalAcceleration */
//该结构体将纵向加速度值（LongitudinalAccelerationValue_t类型）和纵向加速度置信度（AccelerationConfidence_t类型）结合在一起，
// 使得在程序中可以把这两方面相关信息作为一个整体来处理、传递等，方便对车辆纵向加速度相关的综合情况进行表示。
    typedef struct LongitudinalAcceleration
    {
        LongitudinalAccelerationValue_t longitudinalAccelerationValue;// 用于存储车辆纵向加速度具体值的成员变量
        AccelerationConfidence_t longitudinalAccelerationConfidence; // 用于存储车辆纵向加速度置信度情况的成员变量
    } LongitudinalAcceleration_t; 
//以下定义了一个名为CurvatureValue的枚举类型，用来表示道路曲率相关的值状态。
// 枚举中的成员分别对应不同的道路曲率情况，比如直线、向左或向右弯曲以及特殊的曲率相关状态。
    /* CurvatureValue Dependencies */
    typedef enum CurvatureValue
    {
        CurvatureValue_straight = 0,// 表示道路是直线的情况，对应枚举值为0
        CurvatureValue_reciprocalOf1MeterRadiusToRight = -30000,// 表示道路曲率为半径1米的圆向右弯曲（曲率值为半径倒数），对应枚举值为 -30000
        CurvatureValue_reciprocalOf1MeterRadiusToLeft = 30000, // 表示道路曲率为半径1米的圆向左弯曲（曲率值为半径倒数），对应枚举值为30000
        CurvatureValue_unavailable = 30001// 表示道路曲率信息不可用的情况，对应枚举值为30001
    } e_CurvatureValue;
// 定义CurvatureValue_t类型为long类型，用于存储道路曲率相关的数据，在程序后续操作中可以作为合适的变量类型来运用。
    /* CurvatureValue */
    typedef long CurvatureValue_t;

    /* CurvatureConfidence Dependencies*/
    typedef enum CurvatureConfidence
    {
        CurvatureConfidence_onePerMeter_0_00002 = 0, // 表示每米的曲率置信度为0.00002的情况，对应枚举值为0
        CurvatureConfidence_onePerMeter_0_0001  = 1, // 表示每米的曲率置信度为0.0001的情况，对应枚举值为1
        CurvatureConfidence_onePerMeter_0_0005  = 2, // 表示每米的曲率置信度为0.0005的情况，对应枚举值为2
        CurvatureConfidence_onePerMeter_0_002   = 3, // 表示每米的曲率置信度为0.002的情况，对应枚举值为3
        CurvatureConfidence_onePerMeter_0_01    = 4, // 表示每米的曲率置信度为0.01的情况，对应枚举值为4
        CurvatureConfidence_onePerMeter_0_1 = 5,// 表示每米的曲率置信度为0.1的情况，对应枚举值为5
        CurvatureConfidence_outOfRange  = 6,// 表示超出范围的曲率置信度情况，对应枚举值为6
        CurvatureConfidence_unavailable = 7 // 表示不可用的曲率置信度情况，对应枚举值为7
    } e_CurvatureConfidence;
// 为CurvatureConfidence类型定义一个别名CurvatureConfidence_t，方便代码中使用，本质上是long类型
    /* CurvatureConfidence */
    typedef long CurvatureConfidence_t;

    /* Curvature */// 定义结构体Curvature，用于封装曲率相关的信息，包含曲率值和曲率置信度两个成员
    typedef struct Curvature
    {
        CurvatureValue_t curvatureValue; // 表示曲率的具体数值，其类型应该是之前定义的CurvatureValue_t（此处未给出其定义细节）
        CurvatureConfidence_t curvatureConfidence; // 表示该曲率对应的置信度，类型为CurvatureConfidence_t（即long类型的别名）
    } Curvature_t;

    /* CurvatureCalculationMode Dependencies */
    typedef enum CurvatureCalculationMode// 定义枚举类型CurvatureCalculationMode，用于表示不同的曲率计算模式取值情况
    {
        CurvatureCalculationMode_yarRateUsed = 0, // 表示使用偏航率（yaw rate）进行曲率计算的模式，对应枚举值为0
        CurvatureCalculationMode_yarRateNotUsed = 1, // 表示不使用偏航率进行曲率计算的模式，对应枚举值为1
        CurvatureCalculationMode_unavailable = 2 // 表示曲率计算模式不可用的情况，对应枚举值为2
    } e_CurvatureCalculationMode;

    /* CurvatureCalculationMode */// 为CurvatureCalculationMode类型定义一个别名CurvatureCalculationMode_t，方便代码中使用，本质上是long类型
    typedef long CurvatureCalculationMode_t;

    /* YawRateValue Dependencies */// 定义枚举类型YawRateValue，用于表示不同的偏航率值情况
    typedef enum YawRateValue
    {
        YawRateValue_straight = 0, // 表示车辆处于直线行驶状态，对应的偏航率值为0，枚举值设为0
        YawRateValue_degSec_000_01ToRight   = -1, // 表示偏航率为每秒向右转0.01度的情况，用负数表示方向，对应枚举值为 -1
        YawRateValue_degSec_000_01ToLeft    = 1, // 表示偏航率为每秒向左转0.01度的情况，用正数表示方向，对应枚举值为1
        YawRateValue_unavailable    = 32767// 表示偏航率不可用的情况，对应一个较大的特定值32767作为标识
    } e_YawRateValue;

    /* YawRateValue */// 为YawRateValue类型定义一个别名YawRateValue_t，方便代码中使用，本质上是long类型
    typedef long YawRateValue_t;

    /* YawRateConfidence Dependencies */
    typedef enum YawRateConfidence {
        YawRateConfidence_degSec_000_01 = 0,// 表示偏航率的置信度为每秒0.00001度的级别，对应枚举值为0
        YawRateConfidence_degSec_000_05 = 1,  // 表示偏航率的置信度为每秒0.00005度的级别，对应枚举值为1
        YawRateConfidence_degSec_000_10 = 2, // 表示偏航率的置信度为每秒0.00010度的级别，对应枚举值为2
        YawRateConfidence_degSec_001_00 = 3,// 表示偏航率的置信度为每秒0.00100度的级别，对应枚举值为3
        YawRateConfidence_degSec_005_00 = 4,// 表示偏航率的置信度为每秒0.00500度的级别，对应枚举值为4
        YawRateConfidence_degSec_010_00 = 5, // 表示偏航率的置信度为每秒0.01000度的级别，对应枚举值为5
        YawRateConfidence_degSec_100_00 = 6,// 表示偏航率的置信度为每秒0.10000度的级别，对应枚举值为6
        YawRateConfidence_outOfRange    = 7, // 表示偏航率置信度超出正常范围的情况，对应枚举值为7
        YawRateConfidence_unavailable   = 8 // 表示偏航率置信度不可用的情况，对应枚举值为8
    } e_YawRateConfidence;
 
    /* YawRateConfidence */
    typedef long YawRateConfidence_t;// 为YawRateConfidence类型定义一个别名YawRateConfidence_t，方便在代码中使用，其本质上是long类型

    /* YawRate */
    typedef struct YawRate
    {
        YawRateValue_t yawRateValue; // 表示偏航率的具体数值，其类型为之前定义的YawRateValue_t（应该在其他地方有对应定义）
        YawRateConfidence_t yawRateConfidence; // 表示该偏航率对应的置信度，类型为YawRateConfidence_t（即long类型别名）
    } YawRate_t;

    /* AccelerationControl Dependencies */
    typedef enum AccelerationControl {
        AccelerationControl_brakePedalEngaged   = 0, // 表示刹车踏板被踩下的状态，对应枚举值为0
        AccelerationControl_gasPedalEngaged = 1, // 表示油门踏板被踩下的状态，对应枚举值为1
        AccelerationControl_emergencyBrakeEngaged   = 2,// 表示紧急刹车被启动的状态，对应枚举值为2
        AccelerationControl_collisionWarningEngaged = 3,// 表示碰撞预警系统被触发的状态，对应枚举值为3
        AccelerationControl_accEngaged  = 4, // 表示加速功能被启用的状态（此处具体哪种加速功能可根据上下文确定），对应枚举值为4
        AccelerationControl_cruiseControlEngaged    = 5, // 表示定速巡航功能被启用的状态，对应枚举值为5
        AccelerationControl_speedLimiterEngaged = 6// 表示限速功能被启用的状态，对应枚举值为6
    } e_AccelerationControl;
 
    /* AccelerationControl */
    typedef uint8_t AccelerationControl_t;// 将AccelerationControl_t定义为uint8_t类型，用于后续表示加速度控制相关的变量等，可能是基于其取值范围适合用8位无符号整数表示

    /* LanePosition Dependencies */
// 定义枚举类型LanePosition，用于列举车辆可能处于的不同车道位置情况
    typedef enum LanePosition {
        LanePosition_offTheRoad = -1, // 表示车辆已经偏离正常道路范围，例如开到道路外面了，对应枚举值为 -1
        LanePosition_hardShoulder   = 0,// 表示车辆处于硬路肩位置，通常是道路边缘供临时停车等用途的区域，对应枚举值为0
        LanePosition_outermostDrivingLane   = 1, // 表示车辆处于最外侧的行车道上，对应枚举值为1
        LanePosition_secondLaneFromOutside  = 2// 表示车辆处于从最外侧数起的第二车道位置，对应枚举值为2
    } e_LanePosition;
 
    /* LanePosition */
    typedef long LanePosition_t;/ 为LanePosition类型定义一个别名LanePosition_t，方便在代码其他地方使用，本质上是long类型，便于统一处理车道位置相关的数据

    /* SteeringWheelAngleValue Dependencies */
    typedef enum SteeringWheelAngleValue {
        SteeringWheelAngleValue_straight    = 0,// 表示方向盘处于正前方直线行驶对应的角度位置，对应枚举值为0
        SteeringWheelAngleValue_onePointFiveDegreesToRight  = -1, // 表示方向盘向右转了1.5度的情况，用负数表示向右转动方向，对应枚举值为 -1
        SteeringWheelAngleValue_onePointFiveDegreesToLeft   = 1, // 表示方向盘向左转了1.5度的情况，用正数表示向左转动方向，对应枚举值为1
        SteeringWheelAngleValue_unavailable = 512 // 表示方向盘角度信息不可用的情况，对应一个特定的枚举值512作为标识
    } e_SteeringWheelAngleValue;
 
    /* SteeringWheelAngleValue */
    typedef long SteeringWheelAngleValue_t;// 为SteeringWheelAngleValue类型定义一个别名SteeringWheelAngleValue_t，方便在代码中使用，本质上是long类型，便于统一处理方向盘角度值相关的数据

    /* SteeringWheelAngleConfidence Dependencies */
    typedef enum SteeringWheelAngleConfidence {
        SteeringWheelAngleConfidence_equalOrWithinOnePointFiveDegree    = 1, // 表示方向盘角度的置信度为等于或在1.5度范围内的情况，对应枚举值为1
        SteeringWheelAngleConfidence_outOfRange = 126,// 表示方向盘角度置信度超出正常范围的情况，对应枚举值为126
        SteeringWheelAngleConfidence_unavailable    = 127// 表示方向盘角度置信度不可用的情况，对应枚举值为127
    } e_SteeringWheelAngleConfidence;
 
    /* SteeringWheelAngleConfidence */
    typedef long SteeringWheelAngleConfidence_t;// 为SteeringWheelAngleConfidence类型定义一个别名SteeringWheelAngleConfidence_t，方便在代码中使用，本质上是long类型，便于统一处理方向盘角度置信度相关的数据

    /* SteeringWheelAngle */
    typedef struct SteeringWheelAngle
    {
        SteeringWheelAngleValue_t steeringWheelAngleValue;// 表示方向盘的具体角度值，其类型为之前定义的SteeringWheelAngleValue_t
        SteeringWheelAngleConfidence_t steeringWheelAngleConfidence;// 表示该方向盘角度对应的置信度，类型为SteeringWheelAngleConfidence_t
    } SteeringWheelAngle_t;

    /* LateralAccelerationValue Dependencies */
    typedef enum LateralAccelerationValue {
        LateralAccelerationValue_pointOneMeterPerSecSquaredToRight  = -1, // 表示车辆横向加速度为每秒平方0.1米且方向向右的情况，用负数表示向右方向，对应枚举值为 -1
        LateralAccelerationValue_pointOneMeterPerSecSquaredToLeft   = 1,  // 表示车辆横向加速度为每秒平方0.1米且方向向左的情况，用正数表示向左方向，对应枚举值为1
        LateralAccelerationValue_unavailable    = 161// 表示车辆横向加速度信息不可用的情况，对应一个特定的枚举值161作为标识
    } e_LateralAccelerationValue;

    /* LateralAccelerationValue */
    typedef long LateralAccelerationValue_t;// 为LateralAccelerationValue类型定义一个别名LateralAccelerationValue_t，方便在代码中使用，本质上是long类型，便于统一处理横向加速度值相关的数据

    /* LateralAcceleration */
    typedef struct LateralAcceleration
    {
        LateralAccelerationValue_t lateralAccelerationValue;// 表示车辆横向加速度的具体数值，其类型为之前定义的LateralAccelerationValue_t
        AccelerationConfidence_t lateralAccelerationConfidence; // 表示该横向加速度对应的置信度，类型为AccelerationConfidence_t
    } LateralAcceleration_t;

    /* VerticalAccelerationValue Dependencies */
    typedef enum VerticalAccelerationValue {
        VerticalAccelerationValue_pointOneMeterPerSecSquaredUp  = 1, // 表示车辆垂直加速度为每秒平方0.1米且方向向上的情况，用正数表示向上方向，对应枚举值为1
        VerticalAccelerationValue_pointOneMeterPerSecSquaredDown    = -1, // 表示车辆垂直加速度为每秒平方0.1米且方向向下的情况，用负数表示向下方向，对应枚举值为 -1
        VerticalAccelerationValue_unavailable   = 161// 表示车辆垂直加速度信息不可用的情况，对应一个特定的枚举值161作为标识
    } e_VerticalAccelerationValue;
 
    /* VerticalAccelerationValue */
    typedef long VerticalAccelerationValue_t;

    /* VerticalAcceleration */
    typedef struct VerticalAcceleration
    {
        VerticalAccelerationValue_t verticalAccelerationValue; // 表示车辆垂直加速度的具体数值，其类型为之前定义的VerticalAccelerationValue_t
        AccelerationConfidence_t verticalAccelerationConfidence;// 表示该垂直加速度对应的置信度，类型为AccelerationConfidence_t
    } VerticalAcceleration_t;

    /* PerformanceClass Dependencies */
    typedef enum PerformanceClass {
        PerformanceClass_unavailable    = 0,// 表示车辆性能等级不可用的情况，对应枚举值为0
        PerformanceClass_performanceClassA  = 1,// 表示车辆属于性能等级A的情况，对应枚举值为1
        PerformanceClass_performanceClassB  = 2 // 表示车辆属于性能等级B的情况，对应枚举值为2
    } e_PerformanceClass;
 
    /* PerformanceClass */
    typedef long PerformanceClass_t;// 为PerformanceClass类型定义一个别名PerformanceClass_t，方便在代码中使用，本质上是long类型，便于统一处理车辆性能等级相关的数据

    /* ProtectedZoneID */
    typedef long ProtectedZoneID_t;
// 定义类型别名ProtectedZoneID_t，用于表示保护区（可能是特定区域相关的概念）的标识，本质上是long类型，方便在后续代码中使用该类型处理相关逻辑

    /* CenDsrcTollingZoneID */
    typedef ProtectedZoneID_t CenDsrcTollingZoneID_t;// 定义类型别名CenDsrcTollingZoneID_t，它等同于ProtectedZoneID_t类型，可能用于表示特定的基于DSRC（专用短程通信）的收费区域标识，方便代码中统一使用该类型进行相关操作。

    /* CenDsrcTollingZone */
    typedef struct CenDsrcTollingZone {
        Latitude_t   protectedZoneLatitude;// 表示该收费区域的纬度信息，其类型应该是之前定义的Latitude_t（可能在其他地方有定义）
        Longitude_t  protectedZoneLongitude; // 表示该收费区域的经度信息，其类型应该是之前定义的Longitude_t（可能在其他地方有定义）
        CenDsrcTollingZoneID_t  cenDsrcTollingZoneID;   /* OPTIONAL */// 表示该基于DSRC的收费区域的标识，类型为CenDsrcTollingZoneID_t，并且此成员是可选的（可能在某些情况下不存在）
        OptionalValueAvailable_t cenDsrcTollingZoneIDAvailable; // 表示上述收费区域标识是否可用的信息，其类型应该是OptionalValueAvailable_t（可能在其他地方有定义）
    } CenDsrcTollingZone_t;

    /* ProtectedZoneType Dependencies */
    typedef enum ProtectedZoneType {
        ProtectedZoneType_cenDsrcTolling    = 0

    } e_ProtectedZoneType;
 
    /* ProtectedZoneType */
    typedef long     ProtectedZoneType_t;// 为ProtectedZoneType类型定义一个别名ProtectedZoneType_t，方便在代码中统一使用，本质上是long类型，便于后续处理与保护区类型相关的数据。

    /* TimestampIts Dependencies */
    typedef enum TimestampIts {
        TimestampIts_utcStartOf2004 = 0,// 表示2004年UTC时间起始时刻对应的时间戳，对应枚举值为0，可作为一个基准时间参考点。
        TimestampIts_oneMillisecAfterUTCStartOf2004 = 1 // 表示在2004年UTC时间起始时刻之后1毫秒的时间戳，对应枚举值为1，用于表示相对起始时刻稍有延迟的时间点。
    } e_TimestampIts;



    /* TimestampIts */
    typedef long TimestampIts_t; // 为TimestampIts类型定义一个别名TimestampIts_t，方便在代码中使用，本质上是long类型，便于统一处理与ITS时间戳相关的数据。

    /* ProtectedZoneRadius Dependencies */
    typedef enum ProtectedZoneRadius {
        ProtectedZoneRadius_oneMeter    = 1 // 表示保护区半径为1米的情况，对应枚举值为1，用于界定保护区的空间范围大小。
    } e_ProtectedZoneRadius;
 
    /* ProtectedZoneRadius */
    typedef long ProtectedZoneRadius_t;

    /* ProtectedCommunicationZone */
    typedef struct ProtectedCommunicationZone {
        ProtectedZoneType_t  protectedZoneType;// 表示该受保护通信区域的类型，其类型为之前定义的ProtectedZoneType_t，用于明确此区域所属的具体类型。
        TimestampIts_t  expiryTime /* OPTIONAL */; // 表示该受保护通信区域的过期时间，类型为TimestampIts_t，此成员是可选的（可能在某些情况下不存在），用于限定区域有效的时间范围。
        OptionalValueAvailable_t expiryTimeAvailable; // 表示上述过期时间是否可用的信息，其类型应该是OptionalValueAvailable_t（可能在其他地方有定义），用于判断过期时间信息的有效性。
        Latitude_t   protectedZoneLatitude; // 表示该受保护通信区域的纬度信息，其类型应该是之前定义的Latitude_t（可能在其他地方有定义），用于确定区域在地理上的纬度位置。
        Longitude_t  protectedZoneLongitude;// 表示该受保护通信区域的经度信息，其类型应该是之前定义的Longitude_t（可能在其他地方有定义），用于确定区域在地理上的经度位置。
        ProtectedZoneRadius_t   protectedZoneRadius    /* OPTIONAL */; // 表示该受保护通信区域的半径信息，类型为ProtectedZoneRadius_t，此成员是可选的（可能在某些情况下不存在），用于进一步明确区域的范围大小。
        OptionalValueAvailable_t protectedZoneRadiusAvailable; // 表示上述区域半径是否可用的信息，其类型应该是OptionalValueAvailable_t（可能在其他地方有定义），用于判断半径信息的有效性。
        ProtectedZoneID_t   protectedZoneID    /* OPTIONAL */;// 表示该受保护通信区域的标识信息，类型为ProtectedZoneID_t，此成员是可选的（可能在某些情况下不存在），用于唯一标识该区域。
        OptionalValueAvailable_t protectedZoneIDAvailable;// 表示上述区域标识是否可用的信息，其类型应该是OptionalValueAvailable_t（可能在其他地方有定义），用于判断标识信息的有效性。
    } ProtectedCommunicationZone_t;

    /* ProtectedCommunicationZonesRSU */
    typedef struct ProtectedCommunicationZonesRSU {
        long ProtectedCommunicationZoneCount;
        std::vector<ProtectedCommunicationZone_t> list;
    } ProtectedCommunicationZonesRSU_t;

    /* VehicleRole Dependencies */
    typedef enum VehicleRole {
        VehicleRole_default = 0,
        VehicleRole_publicTransport = 1,
        VehicleRole_specialTransport    = 2,
        VehicleRole_dangerousGoods  = 3,
        VehicleRole_roadWork    = 4,
        VehicleRole_rescue  = 5,
        VehicleRole_emergency   = 6,
        VehicleRole_safetyCar   = 7,
        VehicleRole_agriculture = 8,
        VehicleRole_commercial  = 9,
        VehicleRole_military    = 10,
        VehicleRole_roadOperator    = 11,
        VehicleRole_taxi    = 12,
        VehicleRole_reserved1   = 13,
        VehicleRole_reserved2   = 14,
        VehicleRole_reserved3   = 15
    } e_VehicleRole;
 
    /* VehicleRole */
    typedef long VehicleRole_t;

    /* ExteriorLights Dependencies */
    typedef enum ExteriorLights {
        ExteriorLights_lowBeamHeadlightsOn  = 0,
        ExteriorLights_highBeamHeadlightsOn = 1,
        ExteriorLights_leftTurnSignalOn = 2,
        ExteriorLights_rightTurnSignalOn    = 3,
        ExteriorLights_daytimeRunningLightsOn   = 4,
        ExteriorLights_reverseLightOn   = 5,
        ExteriorLights_fogLightOn   = 6,
        ExteriorLights_parkingLightsOn  = 7
    } e_ExteriorLights;

    /* ExteriorLights */
    typedef uint8_t ExteriorLights_t;
    /* DeltaLatitude Dependencies */
    typedef enum DeltaLatitude {
        DeltaLatitude_oneMicrodegreeNorth   = 10,
        DeltaLatitude_oneMicrodegreeSouth   = -10,
        DeltaLatitude_unavailable   = 131072
    } e_DeltaLatitude;
 
    /* DeltaLatitude */
    typedef long DeltaLatitude_t;

    /* DeltaLongitude Dependencies */
    typedef enum DeltaLongitude {
        DeltaLongitude_oneMicrodegreeEast   = 10,
        DeltaLongitude_oneMicrodegreeWest   = -10,
        DeltaLongitude_unavailable  = 131072
    } e_DeltaLongitude;
 
    /* DeltaLongitude */
    typedef long DeltaLongitude_t;

    /* DeltaAltitude Dependencies */
    typedef enum DeltaAltitude {
        DeltaAltitude_oneCentimeterUp   = 1,
        DeltaAltitude_oneCentimeterDown = -1,
        DeltaAltitude_unavailable   = 12800
    } e_DeltaAltitude;
 
    /* DeltaAltitude */
    typedef long DeltaAltitude_t;

    /* DeltaReferencePosition */
    typedef struct DeltaReferencePosition {
        DeltaLatitude_t  deltaLatitude;
        DeltaLongitude_t     deltaLongitude;
        DeltaAltitude_t  deltaAltitude;
    } DeltaReferencePosition_t;

    /* PathDeltaTime Dependencies */
    typedef enum PathDeltaTime {
        PathDeltaTime_tenMilliSecondsInPast = 1
    } e_PathDeltaTime;
 
    /* PathDeltaTime */
    typedef long PathDeltaTime_t;

    /* PathPoint */
    typedef struct PathPoint {
        DeltaReferencePosition_t     pathPosition;
        PathDeltaTime_t *pathDeltaTime  /* OPTIONAL */;

    } PathPoint_t;

    /* PathHistory */
    typedef struct PathHistory {
        long NumberOfPathPoint;
        std::vector<PathPoint_t> data;
    } PathHistory_t;
};

class CAMContainer
{
public:

    /* GenerationDeltaTime Dependencies*/
    typedef enum GenerationDeltaTime
    {
        GenerationDeltaTime_oneMilliSec = 1
    } e_GenerationDeltaTime;

    /* GenerationDeltaTime */
    typedef long GenerationDeltaTime_t;

    /* BasicContainer */
    typedef struct BasicContainer
    {
        ITSContainer::StationType_t stationType;
        ITSContainer::ReferencePosition_t referencePosition;
    } BasicContainer_t;

    /* HighFrequencyContainer Dependencies */
    typedef enum HighFrequencyContainer_PR : long
    {
        HighFrequencyContainer_PR_NOTHING, /* No components present */
        HighFrequencyContainer_PR_basicVehicleContainerHighFrequency,
        HighFrequencyContainer_PR_rsuContainerHighFrequency
    } HighFrequencyContainer_PR;

    typedef bool OptionalStructAvailable_t;

    /* BasicVehicleContainerHighFrequency*/
    typedef struct BasicVehicleContainerHighFrequency
    {
        ITSContainer::Heading_t heading;
        ITSContainer::Speed_t speed;
        ITSContainer::DriveDirection_t driveDirection;
        ITSContainer::VehicleLength_t vehicleLength;
        ITSContainer::VehicleWidth_t vehicleWidth;
        ITSContainer::LongitudinalAcceleration_t longitudinalAcceleration;
        ITSContainer::Curvature_t curvature;
        ITSContainer::CurvatureCalculationMode_t   curvatureCalculationMode;
        ITSContainer::YawRate_t    yawRate;
        
        OptionalStructAvailable_t accelerationControlAvailable;
        ITSContainer::AccelerationControl_t   accelerationControl    /* OPTIONAL */;
        
        OptionalStructAvailable_t lanePositionAvailable;
        ITSContainer::LanePosition_t lanePosition   /* OPTIONAL */;
        
        OptionalStructAvailable_t steeringWheelAngleAvailable;
        ITSContainer::SteeringWheelAngle_t   steeringWheelAngle /* OPTIONAL */;

        OptionalStructAvailable_t lateralAccelerationAvailable;
        ITSContainer::LateralAcceleration_t  lateralAcceleration    /* OPTIONAL */;

        OptionalStructAvailable_t verticalAccelerationAvailable;
        ITSContainer::VerticalAcceleration_t verticalAcceleration   /* OPTIONAL */;

        OptionalStructAvailable_t performanceClassAvailable;
        ITSContainer::PerformanceClass_t performanceClass   /* OPTIONAL */;

        OptionalStructAvailable_t cenDsrcTollingZoneAvailable;
        ITSContainer::CenDsrcTollingZone_t   cenDsrcTollingZone /* OPTIONAL */;
        
    } BasicVehicleContainerHighFrequency_t;

    /* RsuContainerHighFrequency */
    typedef struct RSUContainerHighFrequency
    {
        ITSContainer::ProtectedCommunicationZonesRSU_t protectedCommunicationZonesRSU;
    } RSUContainerHighFrequency_t;

    /* HighFrequencyContainer */
    typedef struct HighFrequencyContainer
    {
        HighFrequencyContainer_PR present;

        BasicVehicleContainerHighFrequency_t basicVehicleContainerHighFrequency;
        RSUContainerHighFrequency_t rsuContainerHighFrequency;

    } HighFrequencyContainer_t;

    /* Dependencies */
    typedef enum LowFrequencyContainer_PR : long
    {
        LowFrequencyContainer_PR_NOTHING,   /* No components present */
        LowFrequencyContainer_PR_basicVehicleContainerLowFrequency,
        /* Extensions may appear below */
     
    } LowFrequencyContainer_PR;

    /* BasicVehicleContainerLowFrequency */
    typedef struct BasicVehicleContainerLowFrequency {
        ITSContainer::VehicleRole_t    vehicleRole;
        ITSContainer::ExteriorLights_t     exteriorLights;
        ITSContainer::PathHistory_t    pathHistory;
    } BasicVehicleContainerLowFrequency_t;

    /* LowFrequencyContainer */
    typedef struct LowFrequencyContainer
    {
        LowFrequencyContainer_PR present;
        // Since only option is available
        BasicVehicleContainerLowFrequency_t basicVehicleContainerLowFrequency;

    } LowFrequencyContainer_t;

    /* CamParameters */
    typedef struct CamParameters
    {
        BasicContainer_t basicContainer;
        HighFrequencyContainer_t highFrequencyContainer;
        LowFrequencyContainer_t lowFrequencyContainer; /* OPTIONAL */
        // Optional TODO: SpecialVehicleContainer *specialVehicleContainer
    } CamParameters_t;

    /* CoopAwareness*/
    typedef struct CoopAwareness 
    {
        GenerationDeltaTime_t generationDeltaTime;
        CamParameters_t camParameters;
    } CoopAwareness_t;


};

    /* CoopAwareness */
    typedef struct CAM
    {
        ITSContainer::ItsPduHeader_t header;
        CAMContainer::CoopAwareness_t cam;
    } CAM_t;


    typedef struct CustomV2XM
    {
        ITSContainer::ItsPduHeader_t header;
        char message[100];
    } CustomV2XM_t;
