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

    using Base::operator=;

    /** 动态大小的构造函数
      */
    inline VectorBlock(VectorType& vector, Index start, Index size)
      : Base(vector,
             IsColVector? start : 0, IsColVector? 0 : start,
             IsColVector? size  : 1, IsColVector? 1 : size)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorBlock);
    }

    /** 固定大小的构造函数
      */
    inline VectorBlock(VectorType& vector, Index start)
      : Base(vector, IsColVector? start : 0, IsColVector? 0 : start)
    {
      EIGEN_STATIC_ASSERT_VECTOR_ONLY(VectorBlock);
    }
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
