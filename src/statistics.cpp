//system headers
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <fstream>

//project headers
#include "statistics.hpp"

namespace assignment {

  Statistics::Statistics(): m_nr_sync_errors(0),
			    m_global_TS_packet_counter(0),
			    m_nr_bytestream(0),
			    m_sync_errors(0)
		
  {
  }
  Statistics::~Statistics()
  {
  }
  void Statistics::addUpSyncErrorCount()
  {
    ++m_sync_errors;
  }
  string Statistics::getPIDType(int pid)
  {
    if (pid == 0) return "PAT";
    else if (pid == 1) return "CAT";
    else if (pid == 2) return "TSDT";
    else if (pid == 3) return "IPMP";
    else if (pid >= 4 && pid <= 15) return "FUT";
    else if (pid >= 16 && pid <= 31) return "DVB_META";
    else if (pid >= 32 && pid <= 8186 || pid >= 8188 && pid <= 8190) return "PMT";
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
		int temp_pidpc = m_payloaded_packets[i];

		m_pid_list[i] = m_pid_list[j];
		m_pid_counters[i] = m_pid_counters[j];
		m_cont_counters[i] = m_cont_counters[j];
		m_payloaded_packets[i] = m_payloaded_packets[j];

		m_pid_list[j] = temp_pid;
		m_pid_counters[j] = temp_pidc;
		m_cont_counters[j] = temp_pidcc;
		m_payloaded_packets[j] = temp_pidpc;
	      }
	  }
      }
  }
  void Statistics::printSingleLine(int index)
  {
    float aux; //to avoid printing /0 divisions
    if (m_payloaded_packets[index] == 0)
      {
	aux = 0;
      }
    else
      {
	aux = ((float) 100* m_cont_counters[index]) / m_payloaded_packets[index];
      }
    cout << "0x" << hex << m_pid_list[index] << "\t" <<
      setw(10) << getPIDType(m_pid_list[index]) << "\t" <<
      setw(10) << dec << m_pid_counters[index] << " (" << setprecision(2) << fixed << (float) 100 * m_pid_counters[index] / m_global_TS_packet_counter << "%)\t" <<
      setw(10) << dec << m_scrambles[index]  << " (" << setprecision(2) << fixed << (float) 100 * m_scrambles[index] / m_pid_counters[index] << "%)\t" <<
      setw(10) << m_cont_counters[index] << " (" << setprecision(2) << fixed << setprecision(2) << fixed << aux << "%)" << endl;
  }
  void Statistics::showStatistics()
  {
    //print out all or most common packet ids
    char opt, save;
    bool correct = true;
    while (correct)
      {
	printf("Print [a]ll (%d) PIDs or [t]op 20 most-common?[a/t]: ",m_pid_list.size());
	scanf(" %c",&opt);
	switch(opt)
	  {
	  case 'a':
	    cout << "Total number of detected TS packets: " << m_global_TS_packet_counter << endl;
	    cout << "Total number of different PIDs: " << m_pid_list.size() << endl;
	    cout << "Number of encountered sync errors: " << m_sync_errors << endl;
	    cout << setw(0) << "PID" << setw(15) << "Type" << setw(20) << "Count(%)" << setw(27) << "Scrambled(%)" << setw(25) << "Cont.Errors(%)\n";
	    for (unsigned i = 0; i < m_pid_list.size(); ++i)
	      {
		printSingleLine(i);
	      }
	    correct = false;
	    break;
	  case 't':
	    cout << "Total number of detected TS packets: " << m_global_TS_packet_counter << endl;
	    cout << "Total number of different PIDs: " << m_pid_list.size() << endl;
	    cout << "Number of encountered sync errors: " << m_sync_errors << endl;
	    cout << setw(0) << "PID" << setw(15) << "Type" << setw(20) << "Count(%)" << setw(27) << "Scrambled(%)" << setw(25) << "Cont.Errors(%)\n";
	    sortPIDList();
	    for (unsigned i = 0; i < 20; ++i)
	      {
		printSingleLine(i);
	      }
	    correct = false;
	    break;
	  default:
	    cout << "Invalid option.\n";
	    break;
	  }
      }
    correct = true;
    while (correct)
      {
	printf("Save to file?[y/n]: ");
	scanf(" %c",&save);
	switch(save)
	  {
	  case 'y':
	    saveOutputToFile(opt);
	    cout << "File saved to " << stat_path << ".Exiting...\n";
	    correct = false;
	    break;
	  case 'n':
	    cout << "Exiting...\n";
	    correct = false;
	    break;
	  default:
	    cout << "Invalid option.\n";
	    break;
	  }
      }
  }
  void Statistics::saveOutputToFile(char opt)
  {
    std::ofstream output(stat_path);
    if (output.is_open())
      {
	output << "Total number of detected TS packets: " << m_global_TS_packet_counter << endl;
	output << "Total number of different PIDs: " << m_pid_list.size() << endl;
	output << "Number of encountered sync errors: " << m_sync_errors << endl;
	output << setw(0) << "PID" << setw(15) << "Type" << setw(20) << "Count(%)" << setw(27) << "Scrambled(%)" << setw(25) << "Cont.Errors(%)\n";
	float aux;
	switch (opt)
	  {
	  case 'a':
	    for (unsigned index = 0; index < m_pid_list.size(); ++index)
	      {
		if (m_payloaded_packets[index] == 0)
		  {
		    aux = 0;
		  }
		else
		  {
		    aux = ((float) 100* m_cont_counters[index]) / m_payloaded_packets[index];
		  }
		output << "0x" << hex << m_pid_list[index] << "\t" <<
		  setw(10) << getPIDType(m_pid_list[index]) << "\t" <<
		  setw(10) << dec << m_pid_counters[index] << " (" << setprecision(2) << fixed << (float) 100 * m_pid_counters[index] / m_global_TS_packet_counter << "%)\t" <<
		  setw(10) << dec << m_scrambles[index] << " (" << setprecision(2) << fixed << (float) 100 * m_scrambles[index] / m_pid_counters[index] << "%)\t" <<
		  setw(10) << m_cont_counters[index] << "(" << setprecision(2) << fixed << aux << "%)" << endl;
	      }
	    break;
	  case 't':
	    for (unsigned index = 0; index < 20; ++index)
	      {
		if (m_payloaded_packets[index] == 0)
		  {
		    aux = 0;
		  }
		else
		  {
		    aux = ((float) 100* m_cont_counters[index]) / m_payloaded_packets[index];
		  }
		output << "0x" << hex << m_pid_list[index] << "\t" <<
		  setw(10) << getPIDType(m_pid_list[index]) << "\t" <<
		  setw(10) << dec << m_pid_counters[index] << " (" << setprecision(2) << fixed << (float) 100 * m_pid_counters[index] / m_global_TS_packet_counter << "%)\t" <<
		  setw(10) << dec << m_scrambles[index] << " (" << setprecision(2) << fixed << (float) 100 * m_scrambles[index] / m_pid_counters[index] << "%)\t" <<
		  setw(10) << m_cont_counters[index] << "(" << setprecision(2) << fixed << aux << "%)" << endl;
	      }
	    break;
	  default:
	    std::cout << "Unknown option. Returning...\n";
	    return;
	  }
	output.close();
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
  int Statistics::getPayloadedPacketCount(int pid)
  {
    return m_payloaded_packets[getPIDindex(pid)];
  }
  void Statistics::registerNewPID(int pid)
  {
    m_pid_list.push_back(pid);
    m_pid_counters.push_back(0);
    m_scrambles.push_back(0);
    m_cont_counters.push_back(0);
    m_payloaded_packets.push_back(0);
  }
  int Statistics::getPIDindex(int pid)
  {
    for (unsigned i = 0; i < m_pid_list.size(); ++i)
      {
	if (m_pid_list[i] == pid)
	  {
	    return i;
	  }
      }
    return -1;
  }
  void Statistics::addUpScrambleCount(int pid)
  {
    ++m_scrambles[getPIDindex(pid)];
  }
  void Statistics::addUpPidCount(int pid)
  {
    ++m_pid_counters[getPIDindex(pid)];
  }
  void Statistics::addUpGlobalPacketCounter()
  {
    ++m_global_TS_packet_counter;
  }
  int Statistics::getGlobalPacketCounter()
  {
    return m_global_TS_packet_counter;
  }
  void Statistics::addUpPayloadedPacketCount(int pid)
  {
    ++m_payloaded_packets[getPIDindex(pid)];
  }
  void Statistics::addUpContCounterError(int pid)
  {
    ++m_cont_counters[getPIDindex(pid)];
  }
  int Statistics::getPidCounter(int pid)
  {
    return m_pid_counters[getPIDindex(pid)];
  }

}
