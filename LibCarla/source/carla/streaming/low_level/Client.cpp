// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/tcp/Client.h"

#include <boost/asio/io_context.hpp>

#include <memory>	// ����C++��׼����ڴ����ͷ�ļ�
#include <unordered_map>	// ����C++��׼�������ӳ������ͷ�ļ�

namespace carla {
namespace streaming {
namespace low_level {

  /// A client able to subscribe to multiple streams. Accepts an external
  /// io_context.
  ///
  /// @warning The client should not be destroyed before the @a io_context is
  /// stopped.
  template <typename T>
  class Client {
  public:
  	
    using underlying_client = T;
    using protocol_type = typename underlying_client::protocol_type;
    using token_type = carla::streaming::detail::token_type;	// ʹ�����ͱ������򻯴����ж�ģ�����T��������͵�����

    explicit Client(boost::asio::ip::address fallback_address)
      : _fallback_address(std::move(fallback_address)) {}	// ���캯��������һ��boost::asio::ip::address���͵ı��õ�ַ��Ϊ����
	
    explicit Client(const std::string &fallback_address)
      : Client(carla::streaming::make_address(fallback_address)) {}		// ���캯��������һ���ַ�����ʽ�ı��õ�ַ��Ϊ���������Ƚ���ת��Ϊboost::asio::ip::address���ͣ��ٵ�����һ�����캯�����г�ʼ��
	
    explicit Client()
      : Client(carla::streaming::make_localhost_address()) {}	// Ĭ�Ϲ��캯�������Ȼ�ȡ����������ַ��Ϊ���õ�ַ���ٵ�����һ�����캯�����г�ʼ��
	
    ~Client() {
      for (auto &pair : _clients) {
        pair.second->Stop();		// ���������������ڶ�������ʱ������Դ������������Ѷ��ĵĿͻ��˲��������ǵ�ֹͣ����
      }
    }	

    /// @warning cannot subscribe twice to the same stream (even if it's a
    /// MultiStream).
    template <typename Functor>
    void Subscribe(
        boost::asio::io_context &io_context,
        token_type token,	// �������ķ���������io_context�������Լ��ص�������Ϊ����
        Functor &&callback) {
      DEBUG_ASSERT_EQ(_clients.find(token.get_stream_id()), _clients.end());	// ����ȷ����ǰҪ���ĵ���ID���Ѷ��Ŀͻ��˵�ӳ�������в����ڣ����������ζ���ͬһ����
      if (!token.has_address()) {
        token.set_address(_fallback_address);	// ������������û�е�ַ���ͽ����õ�ַ���ø�����
      }
      auto client = std::make_shared<underlying_client>(	// ����һ���ײ�ͻ��˵�����ָ�룬��ͨ���ײ�ͻ��˵Ĺ��캯�����г�ʼ��������io_context�����ƺͻص�����
          io_context,
          token,
          std::forward<Functor>(callback));
      client->Connect();	// �ÿͻ��˳������ӵ���Ӧ����
      _clients.emplace(token.get_stream_id(), std::move(client));	// �������õĿͻ�������ָ������IDΪ�����뵽_clientsӳ�������У��Ա��������Ͳ���
    }

    void UnSubscribe(token_type token) {	// ȡ���������ķ���������һ��������Ϊ����
      log_debug("calling sensor UnSubscribe()");	// ���һ��������Ϣ����ʾ���ڵ���ȡ�����Ĳ���
      auto it = _clients.find(token.get_stream_id());	// ���Ѷ��Ŀͻ��˵�ӳ�������в����봫�����Ƶ���ID��Ӧ�Ŀͻ���ָ��
      if (it != _clients.end()) {	// ����ҵ��˶�Ӧ�Ŀͻ���ָ�룬�͵�������ֹͣ����ֹͣ��ز���������ӳ��������ɾ���ÿͻ��˵ļ�¼
        it->second->Stop();
        _clients.erase(it);
      }
    }

  private:	

    boost::asio::ip::address _fallback_address;	// �洢���õ�IP��ַ���ڹ��캯���н��г�ʼ���������������ӳ���������Ҫʹ�ñ��õ�ַʱ��������

    std::unordered_map<	// һ������ӳ���������洢�ײ�ͻ��˵�����ָ�룬���ڹ���Ͳ������ĵĸ�������Ӧ�Ŀͻ���
        detail::stream_id_type,
        std::shared_ptr<underlying_client>> _clients;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
