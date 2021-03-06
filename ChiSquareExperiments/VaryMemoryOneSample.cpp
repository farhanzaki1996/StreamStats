// VaryMemoryOneSample.cpp
// Experiments that vary in memory percent

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <sys/time.h>
#include <cmath>
#include "DataGenerator.cpp"
#include "../SourceCode/ChiSquare/ChiSquareContinuous.cpp"
using namespace std;

double get_estimate(ChiSquareContinuous *quantile_sketch, char distribution_type, int num_buckets, double location, double scale);
int get_DOF(char distribution_type);
void name_file(char *str, char *argv[], int extra);

// Runs the experiments that vary the percent of memory
// Input: takes 9 command line arguments
// Output: creates 5 files, the log file holds all the data generated, the 
// table file holds the pvalue errors deliminated by tabs, the extra file 
// holds all the calculated statistics, the pvalue file holds all the pvalues
// calculated, and the actualerror file holds the actual and estimated 
// statistics deliminated by tabs
int main(int argc, char* argv[])
{
  if (argc < 10)
  {
    cout << "usage: VaryMemory lower-memory upper-memory num-streams(>0) [N|U|P|E] location scale [0|1] stream-size num-buckets" << endl;
    throw ParameterError();
  }
  
  double lower = atof (argv[1]);
  double upper = atof (argv[2]);
  int data_repeats = atoi (argv[3]);
  char distribution_type = argv[4][0];
  double location = atof (argv[5]);
  double scale = atof (argv[6]);
  int all_quantiles = atoi (argv[7]);
  int stream_size = atoi (argv[8]);
  int num_buckets = atoi(argv[9]);
  int seed = 1; 

  // ensures that the parameters will not create error
  if (data_repeats <= 0)
  {
    cout << "The number of streams must be greater than zero." << endl;
    throw ParameterError();
  }
  if (distribution_type != 'N' && distribution_type != 'U' && distribution_type != 'P' && distribution_type != 'E')
  {
    cout << "The distribution is either N, U, P, or E." << endl;
    throw ParameterError();
  }
  if (lower <= 0)
  {
    cout << "The lower memory percent must be greater than 0." << endl;
    throw ParameterError();
  }
  if (stream_size <= 0)
  {
    cout << "The stream_size must be greater than 0.\n";
    throw ParameterError();
  }
  if (num_buckets <= 0)
  {
    cout << "The number of bins must be greater than 0.\n";
    throw ParameterError();
  }

  // finds the number of different sizes the experiment will run on
  double memory_percent;
  int num_mems = 0;
  double mem = lower;
  while (mem <= (upper + 0.0000001))
  {
    //cout << mem << endl;
    num_mems++;
    mem *= sqrt(10);
    //cout << mem << " " << upper << endl;
  }

  // creates the arrays that will hold the calculated statistics
  double actual_values[data_repeats][num_mems];
  double GK_values[data_repeats][num_mems];
  double QD_values[data_repeats][num_mems];
  double RS_values[data_repeats][num_mems];
  double percents[num_mems];
  long times[data_repeats][num_mems];
  long times2[data_repeats][num_mems];

  // creates and initializes the log file
  ofstream data_file;
  char str[150];
  name_file(str, argv, 0);
  data_file.open(str);
  data_file << "Distribution: " << distribution_type << ", location = " << location << ", scale = " << scale << endl;

  for (int i = 0; i < data_repeats; i++) // runs all tests data_repeats times
  {
    memory_percent = lower;
    int j = 0;
    data_file << "Stream " << i+1 << ":" << endl;

    // generates the data based on inputed parameters                                       
    DataGenerator data(distribution_type, stream_size, seed, location, scale);
    double *stream = data.get_stream();

    while (memory_percent <= (upper+0.0000001))  // runs tests for every memory percent
    {
      int sample_size = memory_percent * stream_size;
      if (i == 0)
        percents[j] = memory_percent;
      data_file << "memory_percent = " << memory_percent << endl;
      
      // computes GK estimate
      ChiSquareContinuous sketch1(sample_size, 1);
 
	  timeval timeBefore, timeAfter; // initializes variables
  	  long diffSeconds, diffUSeconds;
 	  gettimeofday(&timeBefore, NULL); 

      for (int i = 0; i < stream_size; i++)
	      sketch1.insert(stream[i]);

	  gettimeofday(&timeAfter, NULL); // get time for insertion
  	  diffSeconds = timeAfter.tv_sec - timeBefore.tv_sec;
  	  diffUSeconds = timeAfter.tv_usec - timeBefore.tv_usec;

	  times[i][j] = diffSeconds;
	  times2[i][j] = diffUSeconds;

      double GK_stat = get_estimate(&sketch1, distribution_type, num_buckets, location, scale);
      GK_values[i][j] = GK_stat;
      data_file << "GK = " << GK_stat << endl;

      if (all_quantiles)
      {
      	// computes QDigest estimate
	ChiSquareContinuous sketch2(sample_size, 2);
	for (int i = 0; i < stream_size; i++)
          sketch2.insert(stream[i]);
	double QD_stat = get_estimate(&sketch2, distribution_type, num_buckets, location, scale);
	QD_values[i][j] = QD_stat;
        data_file << "QDigest = " << QD_stat << endl;

	// computes ReservoirSampling estimate
	ChiSquareContinuous sketch3(sample_size, 3);
	for (int i = 0; i < stream_size; i++)
	  sketch3.insert(stream[i]);
	double RS_stat = get_estimate(&sketch3, distribution_type, num_buckets, location, scale);
	RS_values[i][j] = RS_stat;
        data_file << "Reservoir Sampling = " << RS_stat << endl;
      }

      // computes actual statistic
      double *upper_interval = sketch1.get_upper();
      double *lower_interval = sketch1.get_lower();
      double actual = data.get_stat_one_sample(num_buckets, upper_interval, lower_interval);
      actual_values[i][j] = actual;
      data_file << "Real = " << actual << endl;

      memory_percent *= sqrt(10);
      j++;
    }
    seed++; // changes seed for next repeat
  }
  data_file.close();

  // creates table file
  name_file(str, argv, 1);
  data_file.open(str);

  // creates time table file
  ofstream time_file;
  char str2[150];
  name_file(str2, argv, 5);
  time_file.open(str2);

  // creates pvalues file
  ofstream data2_file;
  char st[150];
  name_file(st, argv, 3);
  data2_file.open(st);

  int deg_freedom = num_buckets - get_DOF(distribution_type);
  for (int i = 0; i < num_mems; i++)
  {
    data_file << percents[i] * 100 << "\t";
	time_file << percents[i] * 100 << "\t";

    // adds pvalue error from the GK sketch to the table file
    double error = 0;
    for (int j = 0; j < data_repeats; j++)
      error += abs(pochisq(GK_values[j][i], deg_freedom) - pochisq(actual_values[j][i], deg_freedom));
    data_file << error / data_repeats;

	// adds times to the time table
	long double avg_time = 0.0;
	for (int j = 0; j < data_repeats; j++)
		avg_time += times[j][i] + times2[j][i]/1000000.0;// diffSeconds + diffUSeconds/1000000.0
	time_file << avg_time / data_repeats << endl;

    if (all_quantiles)
    {
      // adds pvalue error from the QDigest sketch to the table file
      error = 0;
      for (int j = 0; j < data_repeats; j++)
	error += abs(pochisq(QD_values[j][i], deg_freedom) - pochisq(actual_values[j][i], deg_freedom));
      data_file << "\t" << error / data_repeats << "\t";

      // adds pvalue error from the Reservoir Sampling sketch to the table file
      error = 0;

      for (int j = 0; j < data_repeats; j++)
	error += abs(pochisq(RS_values[j][i], deg_freedom) - pochisq(actual_values[j][i], deg_freedom));
      data_file << error / data_repeats;
    }
    data_file << endl;

    // adds all pvalues to the pvalue file
    data2_file << "memory_percent = " << percents[i] << endl;
    for (int j = 0; j < data_repeats; j++)
    {
      data2_file << pochisq(actual_values[j][i], deg_freedom) << " actual" << endl;
      data2_file << pochisq(GK_values[j][i], deg_freedom) << " GK" << endl;
      if (all_quantiles)
      {
	data2_file << pochisq(QD_values[j][i], deg_freedom) << " QD" << endl;
	data2_file << pochisq(RS_values[j][i], deg_freedom) << " RS" << endl;
      }
    }
  }
  data2_file.close();
  data_file.close();
  time_file.close();

  // Creates extra file
  name_file(str, argv, 2);
  data_file.open(str);

  // adds actual statistics to the extra file
  for (int i = 0; i < num_mems; i++)
  {
    for (int j = 0; j < data_repeats; j++)
	    data_file << actual_values[j][i] << "\t";
  }
  data_file << endl;

  // adds GK statistics to the extra file
  for (int i = 0; i < num_mems; i++)
  {
    for(int j = 0; j <data_repeats; j++)
      data_file << GK_values[j][i] << "\t";
  }
  data_file << endl;
  if (all_quantiles)
  {
    // adds QDigest statistics to the extra file
    for (int i = 0; i < num_mems; i++)
    {
	    for(int j = 0; j <data_repeats; j++)
	      data_file << QD_values[j][i] << "\t";
    }
    data_file << endl;

    // adds Reservoir Sampling statistcs to the extra file
    for (int i = 0; i < num_mems; i++)
    {
	    for(int j = 0; j <data_repeats; j++)
	     data_file << RS_values[j][i] << "\t";
    }
    data_file << endl;
  }
  data_file.close();
  return 0;
}

// Returns the estimated chi square statistic based on the type of distribution
double get_estimate(ChiSquareContinuous *quantile_sketch, char distribution_type, int num_buckets, double location, double scale)
{
  if (distribution_type == 'N')
    return quantile_sketch->calculate_statistic_ifNormal(num_buckets, location, scale);
  else if (distribution_type == 'U')
    return quantile_sketch->calculate_statistic_ifUniform(num_buckets, location, scale);
  else if (distribution_type == 'P')
    return quantile_sketch->calculate_statistic_ifPareto(num_buckets, location, scale);
  else
    return quantile_sketch->calculate_statistic_ifExponential(num_buckets, location, scale);
}

// Returns the degree of freedom based on the type of distribution
int get_DOF(char distribution_type)
{
  if (distribution_type == 'E')
    return 2;
  else
    return 3;
}

// Creates a string that will be the name of one of the files using the command
// line arguments and the current day and time
// Input: argv has 9 parameters, extra is from 0 to 4
// Output: when extra is 0 i creates the name for the log file, when 1 creates
// name for table file, when 2 creates name for extra file, when 3 creates name
// for pvalues file, when 4 creates name for actual-error file 
void name_file(char *str, char *argv[], int extra)
{
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%m-%d-%Y.%X", &tstruct);

  strcpy(str, "VaryM1_");
  strcat(str, argv[1]);
  strcat(str, "-");
  strcat(str, argv[2]);
  strcat(str, "_");
  strcat(str, argv[3]);
  strcat(str, "X_");
  strcat(str, argv[4]);
  strcat(str, "-");
  strcat(str, argv[5]);
  strcat(str, "-");
  strcat(str, argv[6]);
  strcat(str, "_");
  strcat(str, argv[8]);
  strcat(str, "_");
  strcat(str, argv[9]);
  strcat(str, "_");
  if (atoi(argv[7]))
    strcat(str, "all_");
  else
    strcat(str, "GK_");
  strcat(str, buf);
  if (extra == 0)
    strcat(str, "_log.dat");
  else if (extra == 1)
    strcat(str, "_table.dat");
  else if (extra == 2)
    strcat(str, "_extra.dat");
  else if (extra == 3)
    strcat(str, "_pvalues.dat");
  else if (extra == 4)
    strcat(str, "_actual-values.dat");
  else 
	strcat(str, "_times.dat");
}
// VaryMemoryOneSample.cpp
