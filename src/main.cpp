#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <bitset>
#include <vector>

#include "statistics.hpp"
#include "types.hpp"

using namespace std;

Header fillHeaderValues(vector<int>header_bytes)
{
  Header TS_Header;
  TS_Header.sync_byte = header_bytes[0];
  //in next byte, more than 1 field, we want part of byte 1 and
  //the whole byte 2 for PID parsing
  unsigned i;
  bitset<13>pid_bitrepr;
  bitset<2>scrambled;
  bitset<8>aux_byte(header_bytes[1]);
  bitset<8>aux_byte2(header_bytes[2]);
  bitset<8>aux_byte3(header_bytes[3]);
  for (i = 0; i < BYTE_SIZE; ++i)
    {
      pid_bitrepr[i] = aux_byte2[i];
    }
  for (i = BYTE_SIZE; i < 13; ++i)
    {
      pid_bitrepr[i] = aux_byte[i - BYTE_SIZE];
    }
  //Now translate pid bitset to ulong
  TS_Header.PID = pid_bitrepr.to_ulong();
  //Next, remanining byte
  scrambled[0] = aux_byte3[BYTE_SIZE - 2];
  scrambled[1] = aux_byte3[BYTE_SIZE - 1];
  //Now translate scrambled to dec
  if (scrambled.to_ulong() == 0)
    TS_Header.scrambled = false;
  else
    TS_Header.scrambled = true;
  
}

vector<int> getNextHeaderBytes(FILE *file, int position)
{
  vector<int>values;
  int nr;
  //set the current stream pointer to the specified position
  fseek (file, position, SEEK_SET);
  for (unsigned i = 0; i < HEADER_BYTES; ++i)
    {   
      fread(&nr,1,1,file);
      values.push_back(nr);
    }
  return values;
}

int main(int argc, char **argv)
{
  /************Variable definition**************/
  ofstream headers("/home/eiger824/Programming/Files/logs/headers.log");
  ofstream to_file("/home/eiger824/Programming/Files/logs/bytes.log");
  FILE *file = fopen(argv[1],"rb");
  bool firstTime = true;
  vector<int>header;
  short PID; //2-byte = 16bits, only need 13
  bool scrambled;
  uint nr_sync_errors;
  TYPE type;
  Header header_struct;
  Statistics watchdog;
  /*********************************************/
  if (file == NULL) perror ("Error opening file");
  else
    {
      int pos = ftell(file);
      int len = fseek(file, 0, SEEK_END);
      int length = ftell(file);
      watchdog.setGlobalByteNumber(length);
      cout << "\nInitial byte pos: " << pos <<
	", File length (bytes): " << length << "\n";
      cout << "Extracting byte stream...\n";
      int c;
      //reset current stream pointer to first position
      fseek(file, 0, SEEK_SET);
      //check if files are open
      if(to_file.is_open() &&
	 headers.is_open())
	{
	  cout << "Files open and ready to be overwritten...\n";
	  to_file << "-Hex-\t" << "-Dec-\t" << "-Binary-" << endl;
	  headers << "-Hex-\t" << "-Binary-\n";
	  while(!feof(file))
	    {
	      //read 1 byte from file and store it to c
	      fread(&c,1,1,file);
	      //sync byte, we will read just 4-byte headers
	      if (c == 71)
		{
		  //packet start with sync byte -> add up packet count
		  watchdog.addUpGlobalPacketCounter();
		  to_file << "Header starts\n";
		  headers << "Header starts\n--------------------\n";
		  
		  //Fetch the 4-byte header
		  if (firstTime)
		      header = getNextHeaderBytes(file,0);
		  else
		      header = getNextHeaderBytes(file,ftell(file)-1);

		  firstTime = false;
		  //process info of the parsed header
		  //1.)create struct out of parsed header bytes
		  header_struct = fillHeaderValues(header);
		  if (!watchdog.isPIDregistered(header_struct.PID))
		    {
		      watchdog.registerNewPID(header_struct.PID);  
		    }
		  //add up pid count
		  watchdog.addUpPidCount(header_struct.PID);
		  //check scramble and store in watchdog
		  if (header_struct.scrambled)
		    {
		      watchdog.addUpScrambleCount(header_struct.PID);
		    }
		  //********************************
		  for (unsigned i = 0; i < HEADER_BYTES; ++i)
		    {
		      bitset<8>bit_rep(header[i]);
		      headers << "0x" << hex << header[i] << "\t" << bit_rep << "\n";
		    }
		  headers << "--------------------\n";
		  
		}
	      bitset<8>bitrep(c);
	      to_file << "0x" << hex << c << "\t" << dec << c  << "\t" << bitrep << endl;
	    }

	  
	  to_file.close();
	  headers.close();
	}
    }
  cout << "Extracted!!\n";
  watchdog.showStatistics();
}
