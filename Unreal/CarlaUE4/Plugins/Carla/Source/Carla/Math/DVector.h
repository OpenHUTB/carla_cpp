// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>

//理想情况下，这个（结构体定义）本可以直接包含在虚幻引擎（Unreal）中，但那样使用起来可能会比较低效。这是一个代码相关的注释，用于说明这段代码的一些背景或者设想情况
struct FDVector
{

  double X = 0.0;
// 定义结构体成员变量 X，类型为双精度浮点数（double），初始化为0.0，用于表示向量在 X 维度上的分量
    double Y = 0.0;
  double Y = 0.0;
// 定义结构体成员变量 Y，类型为双精度浮点数（double），初始化为0.0，用于表示向量在 Y 维度上的分量
  double Z = 0.0;
    // 定义结构体成员变量 Z，类型为双精度浮点数（double），初始化为0.0，用于表示向量在 Z 维度上的分量

  FDVector() : X(0.0), Y(0.0), Z(0.0) {}
 // 定义默认构造函数，使用初始化列表将成员变量 X、Y、Z 都初始化为0.0，用于创建一个默认的（各分量都为0的）向量对象

  FDVector(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ) {}
// 定义一个构造函数，接收三个单精度浮点数（float）类型的参数 InX、InY、InZ，
    // 通过初始化列表分别将成员变量 X、Y、Z 初始化为传入的对应参数值，用于根据给定的浮点数分量创建向量对象

  FDVector(double InX, double InY, double InZ) : X(InX), Y(InY), Z(InZ) {}
// 定义一个构造函数，接收三个双精度浮点数（double）类型的参数 InX、InY、InZ，
    // 同样利用初始化列表把成员变量 X、Y、Z 初始化为传入的相应参数值，方便以双精度浮点数形式给定分量来创建向量对象

  FDVector(const FVector& V) : X(V.X), Y(V.Y), Z(V.Z) {}
// 定义一个构造函数，接收一个常量引用类型的 FVector 参数 V（FVector 可能是虚幻引擎中已有的表示向量的类型），
    // 通过初始化列表将本结构体的成员变量 X、Y、Z 分别初始化为传入的 FVector 参数的对应分量值，
    // 用于从已有的 FVector 类型对象来创建本结构体表示的向量对象

  FDVector(const FIntVector& V) : X(V.X), Y(V.Y), Z(V.Z) {}//构造函数，使用初始化列表来初始化
// 定义一个构造函数，接收一个常量引用类型的 FIntVector 参数 V（FIntVector 可能是虚幻引擎中表示整型向量的类型），
    // 通过初始化列表将成员变量 X、Y、Z 分别初始化为传入的 FIntVector 参数的对应分量值，
    // 从而可以基于整型向量对象来创建本结构体的向量对象，注释再次强调了使用初始化列表来进行初始化的操作方式

  double Size() const
// 定义一个成员函数 Size，返回类型为双精度浮点数（double），函数被声明为 const，意味着调用该函数不会修改对象的状态，
    // 其功能是用于计算并返回向量的长度（通常是根据向量各分量通过勾股定理计算向量的模长）
  {
    return std::sqrt(X*X + Y*Y + Z*Z);//用于计算并返回向量长度
  }

  double SizeSquared() const//用于返回向量长度的平方
  {
    return X*X + Y*Y + Z*Z;  // 直接返回向量各分量的平方和，也就是向量长度的平方
  }

  static double Dist(const FDVector &V1, const FDVector &V2)//用于计算距离的静态函数
  {
    return std::sqrt(FDVector::DistSquared(V1, V2)); // 通过调用本类的静态函数 DistSquared 先计算两个向量距离的平方，然后再使用 std::sqrt 函数取平方根，得到实际的距离值
  }

  static double DistSquared(const FDVector &V1, const FDVector &V2)
 // 定义一个静态成员函数 DistSquared，接收两个常量引用类型的 FDVector 参数 V1 和 V2，返回类型为双精度浮点数（double），
    // 用于计算两个向量之间距离的平方（同样常用于一些比较距离相关情况，避免开方运算来提高效率等）
  {
    return FMath::Square(V2.X-V1.X) + FMath::Square(V2.Y-V1.Y) + FMath::Square(V2.Z-V1.Z);
  // 使用 FMath 相关函数（可能是虚幻引擎自定义的数学函数库）的 Square 函数（用于计算平方），
        // 分别计算两个向量各对应分量差值的平方，然后求和，得到两个向量距离的平方值
  }

  FVector ToFVector() const
// 定义一个成员函数 ToFVector，返回类型为 FVector（虚幻引擎中已有的向量类型），const 修饰表示不改变对象状态，
    // 功能是将本结构体表示的向量转换为 FVector 类型的向量对象
  {
    return FVector(X, Y, Z);
// 通过使用本结构体的成员变量 X、Y、Z 来创建并返回一个 FVector 类型的向量对象，实现类型转换
  }

  FString ToString() const
// 定义一个成员函数 ToString，返回类型为 FString（虚幻引擎中用于表示字符串的类型），const 修饰表明不修改对象，
    // 其功能是将向量对象转换为一个格式特定的字符串，方便输出显示或者其他需要以字符串形式表示向量的场景
  {
    return FString::Printf(TEXT("X=%.2lf Y=%.2lf Z=%.2lf"), X, Y, Z);
// 使用 FString 的 Printf 函数（类似 C 语言中的 printf，用于格式化字符串生成），按照指定格式（保留两位小数的浮点数形式展示各分量）
        // 将向量的各分量组合成一个字符串并返回
  }

  FIntVector ToFIntVector() const
// 定义一个成员函数 ToFIntVector，返回类型为 FIntVector（虚幻引擎中整型向量类型），const 修饰表示不改变对象状态，
    // 用于将本结构体表示的向量转换为整型向量对象
  {
    return FIntVector((int32)X, (int32)Y, (int32)Z);
 // 通过将本结构体的双精度浮点数成员变量 X、Y、Z 强制转换为 32 位整型（int32），然后创建并返回一个 FIntVector 类型的整型向量对象，实现类型转换
  }

  FDVector& operator=(const FDVector& Other)//将对象转换为一个格式的字符串
  {
    this->X = Other.X;
    this->Y = Other.Y;
    this->Z = Other.Z;
    return *this;
  }

  bool operator==(const FDVector& Other)
// 重载相等运算符（==），返回类型为布尔值（bool），接收一个常量引用类型的 FDVector 参数 Other，
    // 用于比较两个 FDVector 对象是否相等（即各对应分量是否都相等）
  {
    return X == Other.X &&
           Y == Other.Y &&
           Z == Other.Z;
 // 通过分别比较两个对象的各分量是否相等，只有当 X、Y、Z 分量都相等时才返回 true，表示两个向量相等，否则返回 false
  }

  FDVector operator+(const FDVector& V) const
// 重载加法运算符（+），返回类型为 FDVector，接收一个常量引用类型的 FDVector 参数 V，函数被声明为 const，
    // 表示调用该运算符函数不会修改参与运算的对象状态，其功能是实现两个向量的加法运算，返回相加后的新向量
  {
    return FDVector(X + V.X, Y + V.Y, Z + V.Z);
        // 创建一个新的 FDVector 对象，其各分量分别是当前向量对象与传入的参数向量 V 的对应分量相加的结果，然后返回这个新的向量对象，实现向量加法
  }

  FDVector& operator+=(float Scalar)
  {
    this->X += Scalar;
    this->Y += Scalar;
    this->Z += Scalar;
    return *this;
  }
  
  FDVector operator-(const FDVector& V) const
 // 重载复合加法赋值运算符（+=），返回类型为 FDVector 的引用，接收一个单精度浮点数（float）参数 Scalar，
    // 用于实现将当前向量的各分量都加上给定的标量值，并返回修改后的当前向量对象引用，支持类似 a += 5; 这样的连续赋值操作
  {
    return FDVector(X - V.X, Y - V.Y, Z - V.Z);
  }

  FDVector operator-=(const FIntVector& V) const
  {
    return FDVector(X - V.X, Y - V.Y, Z - V.Z);
  }

  FDVector& operator-=(const FIntVector& V)
  {
    this->X -= V.X;
    this->Y -= V.Y;
    this->Z -= V.Z;
    return *this;
// 将当前向量对象的各分量分别加上传入的标量值 Scalar，然后返回当前向量对象的引用
  }

  FDVector operator/(float Scale) const
// 重载减法运算符（-），返回类型为 FDVector，接收一个常量引用类型的 FDVector 参数 V，const 修饰表示不修改对象状态，
    // 用于实现两个向量的减法运算，返回相减后的新向量
  {
    const float RScale = 1.f/Scale;
    return FDVector(X * RScale, Y * RScale, Z * RScale);
 // 先计算标量的倒数（为了避免直接除以 Scale 可能出现的除以0等问题，先求倒数然后用乘法实现除法运算），
        // 然后创建一个新的 FDVector 对象，其各分量分别是当前向量对象的对应分量乘以这个倒数的结果，最后返回新向量对象，实现向量除以标量的运算
  }

  FDVector operator*(float Scale) const
// 重载乘法运算符（*），返回类型为 FDVector，接收一个单精度浮点数（float）参数 Scale，const 修饰表示不修改对象状态，
    // 用于实现向量与标量的乘法运算，即把向量的各分量都乘以给定的标量值，返回相乘后的新向量
  {
    return FDVector(X * Scale, Y * Scale, Z * Scale);
// 创建一个新的 FDVector 对象，其各分量分别是当前向量对象的对应分量乘以传入的标量值 Scale 的结果，然后返回这个新向量对象，实现向量与标量的乘法运算
  }

  FDVector operator*=(float Scale)
 // 重载复合乘法赋值运算符（*=），返回类型为 FDVector 的引用，接收一个单精度浮点数（float）参数 Scale，
    // 用于实现将当前向量的各分量都乘以给定的标量值，并返回修改后的当前向量对象引用，支持连续赋值操作
  {
    this->X *= Scale;
    this->Y *= Scale;
    this->Z *= Scale;
    return *this;
// 将当前向量对象的各分量分别乘以传入的标量值 Scale，然后返回当前向量对象的引用
  }

  FDVector operator*=(double Scale)
// 重载复合乘法赋值运算符（*=），返回类型为 FDVector 的引用，接收一个双精度浮点数（double）参数 Scale，
    // 用于实现将当前向量的各分量都乘以给定的双精度浮点数标量值，并返回修改后的当前向量对象引用，支持连续赋值操作
    
  {
    this->X *= Scale;
    this->Y *= Scale;
    this->Z *= Scale;
    return *this;
      // 将当前向量对象的各分量分别乘以传入的双精度浮点数标量值 Scale，然后返回当前向量对象的引用
  }
};
