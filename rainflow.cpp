
#include <stdlib.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <sys/timeb.h>
#include <sys/types.h>
#include <time.h>

#define MAX 51000000

using namespace std;

typedef struct Cycle {
  float peak1;
  float peak2;
  float size;
  float amplitude;
} Cycle;

class Rainflow {
private:
  float C[100];
  float AverageMean[100];
  float MaxPeak[100];
  float MinValley[100];

  float MaxAmp[100];
  float AverageAmp[100];

  double L[20];

  float t;
  float scale;

  long ijk;

  int ic, iscale;

  long N;

  FILE *pFile[6];
  char filename[6][FILENAME_MAX];

  vector<float> y;

  void find_peaks(vector<float> * peaks);
  void find_cycles(vector<float> &peaks, vector<Cycle> * cycles);
public:
  void read_data(const char * inf);
  void calculate(void);
  void print_data(const char * outf);
};

int main(int argc, char * argv[]) {
  char * outf;
  if (argc == 1) {
    printf("Usage: rainflow <input file> [<output file>]\n");
    exit(0);
  } else if (argc == 2) {
    outf = (char *)"rainflow.out";
  } else {
    outf = argv[2];
  }
  char * inf = argv[1];

  Rainflow rf;

  printf("rainflow.cpp  ver 2.8  June 24, 2014 \n");
  printf("by Tom Irvine  Email: tom@vibrationdata.com \n");
  printf("ASTM E 1049-85 (2005) Rainflow Counting Method \n");

  time_t start = time(0);

  rf.read_data(inf);
  rf.calculate();
  rf.print_data(outf);

  time_t end = time(0);
  double time = difftime(end, start);

  printf("Elapsed Time = %8.4g sec \n", time);
}

void Rainflow::print_data(const char * outf) {
  pFile[1] = fopen(outf, "w");

  fprintf(pFile[1], "\n Amplitude = (peak-valley)/2 \n");
  fprintf(pFile[1], "\n ");
  fprintf(pFile[1], "\n          Range            Cycle       Ave     Max     "
                    "Ave     Min      Max");
  fprintf(pFile[1], "\n         (units)           Counts      Amp     Amp     "
                    "Mean    Valley   Peak \n");

  for (unsigned i = 13; i >= 1; i--) {
    fprintf(pFile[1],
            "  %8.4lf to %8.4lf\t%8.1lf\t%6.4g\t%6.4g\t%6.4g\t %6.4g\t %6.4g\n",
            L[i], L[i + 1], C[i], AverageAmp[i], MaxAmp[i], AverageMean[i],
            MinValley[i], MaxPeak[i]);
  }

  fclose(pFile[1]);
}

void Rainflow::read_data(const char * inf) {
  float aa;

  if ((pFile[0] = fopen(inf, "rb")) == NULL) {
    printf("\n Failed to open file: %s \n", inf);
    exit(0);
  }

  while (fscanf(pFile[0], "%f", &aa) > 0) {
    y.push_back(aa);

    if (y.size() == MAX) {
      printf("\n Warning:  input data limit reached \n.");
      break;
    }
  }

  fclose(pFile[0]);
}

void Rainflow::find_peaks(vector<float> *peaks) {
  peaks->push_back(y[0]);

  unsigned i;
  for (i = 1; i < (y.size() - 1); i++) {
    double slope1 = (y[i] - y[i - 1]);
    double slope2 = (y[i + 1] - y[i]);

    if ((slope1 * slope2) <= 0. && fabs(slope1) > 0.) {
      peaks->push_back(y[i]);
    }
  }
  peaks->push_back(y.back());
}

void Rainflow::find_cycles(vector<float> &peaks, vector<Cycle> * cycles) {
  unsigned i = 0;
  Cycle cycle;
  while ((i + 2) < peaks.size()) {
    float delta1 = (fabs(peaks[i] - peaks[i + 1]));
    float delta2 = (fabs(peaks[i + 1] - peaks[i + 2]));

    if (delta2 >= delta1 && delta1 > 0 && delta1 < 1.0e+20) {
      cycle.peak1 = peaks[i];
      cycle.peak2 = peaks[i + 1];
      cycle.size = (i == 0) ? 0.5 : 1.0;
      cycle.amplitude = delta1;

      cycles->push_back(cycle);

      peaks.erase(peaks.begin() + i);
      if (i != 0) {
        peaks.erase(peaks.begin() + i);
      }

      i = 0;
    } else {
      i++;
    }
  }

  for (i = 0; i < peaks.size(); i++) {
    float delta = (fabs(peaks[i] - peaks[i + 1]));
    if (delta > 0. && delta < 1.0e+20) {
      cycle.peak1 = peaks[i];
      cycle.peak2 = peaks[i + 1];
      cycle.size = 0.5;
      cycle.amplitude = delta;
      cycles->push_back(cycle);
    }
  }
}

void Rainflow::calculate(void) {
  vector<float> peaks;
  find_peaks(&peaks);

  vector<Cycle> cycles;
  find_cycles(peaks, &cycles);

  unsigned i = 0;
  float max_cycle_amplitude = 0.;
  float total_cycles = 0.;
  for (i = 0; i < cycles.size(); i++) {
    float amplitude = cycles[i].amplitude;
    if (amplitude > max_cycle_amplitude) {
      max_cycle_amplitude = amplitude;
    }
    total_cycles += cycles[i].size;
  }

  L[1] = 0;
  L[2] = 2.5;
  L[3] = 5;
  L[4] = 10;
  L[5] = 15;
  L[6] = 20;
  L[7] = 30;
  L[8] = 40;
  L[9] = 50;
  L[10] = 60;
  L[11] = 70;
  L[12] = 80;
  L[13] = 90;
  L[14] = 100;

  for (ijk = 1; ijk <= 14; ijk++) {
    L[ijk] *= max_cycle_amplitude / 100.;

    C[ijk] = 0.;
    AverageMean[ijk] = 0.;
    MaxPeak[ijk] = -1.0e+20;
    MinValley[ijk] = 1.0e+20;

    MaxAmp[ijk] = -1.0e+20;
    AverageAmp[ijk] = 1.0e+20;

    //       printf("  L[%ld]=%g max_cycle_amplitude=%8.4g\n",ijk,L[ijk],max_cycle_amplitude);
  }

  //   getch();

  for (ijk = 13; ijk >= 0; ijk--) {
    AverageAmp[ijk] = 0.;
  }

  for (i = 0; i < cycles.size(); i++) {
    float amplitude = cycles[i].amplitude;

    //        printf(" %ld %ld %10.4e \t %3.1f \n",i,kv,delta1,cycles[i][1]);

    //     printf("i=%d delta1=%g \n",i,Y);

    for (ijk = 13; ijk >= 0; ijk--) {
      //           printf(" %8.4g  %8.4g   %8.4g \n",delta1,L[ijk],L[ijk+1]);

      if (amplitude >= L[ijk] && amplitude <= L[ijk + 1]) {
        C[ijk] = C[ijk] + cycles[i].size;
        AverageMean[ijk] +=
            cycles[i].size * (cycles[i].peak1 + cycles[i].peak2) * 0.5; // weighted average

        if (cycles[i].peak1 > MaxPeak[ijk]) {
          MaxPeak[ijk] = cycles[i].peak1;
        }
        if (cycles[i].peak2 > MaxPeak[ijk]) {
          MaxPeak[ijk] = cycles[i].peak2;
        }

        if (cycles[i].peak1 < MinValley[ijk]) {
          MinValley[ijk] = cycles[i].peak1;
        }
        if (cycles[i].peak2 < MinValley[ijk]) {
          MinValley[ijk] = cycles[i].peak2;
        }

        if (amplitude > MaxAmp[ijk]) {
          MaxAmp[ijk] = amplitude;
        }

        AverageAmp[ijk] += cycles[i].size * amplitude * 0.5;

        break;
      }
    }
  }

  for (ijk = 1; ijk <= 14; ijk++) {
    if (C[ijk] > 0) {
      AverageMean[ijk] /= C[ijk];
      AverageAmp[ijk] /= C[ijk];
    }
    MaxAmp[ijk] /= 2.;

    if (C[ijk] < 0.5) {
      AverageAmp[ijk] = 0.;
      MaxAmp[ijk] = 0.;
      AverageMean[ijk] = 0.;
      MinValley[ijk] = 0.;
      MaxPeak[ijk] = 0.;
    }
    //        printf(" %8.4g  %8.4g  %8.4g  %8.4g  %8.4g
    //        \n",AverageAmp[ijk],MaxAmp[ijk],AverageMean[ijk],MinValley[ijk],MaxPeak[ijk]);
  }

  printf("\n\n  Total Cycles = %g  NP=%ld max_cycle_amplitude=%g\n", total_cycles, y.size(), max_cycle_amplitude);
}
