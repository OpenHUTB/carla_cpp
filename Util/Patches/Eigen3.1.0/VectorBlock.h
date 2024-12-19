// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_VECTORBLOCK_H
#define EIGEN_VECTORBLOCK_H

namespace Eigen { 
// 定义名为 "Eigen" 的命名空间，Eigen 通常是一个用于线性代数运算等功能的库，在这个命名空间内会定义相关的类、函数等实现其各种功能。

/** \class VectorBlock
  * \ingroup Core_Module
  *
  * \brief Expression of a fixed-size or dynamic-size sub-vector
  *
  * \param VectorType the type of the object in which we are taking a sub-vector
  * \param Size size of the sub-vector we are taking at compile time (optional)
  *
  * 该类表示一个固定大小或动态大小的子向量的表达式。
  * 它是 DenseBase::segment(Index,Index) 和 DenseBase::segment<int>(Index) 的返回类型，
  * 并且大多数情况下仅通过这种方式使用。
  *
  * 然而，如果您想要直接操作子向量表达式，例如编写一个返回这样表达式的函数，
  * 则需要使用这个类。
  *
  * 以下是一个动态情况的示例：
  * \include class_VectorBlock.cpp
  * Output: \verbinclude class_VectorBlock.out
  *
  * 注意：即使此表达式具有动态大小，但在 VectorType 具有固定大小的情况下，
  * 此表达式继承了一个固定的最大大小，这意味着对其求值不会导致动态内存分配。
  *
  * 以下是一个固定大小情况的示例：
  * \include class_FixedVectorBlock.cpp
  * Output: \verbinclude class_FixedVectorBlock.out
  *
  * \sa class Block, DenseBase::segment(Index,Index,Index,Index), DenseBase::segment(Index,Index)
  */
// 这是一段 Doxygen 风格的注释，用于对下面要定义的 VectorBlock 类进行说明。
// 说明了该类用于表示固定大小或动态大小的子向量表达式，介绍了它作为某些函数返回类型的情况、使用场景、给出了相关示例代码的位置，
// 以及提到了在特定条件下关于内存分配的特点，并通过 \sa 标签关联了相关的其他类和函数，方便查看参考。

namespace internal {
// 在 "Eigen" 命名空间内又定义了一个名为 "internal" 的内部命名空间，通常用于存放一些内部使用的结构体、类、函数等实现细节，对外部相对隐藏。
// 为 VectorBlock 类定义特征类，继承自 Block 的特征类，根据 VectorType 的行主序标志确定维度
template<typename VectorType, int Size>
struct traits<VectorBlock<VectorType, Size> >
  : public traits<Block<VectorType,
                     traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     traits<VectorType>::Flags & RowMajorBit? Size : 1> >
{
};
// 这是一个模板结构体（通常用于定义类型特征相关内容，类似 C++ 中的 traits 编程技巧），它针对 VectorBlock 类型进行特化定义。
// 通过继承 Block 类型的特征类，并根据传入的 VectorType 的一些标志位（这里是 RowMajorBit，可能用于表示是否按行主序存储等相关特性）来确定维度相关信息，
// 以此来描述 VectorBlock 类型在一些模板相关特性方面的表现。
}

template<typename VectorType, int Size> class VectorBlock
  : public Block<VectorType,
                     internal::traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     internal::traits<VectorType>::Flags & RowMajorBit? Size : 1>
{
    typedef Block<VectorType,
                     internal::traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     internal::traits<VectorType>::Flags & RowMajorBit? Size : 1> Base;
// 使用 typedef 为父类 Block（根据 VectorType 的相关标志位确定具体模板参数的 Block 类型）定义了一个别名 "Base"，方便后续代码中引用父类类型。
    enum {
      IsColVector =!(internal::traits<VectorType>::Flags & RowMajorBit)
// 定义了一个枚举类型的成员，用于判断是否为列向量。通过对 VectorType 的相关标志位（RowMajorBit）取反来确定，
    // 如果该标志位未被设置（通常意味着不是行主序存储，可能暗示是列主序存储，即列向量的一种常见表示方式），则认为是列向量。
    };
  public:
    EIGEN_DENSE_PUBLIC_INTERFACE(VectorBlock)
// 这里调用了某个（可能是 Eigen 库内部定义的）宏来设置 VectorBlock 类的公共接口相关特性，具体功能需要查看对应宏的定义，
    // 一般用于对外暴露类的一些必要的公共成员、函数等内容，使其能被外部正常使用。

    using Base::operator=;
 // 使用 "using" 声明，将父类（Base）的赋值运算符（operator=）引入到当前类的作用域中，使得当前类可以使用父类的赋值操作行为。

    /** 动态大小的构造函数
      */
    inline VectorBlock(VectorType& vector, Index start, Index size)
      : Base(vector,
             IsColVector? start : 0, IsColVector? 0 : start,
             IsColVector? size  : 1, IsColVector? 1 : size)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorBlock);
    }
// 定义了一个内联（inline）的构造函数，用于创建具有动态大小的 VectorBlock 对象。
    // 它接收一个 VectorType 类型的引用（表示从中获取子向量的原始向量对象）、一个起始索引（start）和子向量的大小（size）作为参数。
    // 通过初始化列表调用父类（Base）的构造函数，并根据当前对象是否为列向量（IsColVector）来确定传递给父类构造函数的相应索引和大小参数，
    // 最后通过 EIGEN_STATIC_ASSERT_VECTOR_ONLY 这个宏（应该是 Eigen 库自定义的用于静态断言的宏）来确保当前类型是向量类型（只用于向量相关操作）。

    /** 固定大小的构造函数
      */
    inline VectorBlock(VectorType& vector, Index start)
      : Base(vector, IsColVector? start : 0, IsColVector? 0 : start)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorBlock);
    }
// 定义了另一个内联（inline）的构造函数，用于创建具有固定大小的 VectorBlock 对象。
    // 接收一个 VectorType 类型的引用（原始向量对象）和一个起始索引（start）作为参数，同样通过初始化列表调用父类构造函数，
    // 根据是否为列向量来传递合适的索引参数，并且也使用了静态断言宏来确保类型是用于向量相关操作的。

};


/** \returns a dynamic-size expression of a segment (i.e. a vector block) in *this.
  *
  * 仅适用于向量
  *
  * \param start 段中的第一个系数
  * \param size 段中的系数数量
  *
  * 示例： \include MatrixBase_segment_int_int.cpp
  * Output: \verbinclude MatrixBase_segment_int_int.out
  *
  * 注意：即使返回的表达式具有动态大小，但在将其应用于固定大小的向量时，
  * 它继承了一个固定的最大大小，这意味着对其求值不会导致动态内存分配。
  *
  * \sa class Block, segment(Index)
  */
template<typename Derived>
inline typename DenseBase<Derived>::SegmentReturnType
DenseBase<Derived>::segment(Index start, Index size)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return SegmentReturnType(derived(), start, size);
}

/** 这是 segment(Index,Index) 的 const 版本。*/
template<typename Derived>
inline const typename DenseBase<Derived>::ConstSegmentReturnType
DenseBase<Derived>::segment(Index start, Index size) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return ConstSegmentReturnType(derived(), start, size);
}
// 这是一个模板函数，定义在 DenseBase 类（这里应该是通过模板参数 Derived 来适配不同的派生类情况）中。
// 函数返回类型是 DenseBase<Derived>::SegmentReturnType（根据不同的派生类 Derived，确定对应的表示子向量段返回类型的类型），
// 函数名为 segment，接收两个参数，分别表示子向量段的起始索引（start）和子向量段包含的系数数量（size）。
// 函数内部首先通过 EIGEN_STATIC_ASSERT_VECTOR_ONLY 宏进行静态断言，确保传入的类型 Derived 是用于向量相关操作的，
// 然后返回一个通过 SegmentReturnType 类型构造的对象（调用其构造函数并传入一些必要参数，这里的 derived() 应该是获取当前 DenseBase 派生类对象的函数，具体要看相关定义），
// 整体用于获取当前对象（*this）中指定起始和大小的动态大小的子向量段表达式，同时文档注释说明了相关的使用场景、示例以及内存分配特点，并关联了相关的其他类和函数。

/** \returns a dynamic-size expression of the first coefficients of *this.
  *
  * 仅适用于向量
  *
  * \param size 块中的系数数量
  *
  * 示例： \include MatrixBase_start_int.cpp
  * Output: \verbinclude MatrixBase_start_int.out
  *
  * 注意：即使返回的表达式具有动态大小，但在将其应用于固定大小的向量时，
  * 它继承了一个固定的最大大小，这意味着对其求值不会导致动态内存分配。
  *
  * \sa class Block, block(Index,Index)
  */
template<typename Derived>
inline typename DenseBase<Derived>::SegmentReturnType
DenseBase<Derived>::head(Index size)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return SegmentReturnType(derived(), 0, size);
}

/** 这是 head(Index) 的 const 版本。*/
template<typename Derived>
inline const typename DenseBase<Derived>::ConstSegmentReturnType
DenseBase<Derived>::head(Index size) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return ConstSegmentReturnType(derived(), 0, size);
}

/** \returns a dynamic-size expression of the last coefficients of *this.
  *
  * 仅适用于向量
  *
  * \param size 块中的系数数量
  *
  * 示例： \include MatrixBase_end_int.cpp
  * Output: \verbinclude MatrixBase_end_int.out
  *
  * 注意：即使返回的表达式具有动态大小，但在将其应用于固定大小的向量时，
  * 它继承了一个固定的最大大小，这意味着对其求值不会导致动态内存分配。
  *
  * \sa class Block, block(Index,Index)
  */
template<typename Derived>
inline typename DenseBase<Derived>::SegmentReturnType
DenseBase<Derived>::tail(Index size)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return SegmentReturnType(derived(), this->size() - size, size);
}

/** 这是 tail(Index) 的 const 版本。*/
template<typename Derived>
inline const typename DenseBase<Derived>::ConstSegmentReturnType
DenseBase<Derived>::tail(Index size) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return ConstSegmentReturnType(derived(), this->size() - size, size);
}

/** \returns a fixed-size expression of a segment (i.e. a vector block) in \c *this
  *
  * 仅适用于向量
  *
  * 模板参数 Size 是块中的系数数量
  *
  * \param start 子向量第一个元素的索引
  *
  * 示例： \include MatrixBase_template_int_segment.cpp
  * Output: \verbinclude MatrixBase_template_int_segment.out
  *
  * \sa class Block
  */
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template FixedSegmentReturnType<Size>::Type
DenseBase<Derived>::segment(Index start)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<Size>::Type(derived(), start);
}

/** 这是 segment<int>(Index) 的 const 版本。*/
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template ConstFixedSegmentReturnType<Size>::Type
DenseBase<Derived>::segment(Index start) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<Size>::Type(derived(), start);
}

/** \returns a fixed-size expression of the first coefficients of *this.
  *
  * 仅适用于向量
  *
  * 模板参数 Size 是块中的系数数量
  *
  * 示例： \include MatrixBase_template_int_start.cpp
  * Output: \verbinclude MatrixBase_template_int_start.out
  *
  * \sa class Block
  */
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template FixedSegmentReturnType<Size>::Type
DenseBase<Derived>::head()
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<Size>::Type(derived(), 0);
}

/** 这是 head<int>() 的 const 版本。*/
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template ConstFixedSegmentReturnType<Size>::Type
DenseBase<Derived>::head() const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<Size>::Type(derived(), 0);
}

/** \returns a fixed-size expression of the last coefficients of *this.
  *
  * 仅适用于向量
  *
  * 模板参数 Size 是块中的系数数量
  *
  * 示例： \include MatrixBase_template_int_end.cpp
  * Output: \verbinclude MatrixBase_template_int_end.out
  *
  * \sa class Block
  */
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template FixedSegmentReturnType<Size>::Type
DenseBase<Derived>::tail()
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<Size>::Type(derived(), size() - Size);
}

/** 这是 tail<int> 的 const 版本。*/
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template ConstFixedSegmentReturnType<Size>::Type
DenseBase<Derived>::tail() const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<Size>::Type(derived(), size() - Size);
}

} // end namespace Eigen

#ifndef EIGEN_VECTORBLOCK_H
#define EIGEN_VECTORBLOCK_H

namespace Eigen { 

/** \class VectorBlock
  * \ingroup Core_Module
  *
  * \brief Expression of a fixed-size or dynamic-size sub-vector
  *
  * \param VectorType the type of the object in which we are taking a sub-vector
  * \param Size size of the sub-vector we are taking at compile time (optional)
  *
  * 该类表示一个固定大小或动态大小的子向量的表达式。
  * 它是 DenseBase::segment(Index,Index) 和 DenseBase::segment<int>(Index) 的返回类型，
  * 并且大多数情况下仅通过这种方式使用。
  *
  * 然而，如果您想要直接操作子向量表达式，例如编写一个返回这样表达式的函数，
  * 则需要使用这个类。
  *
  * 以下是一个动态情况的示例：
  * \include class_VectorBlock.cpp
  * Output: \verbinclude class_VectorBlock.out
  *
  * 注意：即使此表达式具有动态大小，但在 VectorType 具有固定大小的情况下，
  * 此表达式继承了一个固定的最大大小，这意味着对其求值不会导致动态内存分配。
  *
  * 以下是一个固定大小情况的示例：
  * \include class_FixedVectorBlock.cpp
  * Output: \verbinclude class_FixedVectorBlock.out
  *
  * \sa class Block, DenseBase::segment(Index,Index,Index,Index), DenseBase::segment(Index,Index)
  */

namespace internal {
// 为 VectorBlock 类定义特征类，继承自 Block 的特征类，根据 VectorType 的行主序标志确定维度
template<typename VectorType, int Size>
struct traits<VectorBlock<VectorType, Size> >
  : public traits<Block<VectorType,
                     traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     traits<VectorType>::Flags & RowMajorBit? Size : 1> >
{
};
}

template<typename VectorType, int Size> class VectorBlock
  : public Block<VectorType,
                     internal::traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     internal::traits<VectorType>::Flags & RowMajorBit? Size : 1>
{
    typedef Block<VectorType,
                     internal::traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     internal::traits<VectorType>::Flags & RowMajorBit? Size : 1> Base;
    enum {
      IsColVector =!(internal::traits<VectorType>::Flags & RowMajorBit)
    };
  public:
    EIGEN_DENSE_PUBLIC_INTERFACE(VectorBlock)
// 这里调用了某个（可能是 Eigen 库内部定义的）宏来设置 VectorBlock 类的公共接口相关特性，具体功能需要查看对应宏的定义，
    // 一般用于对外暴露类的一些必要的公共成员、函数等内容，使其能被外部正常使用。

    using Base::operator=;  // 使用 "using" 声明，将父类（Base）的赋值运算符（operator=）引入到当前类的作用域中，使得当前类可以使用父类的赋值操作行为。

    /** 动态大小的构造函数
      */
    inline VectorBlock(VectorType& vector, Index start, Index size)
      : Base(vector,
             IsColVector? start : 0, IsColVector? 0 : start,
             IsColVector? size  : 1, IsColVector? 1 : size)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorBlock);
    }
// 定义了一个内联（inline）的构造函数，用于创建具有动态大小的 VectorBlock 对象。
    // 它接收一个 VectorType 类型的引用（表示从中获取子向量的原始向量对象）、一个起始索引（start）和子向量的大小（size）作为参数。
    // 通过初始化列表调用父类（Base）的构造函数，并根据当前对象是否为列向量（IsColVector）来确定传递给父类构造函数的相应索引和大小参数，
    // 最后通过 EIGEN_STATIC_ASSERT_VECTOR_ONLY 这个宏（应该是 Eigen 库自定义的用于静态断言的宏）来确保当前类型是向量类型（只用于向量相关操作）。

    /** 固定大小的构造函数
      */
    inline VectorBlock(VectorType& vector, Index start)
      : Base(vector, IsColVector? start : 0, IsColVector? 0 : start)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorBlock);
    }
// 定义了另一个内联（inline）的构造函数，用于创建具有固定大小的 VectorBlock 对象。
    // 接收一个 VectorType 类型的引用（原始向量对象）和一个起始索引（start）作为参数，同样通过初始化列表调用父类构造函数，
    // 根据是否为列向量来传递合适的索引参数，并且也使用了静态断言宏来确保类型是用于向量相关操作的。

};


/** \returns a dynamic-size expression of a segment (i.e. a vector block) in *this.
  *
  * 仅适用于向量
  *
  * \param start 段中的第一个系数
  * \param size 段中的系数数量
  *
  * 示例： \include MatrixBase_segment_int_int.cpp
  * Output: \verbinclude MatrixBase_segment_int_int.out
  *
  * 注意：即使返回的表达式具有动态大小，但在将其应用于固定大小的向量时，
  * 它继承了一个固定的最大大小，这意味着对其求值不会导致动态内存分配。
  *
  * \sa class Block, segment(Index)
  */
template<typename Derived>
inline typename DenseBase<Derived>::SegmentReturnType
DenseBase<Derived>::segment(Index start, Index size)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return SegmentReturnType(derived(), start, size);
}

/** 这是 segment(Index,Index) 的 const 版本。*/
template<typename Derived>
inline const typename DenseBase<Derived>::ConstSegmentReturnType
DenseBase<Derived>::segment(Index start, Index size) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return ConstSegmentReturnType(derived(), start, size);
}

/** \returns a dynamic-size expression of the first coefficients of *this.
  *
  * 仅适用于向量
  *
  * \param size 块中的系数数量
  *
  * 示例： \include MatrixBase_start_int.cpp
  * Output: \verbinclude MatrixBase_start_int.out
  *
  * 注意：即使返回的表达式具有动态大小，但在将其应用于固定大小的向量时，
  * 它继承了一个固定的最大大小，这意味着对其求值不会导致动态内存分配。
  *
  * \sa class Block, block(Index,Index)
  */
template<typename Derived>
inline typename DenseBase<Derived>::SegmentReturnType
DenseBase<Derived>::head(Index size)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return SegmentReturnType(derived(), 0, size);
}
// 这是一个模板函数，定义在 DenseBase 类（这里应该是通过模板参数 Derived 来适配不同的派生类情况）中。
// 函数返回类型是 DenseBase<Derived>::SegmentReturnType（根据不同的派生类 Derived，确定对应的表示子向量段返回类型的类型），
// 函数名为 segment，接收两个参数，分别表示子向量段的起始索引（start）和子向量段包含的系数数量（size）。
// 函数内部首先通过 EIGEN_STATIC_ASSERT_VECTOR_ONLY 宏进行静态断言，确保传入的类型 Derived 是用于向量相关操作的，
// 然后返回一个通过 SegmentReturnType 类型构造的对象（调用其构造函数并传入一些必要参数，这里的 derived() 应该是获取当前 DenseBase 派生类对象的函数，具体要看相关定义），
// 整体用于获取当前对象（*this）中指定起始和大小的动态大小的子向量段表达式，同时文档注释说明了相关的使用场景、示例以及内存分配特点，并关联了相关的其他类和函数。

/** 这是 head(Index) 的 const 版本。*/
template<typename Derived>
inline const typename DenseBase<Derived>::ConstSegmentReturnType
DenseBase<Derived>::head(Index size) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return ConstSegmentReturnType(derived(), 0, size);
}
// 这是上面 segment(Index, Index) 函数的 const 版本，意味着这个函数可以在 const 对象上调用（不会修改对象的状态）。
// 函数的返回类型是 const 修饰的 DenseBase<Derived>::ConstSegmentReturnType（表示常量的子向量段返回类型），
// 函数参数和基本逻辑与非 const 版本类似，同样进行静态断言确保类型是向量相关的，然后返回对应的常量子向量段表达式对象，用于获取常量对象中的子向量段。

/** \returns a dynamic-size expression of the last coefficients of *this.
  *
  * 仅适用于向量
  *
  * \param size 块中的系数数量
  *
  * 示例： \include MatrixBase_end_int.cpp
  * Output: \verbinclude MatrixBase_end_int.out
  *
  * 注意：即使返回的表达式具有动态大小，但在将其应用于固定大小的向量时，
  * 它继承了一个固定的最大大小，这意味着对其求值不会导致动态内存分配。
  *
  * \sa class Block, block(Index,Index)
  */
template<typename Derived>
inline typename DenseBase<Derived>::SegmentReturnType
DenseBase<Derived>::tail(Index size)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return SegmentReturnType(derived(), this->size() - size, size);
}
// 这是另一个模板函数，同样定义在 DenseBase 类中（通过模板参数 Derived 适配不同派生类情况），用于获取当前对象（*this）的开头部分的动态大小的子向量表达式。
// 返回类型是 DenseBase<Derived>::SegmentReturnType，接收一个参数 size，表示要获取的开头部分子向量的系数数量。
// 函数内部先进行静态断言确保类型用于向量操作，然后通过 SegmentReturnType 类型构造并返回对应的开头部分子向量表达式对象，
// 文档注释说明了适用范围、给出了示例以及提到了相关内存分配特性，并关联了相关的类和函数。


/** 这是 tail(Index) 的 const 版本。*/
template<typename Derived>
inline const typename DenseBase<Derived>::ConstSegmentReturnType
DenseBase<Derived>::tail(Index size) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return ConstSegmentReturnType(derived(), this->size() - size, size);
}
// 这是上面 head(Index) 函数的 const 版本，可在 const 对象上调用，返回类型是 const 修饰的 DenseBase<Derived>::ConstSegmentReturnType，
// 功能是获取 const 对象开头部分指定系数数量的动态大小的子向量表达式，内部也是先静态断言然后返回对应的常量子向量表达式对象。


/** \returns a fixed-size expression of a segment (i.e. a vector block) in \c *this
  *
  * 仅适用于向量
  *
  * 模板参数 Size 是块中的系数数量
  *
  * \param start 子向量第一个元素的索引
  *
  * 示例： \include MatrixBase_template_int_segment.cpp
  * Output: \verbinclude MatrixBase_template_int_segment.out
  *
  * \sa class Block
  */
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template FixedSegmentReturnType<Size>::Type
DenseBase<Derived>::segment(Index start)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<Size>::Type(derived(), start);
}
// 定义在 DenseBase 类中的模板函数，用于获取当前对象（*this）末尾部分的动态大小的子向量表达式。
// 返回类型是 DenseBase<Derived>::SegmentReturnType，接收参数 size 表示要获取的末尾部分子向量的系数数量。
// 函数内部先进行静态断言确保是向量相关类型，然后通过 SegmentReturnType 类型构造并返回对应末尾部分子向量表达式对象，
// 其计算子向量起始索引的方式是用当前对象的总大小（通过 this->size() 获取）减去要获取的子向量大小（size），文档注释同样说明了相关特性和关联信息。


/** 这是 segment<int>(Index) 的 const 版本。*/
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template ConstFixedSegmentReturnType<Size>::Type
DenseBase<Derived>::segment(Index start) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<Size>::Type(derived(), start);
}
// 这是 tail(Index) 函数的 const 版本，用于获取 const 对象末尾部分指定系数数量的动态大小的子向量表达式，
// 返回 const 修饰的相应返回类型对象，内部先静态断言然后按上述计算方式返回对应的常量子向量表达式对象。

/** \returns a fixed-size expression of the first coefficients of *this.
  *
  * 仅适用于向量
  *
  * 模板参数 Size 是块中的系数数量
  *
  * 示例： \include MatrixBase_template_int_start.cpp
  * Output: \verbinclude MatrixBase_template_int_start.out
  *
  * \sa class Block
  */
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template FixedSegmentReturnType<Size>::Type
DenseBase<Derived>::head()
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<Size>::Type(derived(), 0);
// 这是一个模板成员函数模板（template template member function，是一种比较复杂的 C++ 模板用法），定义在 DenseBase 类中。
// 用于获取当前对象（*this）中指定起始索引（start）的固定大小（由模板参数 Size 指定系数数量）的子向量表达式。
// 函数内部先通过静态断言确保类型是用于向量操作的，然后返回通过 FixedSegmentReturnType<Size>::Type 类型构造的对应固定大小子向量表达式对象，
// 文档注释说明了适用范围、参数含义以及给出了示例，并关联了相关的类。

}

/** 这是 head<int>() 的 const 版本。*/
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template ConstFixedSegmentReturnType<Size>::Type
DenseBase<Derived>::head() const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<Size>::Type(derived(), 0);
}
// 这是上面 segment 函数模板（针对固定大小且接收模板参数 Size 的情况）的 const 版本，用于获取 const 对象中指定起始索引的固定大小子向量的常量表达式，
// 返回 const 修饰的相应类型对象，内部同样先进行静态断言然后返回对应的常量子向量表达式对象。

/** \returns a fixed-size expression of the last coefficients of *this.
  *
  * 仅适用于向量
  *
  * 模板参数 Size 是块中的系数数量
  *
  * 示例： \include MatrixBase_template_int_end.cpp
  * Output: \verbinclude MatrixBase_template_int_end.out
  *
  * \sa class Block
  */
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template FixedSegmentReturnType<Size>::Type
DenseBase<Derived>::tail()
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<Size>::Type(derived(), size() - Size);
}

/** 这是 tail<int> 的 const 版本。*/
template<typename Derived>
template<int Size>
inline typename DenseBase<Derived>::template ConstFixedSegmentReturnType<Size>::Type
DenseBase<Derived>::tail() const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<Size>::Type(derived(), size() - Size);
}
// 这是一个模板成员函数模板，定义在 DenseBase 类中，用于获取当前对象（*this）开头部分固定大小（由模板参数 Size 指定系数数量）的子向量表达式。
// 函数先进行静态断言确保类型用于向量操作，

} // end namespace Eigen

#endif // EIGEN_VECTORBLOCK_H

