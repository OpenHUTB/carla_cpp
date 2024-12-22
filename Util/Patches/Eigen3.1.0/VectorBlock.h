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
// 这是一个预处理指令，用于防止头文件被重复包含。如果在编译过程中这个头文件还没有被定义（即第一次包含该头文件），
// 则后续的代码会被编译处理；如果已经被定义了（意味着已经包含过一次了），那么在这个位置到 #endif 之间的代码将被忽略，避免重复定义等问题
#define EIGEN_VECTORBLOCK_H
// 定义一个宏，通常用于标识这个头文件已经被包含过了，配合上面的#ifndef一起使用，实现头文件的防重复包含机制
namespace Eigen { 
// 开始Eigen命名空间，Eigen是一个用于线性代数运算的C++模板库，在这里定义的类、函数等都属于Eigen库的相关内容，方便对库内元素进行逻辑分组和避免名字冲突
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
// 在 Eigen 命名空间内再定义一个 internal 内部命名空间，通常用于存放一些内部使用的辅助类、结构体、函数等实现细节相关的代码，对外隐藏这些实现细节，使库的外部接口更简洁清晰
// 为 VectorBlock 类定义特征类，继承自 Block 的特征类，根据 VectorType 的行主序标志确定维度
template<typename VectorType, int Size>
struct traits<VectorBlock<VectorType, Size> >
  : public traits<Block<VectorType,
                     traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     traits<VectorType>::Flags & RowMajorBit? Size : 1> >
{
};
// 这个结构体（traits）是一种在模板编程中常用的技术，用于提取和封装类型的一些特性信息。这里它定义了 VectorBlock 类的特征类，
// 通过继承 Block 类的特征类（traits<Block<...>>）并根据 VectorType 的行主序标志（RowMajorBit）来确定维度相关的信息，
// 具体是根据标志位判断是按行主序还是列主序来确定子向量在整体向量中的维度表示方式（比如是当作一行还是一列等情况），以此来适配不同的存储和运算需求
}

template<typename VectorType, int Size> class VectorBlock
  : public Block<VectorType,
                     internal::traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     internal::traits<VectorType>::Flags & RowMajorBit? Size : 1>
{
    typedef Block<VectorType,
                     internal::traits<VectorType>::Flags & RowMajorBit? 1 : Size,
                     internal::traits<VectorType>::Flags & RowMajorBit? Size : 1> Base;
// 定义一个类型别名 Base，将 VectorBlock 类继承的基类（Block 类型）进行重命名，方便后续代码中使用这个别名来指代基类，使代码更简洁易读
    enum {
      IsColVector =!(internal::traits<VectorType>::Flags & RowMajorBit)
    };
// 定义一个枚举类型的成员，用于判断是否是列向量，通过对 VectorType 的行主序标志位（RowMajorBit）进行取反操作来确定，
    // 如果行主序标志位未被设置（即按列主序存储），则认为是列向量，反之则不是列向量，这对于后续在处理向量元素的顺序、维度相关操作时有重要作用
  public:
    EIGEN_DENSE_PUBLIC_INTERFACE(VectorBlock)//这是Eigen库中用于继承公共接口的宏，允许   VectorBlock   类使用   VectorType   的公共接口。
// 通过这个宏，使得 VectorBlock 类能够继承 VectorType 的公共接口，意味着可以像使用 VectorType 类型对象一样使用 VectorBlock 类对象，
    // 调用其公共的成员函数、访问公共成员变量等，方便统一接口风格和复用已有功能。
    using Base::operator=;//允许   VectorBlock   类使用其基类   Base   的赋值运算符。
// 这行代码使得 VectorBlock 类可以使用其基类（Base，也就是前面定义的 Block 类型）的赋值运算符（operator=），
    // 这样在对 VectorBlock 类对象进行赋值操作时，可以按照基类的赋值逻辑来进行，实现了代码复用并且保证了赋值操作在继承体系下的一致性。
    /** 动态大小的构造函数
      */
    inline VectorBlock(VectorType& vector, Index start, Index size)
      : Base(vector,
             IsColVector? start : 0, IsColVector? 0 : start,
             IsColVector? size  : 1, IsColVector? 1 : size)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorBlock);
    }
// 这是 VectorBlock 类的一个构造函数，用于创建动态大小的子向量对象。它接受三个参数，分别是要提取子向量的原始向量对象（vector）、
    // 子向量在原始向量中的起始索引（start）以及子向量的大小（size）。在函数体中，通过调用基类（Base）的构造函数来初始化对象，
    // 根据是否是列向量（IsColVector）来调整传递给基类构造函数的参数顺序和取值，以正确地定位和确定子向量在原始向量中的范围。
    // 最后通过 EIGEN_STATIC_ASSERT_VECTOR_ONLY 宏进行静态断言检查，确保当前操作的对象类型是向量类型（即符合 VectorBlock 类所要求的类型），
    // 如果不是向量类型，在编译阶段就会报错提示，保证代码的类型安全性。
    /** 固定大小的构造函数
      */
    inline VectorBlock(VectorType& vector, Index start)
      : Base(vector, IsColVector? start : 0, IsColVector? 0 : start)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorBlock);
    }
// 这是另一个构造函数，用于创建固定大小的子向量对象。它接受两个参数，即要提取子向量的原始向量对象（vector）和子向量在原始向量中的起始索引（start）。
    // 同样在函数体中调用基类（Base）的构造函数来初始化对象，根据是否是列向量（IsColVector）来传递合适的起始索引参数。
    // 最后也通过 EIGEN_STATIC_ASSERT_VECTOR_ONLY 宏进行静态断言检查，确保类型的正确性，避免非向量类型的误用。
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
// 这是一个函数模板定义，它是 DenseBase 类模板的一个成员函数（通过 template<typename Derived> 表示这是一个依赖于模板参数 Derived 的成员函数），
// 函数名为 segment，接受两个参数 start 和 size，分别表示子向量的起始索引和大小。函数的返回类型是 typename DenseBase<Derived>::SegmentReturnType，
// 即根据模板参数 Derived 所确定的 DenseBase 类中的 SegmentReturnType 类型（这通常也是一个与子向量相关的类型）。
// 在函数体中，首先通过 EIGEN_STATIC_ASSERT_VECTOR_ONLY 宏进行静态断言检查，确保 Derived 类型对应的对象是向量类型，保证类型的正确性。
// 然后通过调用 SegmentReturnType 类型的构造函数（这里假设它有合适的构造函数接受相应参数），传入当前对象（通过 derived() 获取，
// 具体实现应该在 DenseBase 类中定义如何获取当前对象）、起始索引（start）和子向量大小（size）参数，最终返回构建好的表示子向量的返回值。
/** 这是 segment(Index,Index) 的 const 版本。*/
template<typename Derived>
inline const typename DenseBase<Derived>::ConstSegmentReturnType
DenseBase<Derived>::segment(Index start, Index size) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return ConstSegmentReturnType(derived(), start, size);
}
// 这是上面 segment(Index,Index) 函数的 const 版本，用于处理常量对象（即不能被修改的对象）的情况。函数的参数和功能与非 const 版本类似，
// 不过返回类型变成了 const typename DenseBase<Derived>::ConstSegmentReturnType，表明返回的是一个常量的子向量表达式类型，
// 调用这个函数不会对原始对象进行修改，同样在函数体中进行了向量类型的静态断言检查，并通过调用相应构造函数返回子向量表达式。
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

#endif // EIGEN_VECTORBLOCK_H
