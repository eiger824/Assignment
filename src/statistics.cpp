#include <iostream>

#include "statistics.hpp"

Statistics::Statistics(): m_nr_sync_errors(0),
			  m_global_TS_packet_counter(0),
			  m_nr_bytestream(0)
		
{
}
Statistics::~Statistics()
{
}
void Statistics::showStatistics()
{
  //print out info
  cout << "--PID--\t" << "Count(%)\t" << "Scrambled(%)\t" << "Cont.Errors(%)\n";
  cout << "(m_pid_list.size() = " << m_pid_list.size() << ")\n";
  for (unsigned i = 0; i < m_pid_list.size(); ++i)
    {
      cout << "0x" << hex << m_pid_list[i] << "\t" <<
	dec << m_pid_counters[i] << "(" << (float) 100 * m_pid_counters[i] / m_global_TS_packet_counter << ")\t" <<
	dec << m_scrambles[i] << "(" << (float) 100 * m_scrambles[i] / m_pid_counters[i] << ")\t" <<
	endl;
	
    }
}
void Statistics::setGlobalByteNumber(int nr)
{
  m_nr_bytestream = nr;
}
bool Statistics::isPIDregistered(int pid)
{
  cout << "list size is : " << m_pid_list.size() << "\n";
  for (unsigned i = 0; i < m_pid_list.size(); ++i)
    {
      if (m_pid_list[i] == pid)
	{
	  cout << "Pid " << pid << " is registered!";
	  return true;
	}
    }
  return false;
}
void Statistics::registerNewPID(int pid)
{
  cout << "Registering new pid: " << pid << endl;
  m_pid_list.push_back(pid);
  m_pid_counters.push_back(0);
  m_scrambles.push_back(0);
}
int Statistics::getPIDindex(int pid)
{
  for (unsigned i = 0; i < m_pid_list.size(); ++i)
    {
      if (m_pid_list[i] == pid)
	{
	  cout << "Returning index: " << i << endl;
	  return i;
	}
    }
  return -1;
}
void Statistics::addUpScrambleCount(int pid)
{
  ++m_scrambles[getPIDindex(pid)];
  cout << "Scramble count: " << m_scrambles[getPIDindex(pid)] << endl;
}
void Statistics::addUpPidCount(int pid)
{
  ++m_pid_counters[getPIDindex(pid)];
  cout << "Counter of pid " << pid << ": " << m_pid_counters[getPIDindex(pid)];
}
void Statistics::addUpGlobalPacketCounter()
{
  ++m_global_TS_packet_counter;
  cout << "Global TS packet count: " << m_global_TS_packet_counter << endl;
}
int Statistics::getGlobalPacketCounter()
{
  return m_global_TS_packet_counter;
}
