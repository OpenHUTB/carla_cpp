// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>

#pragma pack(push, 1)
struct CarlaRecorderAnimWalker
{
  uint32_t DatabaseId;
  float Speed;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);

};
#pragma pack(pop)
//test
class CarlaRecorderAnimWalkers
{
public:

  void Add(const CarlaRecorderAnimWalker &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

  void Read(std::istream &InFile);

  const std::vector<CarlaRecorderAnimWalker>& GetWalkers();
  
private:

  std::vector<CarlaRecorderAnimWalker> Walkers;
};
