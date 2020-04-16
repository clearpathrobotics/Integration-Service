/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef SOSS_WEBSOCKET__MSGPACKSERIALIZER_HPP
#define SOSS_WEBSOCKET__MSGPACKSERIALIZER_HPP

#include "websocket_types.hpp"
#include "MsgpackTypes.hpp"
#include <soss/json/json.hpp>

namespace soss {
namespace websocket {

class MsgpackSerializer {
public:

  static constexpr websocketpp::frame::opcode::value opcode = websocketpp::frame::opcode::value::BINARY;

  static MessagePtrT serialize(ConMsgManagerPtrT& con_msg_mgr, const MsgpackMessage& msg);
  static MsgpackMessage deserialize(const std::string& data);
};

} // namespace websocket
} // namespace soss

#endif //SOSS_WEBSOCKET__MSGPACKSERIALIZER_HPP