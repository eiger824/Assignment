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
AdaptationFieldWrapper fetchAdaptationData(vector<uint8_t>data);
vector<uint8_t> getNextHeaderBytes(FILE *file, int position);
vector<uint8_t> getAdaptationFieldBytes(FILE *file, int position);
bool isContCounterError(int glob_cnt, int parsed_cnt, unsigned int flag);
bool checkDistance(unsigned int d);
void displayHelp();
float computeBitrate(long PCR_i_1, unsigned int index_i_1, long PCR_i, unsigned int index_i);

//main
int main(int argc, char **argv)
{
  /************Variable definition**************/
  ofstream to_file(raw_path);
  FILE *file;
  bool firstTime = true;
  bool a = true;
  vector<uint8_t>header;
  vector<uint8_t>adaptation;
  unsigned int nr_sync_errors;
  unsigned int sync_error_counter;
  int len, length;
  Header header_struct;
  AdaptationFieldWrapper adaptation_struct;
  uint8_t c;
  //object that will perform statistics on our input stream
  Statistics watchdog;
  /*********************************************/
  if (argc == 1)
    {
      fprintf (stderr, "Error: Not enough input arguments.\n");
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
	      i+=2;
	    }
	  else if (!strcmp(argv[i],"-d") || !strcmp(argv[i], "--debug"))
	    {
	      watchdog.setDebugMode(true);
	      ++i;
	    }
	  else
	    {
	      fprintf (stderr, "Error: Invalid option.\n");
	      displayHelp();
	      return -1;
	    }
	}
    }  
  if (file == NULL)
    {
      fprintf (stderr, "Error: bad address or inexistent file.\n");
      return -1;
    }
  else
    {
      len = fseek(file, 0, SEEK_END);
      length = ftell(file);
      watchdog.setGlobalByteNumber(length);
      watchdog.notify("File length (bytes): " + to_string(length) + ')');
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
		  watchdog.notify("New sync byte detected.");
		  watchdog.notify("Checking sync error...)");
		  //check "distance" between two consecutive sync bytes
		  if (!firstTime && !checkDistance(sync_error_counter))
		    {
		      watchdog.addUpSyncErrorCount();
		      watchdog.notify("Checking sync error...[FAIL]");
		    }
		  firstTime = false;
		  watchdog.notify("Checking sync error...[OK]");
		  //reset sync error counter
		  sync_error_counter = 0;
		  
		  watchdog.notify("Adding up global packet counter...");
		  //packet start with sync byte -> add up packet count
		  watchdog.addUpGlobalPacketCounter();
		  watchdog.notify("Adding up global packet counter...[OK]");
		  
		  to_file << "Header starts\n";
		  
		  //Fetch the 4-byte header
		  watchdog.notify("Fetching 4-byte header...");
		  header = getNextHeaderBytes(file,ftell(file)-1);	  
		  watchdog.notify("Fetching 4-byte header...[OK]");
		  
		  watchdog.notify("Parsed PID (decimal): " + to_string(header_struct.PID));
		  watchdog.notify("Processing parsed header...");
		  
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
		  //4.)if payload flag is set, add up payloaded packet counter
		  if (header_struct.payload_flag == 1)
		    {
		      watchdog.addUpPayloadedPacketCount(header_struct.PID);
		    }
		  //5.)check if parsed counter is correct
		  if (isContCounterError(watchdog.getPayloadedPacketCount(header_struct.PID) - 1,
					 header_struct.cont_counter,
					 header_struct.payload_flag))
		    {
		      watchdog.addUpContCounterError(header_struct.PID);
		    }
		  //6.)check if adaptation field flag is set
		  if (header_struct.adaptation_field_flag == 1)
		    {
		      //get pcr flag and PCR value
		      adaptation = getAdaptationFieldBytes(file,ftell(file) + 1);
		      adaptation_struct = fetchAdaptationData(adaptation);
		      //register last read position (byte count)
		      watchdog.setGlobalByteCount(ftell(file) - 1);		      
		      //register values in watchdog
		      if (watchdog.noneRegistered(header_struct.PID))
			{
			  watchdog.registerPair(watchdog.getByteCount(), 
						adaptation_struct.PCR,
						header_struct.PID,
						true);
			}
			else
			  {
			    if (!watchdog.areBothRegistered(header_struct.PID))
			      {
				watchdog.registerPair(watchdog.getByteCount(), 
						      adaptation_struct.PCR,
						      header_struct.PID,
						      false);
			      }
			    else
			      { //both registered, calculate bitrate
			
				vector<unsigned int>indexes = watchdog.getIndexes(header_struct.PID);
				vector<unsigned long>values = watchdog.getValues(header_struct.PID);
				cout << indexes[0] << "," << indexes[1] << "," << values[0] << "," << values[1] << endl;
				float bitrate = computeBitrate(values[0],
							       indexes[0],
							       values[1],
							       indexes[1]);
				/*cout << bitrate << endl;
				watchdog.setBitrate(bitrate, header_struct.PID);*/
			      }
			  }
		    }
		  watchdog.notify("Processing parsed header...[OK]");
		}
	      else
		{
		  //increase packet "distance" counter (sync errors)
		  ++sync_error_counter;
		  watchdog.notify("Increased sync error count: " + to_string(sync_error_counter));
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

//function definitions
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
  bitset<1>adaptation_field_flag_bitrepr;
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
  //remaining byte: adaptation field flag
  adaptation_field_flag_bitrepr[0] = aux_byte3[5];
  //Now translate scrambled & cont_counter & payload & adaptation flag to dec
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
  TS_Header.adaptation_field_flag = adaptation_field_flag_bitrepr.to_ulong();
  //return assembled struct
  return TS_Header;  
}

AdaptationFieldWrapper fetchAdaptationData(vector<uint8_t>data)
{
  AdaptationFieldWrapper TS_AdaptationField;
  unsigned i;
  bitset<1>PCR_flag_bitrepr;
  bitset<8>byte0(data[0]);

  bitset<48>PCR_field_bitrepr;
  bitset<8>byte1(data[1]);
  bitset<8>byte2(data[2]);
  bitset<8>byte3(data[3]);
  bitset<8>byte4(data[4]);
  bitset<8>byte5(data[5]);
  bitset<8>byte6(data[6]);
  
  for (i = 0; i < 6 * BYTE_SIZE; ++i)
    {
      if (i < 8)
	PCR_field_bitrepr[i] = byte6[i % BYTE_SIZE];
      else if (8 <= i && 16 > i)
	PCR_field_bitrepr[i] = byte5[i % BYTE_SIZE];
      else if (16 <= i && 24 > i)
	PCR_field_bitrepr[i] = byte4[i % BYTE_SIZE];
      else if (24 <= i && 32 > i)
	PCR_field_bitrepr[i] = byte3[i % BYTE_SIZE];
      else if (32 <= i && 40 > i)
	PCR_field_bitrepr[i] = byte2[i % BYTE_SIZE];
      else if (40 <= i && 48 > i)
	PCR_field_bitrepr[i] = byte1[i % BYTE_SIZE];
    }
  bitset<33>PCR_base_repr;
  bitset<9>PCR_ext_repr;
  for (int j = 6 * BYTE_SIZE - 1; j >= 0; --j)
    {
      if (j > 14) //base
	{
	 PCR_base_repr[j - 15] = PCR_field_bitrepr[j];
	}
      else if (j < 9) //ext
	{
	  PCR_ext_repr[j] = PCR_field_bitrepr[j];
	}
    }
    
  PCR_flag_bitrepr[0] = byte0[4];
  TS_AdaptationField.PCR_flag = PCR_flag_bitrepr.to_ulong();
    
  if (TS_AdaptationField.PCR_flag == 0)
    {
      //set the PCR value to 0 if flag was not set
      TS_AdaptationField.PCR = 0;
    }
  else
    {
      //according to ISO-13818-1
      TS_AdaptationField.PCR = PCR_base_repr.to_ullong() * 300 + PCR_ext_repr.to_ulong();
    }
  return TS_AdaptationField;
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

vector<uint8_t> getAdaptationFieldBytes(FILE *file, int position)
{
  vector<uint8_t>values;
  uint8_t nr;
  //set the current stream pointer to the specified position
  fseek (file, position, SEEK_SET);
  //we want to read 8 bytes of the adaptation field
  for (unsigned i = 0; i < BYTE_SIZE - 1; ++i)
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

float computeBitrate(long PCR_i_1, unsigned int index_i_1, long PCR_i, unsigned int index_i)
{
  //according to ISO-13818-1 (bits / s)
  return BYTE_SIZE * ((index_i - index_i_1) * TS_CLOCK_FREQ) / (PCR_i - PCR_i_1);
}

void displayHelp()
{
  cout << "USAGE:\n";
  cout << "./TS_Analyzer -f <path-to-filename>\n";
  cout << "(**HINT: if debug flat is set, consider pipe-ing the output (>) to a file)\n";
  cout << "-d, --debug Print exhaustive information\n";
  cout << "-f, --file <path-to-filename> Input file to parse\n";
  cout << "-h, --help Print this help\n";
}
