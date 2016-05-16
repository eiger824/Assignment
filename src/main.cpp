/*
MPEG-2 Transport Stream Analyzer
Author: Santiago Pagola
Date: May 16th 2016
*/

//system headers
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <cstdlib>
#include <bitset>
#include <vector>
#include <cstdint>
#include <cstring>

//project headers
#include "statistics.hpp"

using namespace std;
using namespace assignment;

//Function declarations
Header fillHeaderValues(vector<uint8_t>header_bytes);
vector<uint8_t> getNextHeaderBytes(FILE *file, int position);
bool isContCounterError(int glob_cnt, int parsed_cnt, unsigned int flag);
bool checkDistance(unsigned int d);
void displayHelp();

//main
int main(int argc, char **argv)
{
  /************Variable definition**************/
  ofstream to_file("../logs/raw_bytes.log");
  FILE *file;
  bool firstTime = true;
  vector<uint8_t>header;
  unsigned int nr_sync_errors;
  unsigned int sync_error_counter;
  int len, length;
  Header header_struct;
  uint8_t c;
  //object that will perform statistics on our input stream
  Statistics watchdog;
  /*********************************************/
  if (argc == 1)
    {
      displayHelp();
      return -1;
    }
  else
    {
      unsigned i = 1;
      while (i < argc)
	{
	  if (!strcmp(argv[i],"-h") || !strcmp(argv[i], "--help"))
	    {
	      displayHelp();
	      return -1;
	    }
	  else if (!strcmp(argv[i],"-f") || !strcmp(argv[i], "--file"))
	    {
	      file = fopen(argv[i+1],"rb");
	      break;
	    }
	  else
	    {
	      perror ("Error: Invalid option.");
	      displayHelp();
	      return -1;
	    }
	  ++i;
	}
    }
  
  if (file == NULL)
    {
      perror ("Error opening file");
      return -1;
    }
  else
    {
      len = fseek(file, 0, SEEK_END);
      length = ftell(file);
      watchdog.setGlobalByteNumber(length);
      cout << "File length (bytes): " << length << "\n";
      sync_error_counter = 0;
      //reset current stream pointer to first position
      fseek(file, 0, SEEK_SET);
      //check if files are open
      if (to_file.is_open())
	{
	  cout << "File open and ready to be (over)written...\n";
	  cout << "Extracting byte stream...\n";
	  to_file << "-Hex-\t" << "-Dec-\t" << "-Binary-" << endl;
	  while (!feof(file))
	    {
	      //read 1 byte from file and store it to c
	      fread(&c,1,1,file);
	      //sync byte, we will read just 4-byte headers
	      if (c == 71)
		{
		  //check "distance" between two consecutive sync bytes
		  if (!firstTime && !checkDistance(sync_error_counter))
		    {
		      watchdog.addUpSyncErrorCount();
		    }
		  //reset sync error counter
		  sync_error_counter = 0;
		  //packet start with sync byte -> add up packet count
		  watchdog.addUpGlobalPacketCounter();
		  to_file << "Header starts\n";
		  //Fetch the 4-byte header
		  if (firstTime)
		    {
		      header = getNextHeaderBytes(file,0);
		    }
		  else
		    {
		      header = getNextHeaderBytes(file,ftell(file)-1);
		    }
		  firstTime = false;
		  //process info of the parsed header
		  //1.)create struct out of parsed header bytes
		  header_struct = fillHeaderValues(header);
		  //2.) if parsed pid is not registered, do so and add up its counter
		  if (!watchdog.isPIDregistered(header_struct.PID))
		    {
		      watchdog.registerNewPID(header_struct.PID);  
		    }
		  watchdog.addUpPidCount(header_struct.PID);
		  //3.)check scramble and store in watchdog
		  if (header_struct.scrambled)
		    {
		      watchdog.addUpScrambleCount(header_struct.PID);
		    }
		  //if payload flag is set, add up payloaded packet counter
		  if (header_struct.payload_flag == 1)
		    {
		      watchdog.addUpPayloadedPacketCount(header_struct.PID);
		    }
		  //check if parsed counter is correct
		  if (isContCounterError(watchdog.getPayloadedPacketCount(header_struct.PID) - 1,
					 header_struct.cont_counter,
					 header_struct.payload_flag))
		    {
		      watchdog.addUpContCounterError(header_struct.PID);
		    }		  
		}
	      else
		{
		  //increase packet "distance" counter (sync errors)
		  ++sync_error_counter;
		}
	      bitset<8>bitrep(c);
	      to_file << "0x" << hex << (unsigned int)c << "\t" << dec << (unsigned int)c  << "\t" << bitrep << endl;
	    }
	  to_file.close();
	}
    }
  cout << "Extracted!!\n";
  watchdog.showStatistics();
}

Header fillHeaderValues(vector<uint8_t>header_bytes)
{
  Header TS_Header;
  TS_Header.sync_byte = header_bytes[0];
  //in next byte, more than 1 field, we want part of byte 1 and
  //the whole byte 2 for PID parsing
  unsigned i;
  bitset<13>pid_bitrepr;
  bitset<2>scrambled_bitrepr;
  bitset<4>cont_counter_bitrepr;
  bitset<1>payload_flag_bitrepr;
  bitset<8>aux_byte1(header_bytes[1]);
  bitset<8>aux_byte2(header_bytes[2]);
  bitset<8>aux_byte3(header_bytes[3]);
  for (i = 0; i < BYTE_SIZE; ++i)
    {
      pid_bitrepr[i] = aux_byte2[i];
    }
  for (i = BYTE_SIZE; i < 13; ++i)
    {
      pid_bitrepr[i] = aux_byte1[i - BYTE_SIZE];
    }
  //Now translate pid bitset to ulong
  TS_Header.PID = pid_bitrepr.to_ulong();
  //cout << "Parsed PID: 0x" << hex << TS_Header.PID << endl;
  //remanining byte: scrambling
  scrambled_bitrepr[0] = aux_byte3[BYTE_SIZE - 2];
  scrambled_bitrepr[1] = aux_byte3[BYTE_SIZE - 1];
  //remaning byte: cont_counter
  for (i = 0; i < BYTE_SIZE / 2; ++i)
    {
      cont_counter_bitrepr[i] = aux_byte3[i];
    }
  //remaining byte: payload flag
  payload_flag_bitrepr[0] = aux_byte3[4];
  //Now translate scrambled & cont_counter & payload flag to dec
  if (scrambled_bitrepr.to_ulong() == 0)
    {
      TS_Header.scrambled = false;
    }
  else
    {
      TS_Header.scrambled = true;
    }
  TS_Header.cont_counter = cont_counter_bitrepr.to_ulong();
  TS_Header.payload_flag = payload_flag_bitrepr.to_ulong();
  //return assembled struct
  return TS_Header;  
}

vector<uint8_t> getNextHeaderBytes(FILE *file, int position)
{
  vector<uint8_t>values;
  uint8_t nr;
  //set the current stream pointer to the specified position
  fseek (file, position, SEEK_SET);
  for (unsigned i = 0; i < HEADER_BYTES; ++i)
    {   
      fread(&nr,1,1,file);
      values.push_back(nr);
    }
  return values;
}

bool isContCounterError(int glob_cnt, int parsed_cnt, unsigned int flag)
{
  if (flag == 1 &&
      ((glob_cnt % CONT_COUNTER_MAX) != parsed_cnt))
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool checkDistance(unsigned int d)
{
  //Assuming a constant & fixed TS packet size of 188 bytes
  if (d != PACKET_SIZE)
    {
      return false;
    }
  else
    {
      return true;
    }
}

void displayHelp()
{
  cout << "USAGE:\n";
  cout << "./TS_Analyzer -f <filename>\n";
  cout << "-f, --file <filename> Input file to parse\n";
  cout << "-h, --help Print this help\n";
}
