static const int PACKET_SIZE = 188;
static const int HEADER_BYTES = 4;
static const int BYTE_SIZE = 8;

enum TYPE
  {
    PSI,
    PES
  };
enum PID_TYPE
  {
    PAT, //0
    CAT, //1
    TSDT, //2
    IPMP, //3
    FUT, //4-15, future implementation
    DVB_META, //16-31
    PMT, //32-8186 && 8188-8190
    MGT_META, //8187
    NULLP //8191
  };
struct Header
{
  uint sync_byte;
  short PID;
  bool scrambled;
  //TYPE type;
};
