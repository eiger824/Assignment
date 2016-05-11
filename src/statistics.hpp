#include <vector>
#include <cstdlib>

using namespace std;
class Statistics
{
public:
  Statistics();
  ~Statistics();
  void showStatistics();
  bool isPIDregistered(int pid);
  void registerNewPID(int pid);
  int getPIDindex(int pid);
  void addUpPidCount(int pid);
  void addUpScrambleCount(int pid);
  void addUpGlobalPacketCounter();
  int getGlobalPacketCounter();
  void setGlobalByteNumber(int nr);

private:
  uint m_nr_sync_errors;
  vector<int>m_pid_list;
  vector<int>m_pid_counters;
  uint m_global_TS_packet_counter;
  vector<int>m_scrambles;
  int m_nr_bytestream;
  
};
