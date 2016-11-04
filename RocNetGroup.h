#ifndef ROCNET_GROUPS_H
#define ROCNET_GROUPS_H

enum class RocNetGroup: uint8_t {
  host = 0, 
  command_station = 1, 
  mobile_decoders = 2, 
  stationary_decoders = 3, 
  programming_mobile = 4, 
  programming_stationary = 5,
  gp_data = 6, 
  clock = 7, 
  sensor = 8, 
  output = 9, 
  input = 10, 
  sound = 11, 
  display = 12
};

#endif
