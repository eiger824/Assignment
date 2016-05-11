//system headers
#include <vector>
#include <cstdlib>
#include <string>

//project headers
#include "types.hpp"

using namespace std;
class Statistics
{
public:
  Statistics();
  ~Statistics();
  void showStatistics();
  bool isPIDregistered(int pid);
  void registerNewPID(int pid);
  void addUpPidCount(int pid);
  void addUpScrambleCount(int pid);
  void addUpGlobalPacketCounter();
  void addUpContCounterError(int pid);
  int getGlobalPacketCounter();
  void setGlobalByteNumber(int nr);
private:
  void sortPIDList();
  int getPIDindex(int pid);
  string getPIDType(int pid);
  void printSingleLine(int index);
private:
  uint m_nr_sync_errors;
  vector<int>m_pid_list;
  vector<int>m_pid_counters;
  uint m_global_TS_packet_counter;
  vector<int>m_scrambles;
  int m_nr_bytestream;
  vector<int>m_cont_counters;
};
