namespace assignment {

  static const int PACKET_SIZE = 188;
  static const int HEADER_BYTES = 4;
  static const int BYTE_SIZE = 8;
  static const int CONT_COUNTER_MAX = 16;
  static const int TS_CLOCK_FREQ = 27000000;
  static const char* stat_path = "../logs/statistics.log";
  static const char* raw_path = "../logs/raw_bytes.log";

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
    unsigned int sync_byte;
    short PID; //2-byte = 16bits, only need 13
    bool scrambled;
    uint cont_counter;
    unsigned short payload_flag;
    unsigned short adaptation_field_flag;
  };
  struct AdaptationFieldWrapper
  {
    unsigned short PCR_flag;
    unsigned long long PCR;
  };
}
