
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

class Rainflow {
private:
  float C[100];
  float AverageMean[100];
  float MaxPeak[100];
  float MinValley[100];

  float MaxAmp[100];
  float AverageAmp[100];

  double L[20];

  float sum;
  float ymax;

  float mina, maxa;

  float t;
  float scale;
  float X, Y;

  long ijk;

  long kv;

  long hold;
  long j, n;
  long num;

  long nkv;

  int ic, iscale;

  long N;

  long last_a;

  FILE *pFile[6];
  char filename[6][FILENAME_MAX];

  vector<float> y;
  vector<vector<float> > B; // need space > >

public:
  void read_data(const char * inf);
  void calculate(void);
  void find_peaks(vector<float> * peaks);
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

  printf("\n\n  Total Cycles = %g  hold=%ld  NP=%ld ymax=%g\n", sum, hold, y.size(),
         ymax);
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

void Rainflow::calculate(void) {
  vector<float> peaks;
  find_peaks(&peaks);

  last_a = peaks.size() - 1;

  hold = last_a;

  long WIDTH = 4;
  long HEIGHT = 0;
  B.resize(HEIGHT);

  unsigned i;
  for (i = 0; i < HEIGHT; i++) {
    B[i].resize(WIDTH);
  }

  //   printf(" H=%ld W=%ld ",HEIGHT,WIDTH);
  //   getch();

  mina = 100000;
  maxa = -mina;

  for (i = 0; i <= last_a; i++) {
    if (peaks[i] < mina) {
      mina = peaks[i];
    }
    if (peaks[i] > maxa) {
      maxa = peaks[i];
    }

    //		fprintf(pFile[5]," %8.4g \n",a[i]);
  }

  num = long(maxa - mina) + 1;

  n = 0;
  i = 0;
  j = 1;

  sum = 0;

  kv = 0;

  long LLL = last_a;

  std::vector<float> row(4);

  printf("\n percent completed \n");

  ymax = 0.;
  nkv = 0;
  while (1) {
    Y = (fabs(peaks[i] - peaks[i + 1]));
    X = (fabs(peaks[j] - peaks[j + 1]));

    if (X >= Y && Y > 0 && Y < 1.0e+20) {
      if (Y > ymax) {
        ymax = Y;
      }

      if (i == 0) {

        n = 0;
        sum += 0.5;

        row[3] = peaks[i + 1];
        row[2] = peaks[i];
        row[1] = 0.5;
        row[0] = Y;

        B.push_back(row);

        //                printf("1 %8.4g %8.4g %8.4g %8.4g
        //                \n",B[kv][0],B[kv][1],B[kv][2],B[kv][3]);

        kv++;

        peaks.erase(peaks.begin());

        last_a--;

        i = 0;
        j = 1;
      } else {
        sum += 1;

        row[3] = peaks[i + 1];
        row[2] = peaks[i];
        row[1] = 1.;
        row[0] = Y;

        B.push_back(row);
        //                printf("2 %8.4g %8.4g %8.4g %8.4g
        //                \n",B[kv][0],B[kv][1],B[kv][2],B[kv][3]);

        kv++;

        n = 0;

        peaks.erase(peaks.begin() + (i + 1));
        peaks.erase(peaks.begin() + i);

        last_a -= 2;

        i = 0;
        j = 1;
      }

      nkv++;

      if (nkv == 3000) {
        double ratio = fabs((last_a) / double(LLL));

        printf(" %3.1lf \n", (1 - ratio) * 100.);
        nkv = 0;
      }
    } else {
      i++;
      j++;
    }

    if ((j + 1) > last_a) {
      break;
    }
  }

  for (i = 0; i < (last_a); i++) {
    Y = (fabs(peaks[i] - peaks[i + 1]));
    if (Y > 0. && Y < 1.0e+20) {
      sum += 0.5;

      row[3] = peaks[i + 1];
      row[2] = peaks[i];
      row[1] = 0.5;
      row[0] = Y;
      B.push_back(row);
      //            printf("3  %8.4g %8.4g %8.4g %8.4g
      //            \n",B[kv][0],B[kv][1],B[kv][2],B[kv][3]);

      kv++;

      if (Y > ymax) {
        ymax = Y;
      }
    }
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
    L[ijk] *= ymax / 100.;

    C[ijk] = 0.;
    AverageMean[ijk] = 0.;
    MaxPeak[ijk] = -1.0e+20;
    MinValley[ijk] = 1.0e+20;

    MaxAmp[ijk] = -1.0e+20;
    AverageAmp[ijk] = 1.0e+20;

    //       printf("  L[%ld]=%g ymax=%8.4g\n",ijk,L[ijk],ymax);
  }

  //   getch();

  kv--;

  for (ijk = 13; ijk >= 0; ijk--) {
    AverageAmp[ijk] = 0.;
  }

  for (i = 0; i <= kv; i++) {
    Y = B[i][0];

    //        printf(" %ld %ld %10.4e \t %3.1f \n",i,kv,Y,B[i][1]);

    //     printf("i=%d Y=%g \n",i,Y);

    for (ijk = 13; ijk >= 0; ijk--) {
      //           printf(" %8.4g  %8.4g   %8.4g \n",Y,L[ijk],L[ijk+1]);

      if (Y >= L[ijk] && Y <= L[ijk + 1]) {
        C[ijk] = C[ijk] + B[i][1];
        AverageMean[ijk] +=
            B[i][1] * (B[i][3] + B[i][2]) * 0.5; // weighted average

        if (B[i][3] > MaxPeak[ijk]) {
          MaxPeak[ijk] = B[i][3];
        }
        if (B[i][2] > MaxPeak[ijk]) {
          MaxPeak[ijk] = B[i][2];
        }

        if (B[i][3] < MinValley[ijk]) {
          MinValley[ijk] = B[i][3];
        }
        if (B[i][2] < MinValley[ijk]) {
          MinValley[ijk] = B[i][2];
        }

        if (Y > MaxAmp[ijk]) {
          MaxAmp[ijk] = Y;
        }

        AverageAmp[ijk] += B[i][1] * Y * 0.5;

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
}
