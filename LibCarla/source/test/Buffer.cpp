// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Buffer.h"

#include <boost/random/independent_bits.hpp>

#include <climits>
#include <random>
// ���������ռ� util�����а����������ռ� buffer
namespace util {
namespace buffer {
// ����ָ����С�����������
  shared_buffer make_random(size_t size) {
  	// �������Ĵ�СΪ 0���򷵻�һ���յĻ�����
    if (size == 0u)
      return make_empty();
        // ʹ�� boost::random::independent_bits_engine �� std::random_device ��������ֽ�����
        // independent_bits_engine ��һ����������棬��ʹ�� std::random_device ��Ϊ�������Դ
        // ����ָ��λ���������� CHAR_BIT��ͨ���� 8 λ���Ķ������λ���������޷����ַ����͵������
    using random_bytes_engine = boost::random::independent_bits_engine<
        std::random_device,
        CHAR_BIT,
        unsigned char>;
    random_bytes_engine rbe;
    auto buffer = make_empty(size);
    std::generate(buffer->begin(), buffer->end(), std::ref(rbe));
    return buffer;
  }

  std::string to_hex_string(const Buffer &buf, size_t length) {
    length = std::min(static_cast<size_t>(buf.size()), length);
        // ����һ���㹻����ַ��������洢ʮ�������ַ���
        // ����Ϊ 2u * length + 1u����Ϊÿ���ֽ���Ҫ������ʮ���������ֱ�ʾ���ټ���һ���ַ���������
    auto buffer = std::make_unique<char[]>(2u * length + 1u);
    for (auto i = 0u; i < length; ++i)
      sprintf(&buffer[2u * i], "%02x", buf.data()[i]);
    if (length < buf.size())
      return std::string(buffer.get()) + std::string("...");
    return std::string(buffer.get());
  }

} // namespace buffer
} // namespace util
