// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // ֻ    һ θ ͷ ļ   ָ 

#include "carla/image/ImageView.h" //      ImageView ͷ ļ   

namespace carla {  //      carla      ռ䡣
namespace image {  //      image      ռ䡣

  class ImageConverter {  //     ͼ  ת     ࡣ
  public:   //    з      η   

      // ģ 庯       ڸ      ء 
    template <typename SrcViewT, typename DstViewT>
    static void CopyPixels(const SrcViewT &src, DstViewT &dst) {
      boost::gil::copy_pixels(src, dst); // ʹ   boost::gil  ⸴  Դ  ͼ     ص Ŀ    ͼ  
    }

     // ģ 庯       ھ͵ ת  ͼ  
    template <typename ColorConverter, typename MutableImageView>
    static void ConvertInPlace(
        MutableImageView &image_view, //  ɱ ͼ    ͼ   á 
        ColorConverter converter = ColorConverter()) { //   ɫת      Ĭ  ֵΪ ColorConverter()  
      using DstPixelT = typename MutableImageView::value_type;  //   ȡ ɱ ͼ    ͼ         ͡ 
      CopyPixels(  //      CopyPixels       
          ImageView::MakeColorConvertedView<MutableImageView, DstPixelT>(image_view, converter),
          image_view);  //   ת       ͼ   ƻ ԭʼͼ    ͼ  
    }
  };

} // namespace image
} // namespace carla
