//system headers
#include <iostream>
#include <stdio.h>
#include <iomanip>

//project headers
#include "statistics.hpp"

Statistics::Statistics(): m_nr_sync_errors(0),
			  m_global_TS_packet_counter(0),
			  m_nr_bytestream(0)
		
{
}
Statistics::~Statistics()
{
}
string Statistics::getPIDType(int pid)
{
  if (pid == 0) return "PAT";
  else if (pid == 1) return "CAT";
  else if (pid == 2) return "TSDT";
  else if (pid == 3) return "IPMP";
  else if (pid >= 4 && pid <= 15) return "FUT";
  else if (pid >= 16 && pid <= 31) return "DVB_META";
  else if (pid >= 32 && pid <= 8186 || pid >= 8188 && pid >= 8190) return "PMT";
  else if (pid == 8187) return "MGT_META";
  else if (pid == 8191) return "NULLP";
}
void Statistics::sortPIDList()
{
  unsigned i,j; 
  for (i = 0; i < m_pid_list.size(); ++i)
    {
      for (j = i; j < m_pid_list.size(); ++j)
	{
	  if (m_pid_counters[i] < m_pid_counters[j])
	    {
	      int temp_pid = m_pid_list[i];
	      int temp_pidc = m_pid_counters[i];
	      int temp_pidcc = m_cont_counters[i];
	      m_pid_list[i] = m_pid_list[j];
	      m_pid_counters[i] = m_pid_counters[j];
	      m_cont_counters[i] = m_cont_counters[j];
	      m_pid_list[j] = temp_pid;
	      m_pid_counters[j] = temp_pidc;
	      m_cont_counters[j] = temp_pidcc;
	    }
	}
    }
}
void Statistics::printSingleLine(int index)
{
  cout << "0x" << hex << m_pid_list[index] << "\t" <<
    setw(10) << getPIDType(m_pid_list[index]) << "\t" <<
    setw(10) << dec << m_pid_counters[index] << " (" << (float) 100 * m_pid_counters[index] / m_global_TS_packet_counter << "%)\t" <<
    setw(10) << dec << m_scrambles[index] << " (" << (float) 100 * m_scrambles[index] / m_pid_counters[index] << "%)\t" <<
    setw(10) << m_cont_counters[index] << "(" << (float) 100* m_cont_counters[index] / m_global_TS_packet_counter << "%)" << endl;
}
void Statistics::showStatistics()
{
  //print out all or most common packet ids
  char opt;
  printf("Print [a]ll PIDs or [t]op 20 most-common?[a/t]: ");
  scanf("%c",&opt);
  cout << "Total number of detected TS packets: " << m_global_TS_packet_counter << endl;
  cout << setw(0) << "PID" << setw(15) << "Type" << setw(20) << "Count(%)" << setw(27) << "Scrambled(%)" << setw(20) << "Cont.Errors(%)\n";
  switch(opt)
    {
    case 'a':
      for (unsigned i = 0; i < m_pid_list.size(); ++i)
	{
	  printSingleLine(i);
	}
      break;
    case 't':
      sortPIDList();
      for (unsigned i = 0; i < 20; ++i)
	{
	  printSingleLine(i);
	}  
      break;
    default:
      cout << "Bad option. Returning...\n";
      return;
    }    	
}
void Statistics::setGlobalByteNumber(int nr)
{
  m_nr_bytestream = nr;
}
bool Statistics::isPIDregistered(int pid)
{
  for (unsigned i = 0; i < m_pid_list.size(); ++i)
    {
      if (m_pid_list[i] == pid)
	{
	  return true;
	}
    }
  return false;
}
void Statistics::registerNewPID(int pid)
{
  m_pid_list.push_back(pid);
  m_pid_counters.push_back(0);
  m_scrambles.push_back(0);
  m_cont_counters.push_back(0);
}
int Statistics::getPIDindex(int pid)
{
  for (unsigned i = 0; i < m_pid_list.size(); ++i)
    {
      if (m_pid_list[i] == pid)
	{
	  //cout << "Returning index: " << i << endl;
	  return i;
	}
    }
  return -1;
}
void Statistics::addUpScrambleCount(int pid)
{
  ++m_scrambles[getPIDindex(pid)];
  //cout << "Scramble count: " << m_scrambles[getPIDindex(pid)] << endl;
}
void Statistics::addUpPidCount(int pid)
{
  ++m_pid_counters[getPIDindex(pid)];
  //cout << "Counter of pid " << pid << ": " << m_pid_counters[getPIDindex(pid)];
}
void Statistics::addUpGlobalPacketCounter()
{
  ++m_global_TS_packet_counter;
  //cout << "Global TS packet count: " << dec << m_global_TS_packet_counter << endl;
}
int Statistics::getGlobalPacketCounter()
{
  return m_global_TS_packet_counter;
}
void Statistics::addUpContCounterError(int pid)
{
  ++m_cont_counters[getPIDindex(pid)];
}
