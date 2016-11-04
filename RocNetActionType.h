#ifndef ROCNET_ACTION_TYPE_H
#define ROCNET_ACTION_TYPE_H

enum class RocNetActionType: uint8_t {
  request = 0,
  event = 1,
  reply = 2
};

#endif
