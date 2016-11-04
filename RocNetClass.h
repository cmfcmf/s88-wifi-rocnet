#ifndef ROCNET_CLASS
#define ROCNET_CLASS

enum class RocNetClass: uint8_t {
  accessory = 1,
  digital_railsync_generator = 2, // This is untested. http://wiki.rocrail.net/doku.php?id=rocnet:rocnetnode-setup-en
  identification = 3  // This is untested.
};

#endif
