static const int PACKET_SIZE = 188;
static const int HEADER_BYTES = 4;
static const int BYTE_SIZE = 8;

enum TYPE
  {
    PSI,
    PES
  };

struct Header
{
  uint sync_byte;
  short PID;
  bool scrambled;
  TYPE type;
};
