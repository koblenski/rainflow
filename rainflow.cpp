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
  vector<double> _bins;
  vector<float> _points;

  vector<float> _cycle_counts;
  vector<float> _average_means;
  vector<float> _max_peaks;
  vector<float> _min_valleys;

  vector<float> _max_amps;
  vector<float> _average_amps;

  void find_peaks(vector<float> * peaks);
  void find_cycles(vector<float> &peaks, vector<Cycle> * cycles);
  void calculate_statistics(vector<Cycle> const &cycles);
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
  FILE *fp = fopen(outf, "w");

  fprintf(fp, "\n Amplitude = (peak-valley)/2 \n");
  fprintf(fp, "\n ");
  fprintf(fp, "\n          Range            Cycle       Ave     Max     "
                    "Ave     Min      Max");
  fprintf(fp, "\n         (units)           Counts      Amp     Amp     "
                    "Mean    Valley   Peak \n");

  for (int i = 12; i >= 0; i--) {
    fprintf(fp,
            "  %8.4lf to %8.4lf\t%8.1lf\t%6.4g\t%6.4g\t%6.4g\t %6.4g\t %6.4g\n",
            _bins[i], _bins[i + 1], _cycle_counts[i], _average_amps[i], _max_amps[i], _average_means[i],
            _min_valleys[i], _max_peaks[i]);
  }

  fclose(fp);
}

void Rainflow::read_data(const char * inf) {
  float point;
  FILE *fp;

  if ((fp = fopen(inf, "rb")) == NULL) {
    printf("\n Failed to open file: %s \n", inf);
    exit(0);
  }

  while (fscanf(fp, "%f", &point) > 0) {
    _points.push_back(point);

    if (_points.size() == MAX) {
      printf("\n Warning:  input data limit reached \n.");
      break;
    }
  }

  fclose(fp);
}

void Rainflow::find_peaks(vector<float> *peaks) {
  peaks->push_back(_points[0]);

  unsigned i;
  for (i = 1; i < (_points.size() - 1); i++) {
    double slope1 = _points[i] - _points[i - 1];
    double slope2 = _points[i + 1] - _points[i];

    if ((slope1 * slope2) <= 0. && fabs(slope1) > 0.) {
      peaks->push_back(_points[i]);
    }
  }
  peaks->push_back(_points.back());
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

void Rainflow::calculate_statistics(vector<Cycle> const &cycles) {
  unsigned i = 0;
  float max_cycle_amplitude = 0.;
  float total_cycles = 0.;
  for (i = 0; i < cycles.size(); i++) {
    max_cycle_amplitude = max(max_cycle_amplitude, cycles[i].amplitude);
    total_cycles += cycles[i].size;
  }

  double bin_percentages[] = {0, 2.5, 5, 10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 100};
  size_t bin_count = sizeof(bin_percentages)/sizeof(bin_percentages[0]);
  _bins.resize(bin_count);
  for (i = 0; i < 14; i++) {
    _bins[i] = bin_percentages[i] * max_cycle_amplitude / 100.;
  }

  _cycle_counts.assign(bin_count, 0.);
  _average_means.assign(bin_count, 0.);
  _max_peaks.assign(bin_count, -1.0e+20);
  _min_valleys.assign(bin_count, 1.0e+20);
  _max_amps.assign(bin_count, -1.0e+20);
  _average_amps.assign(bin_count, 0.);

  for (i = 0; i < cycles.size(); i++) {
    float amplitude = cycles[i].amplitude;

    for (unsigned j = 0; j < 13; j++) {
      if (amplitude >= _bins[j] && amplitude <= _bins[j + 1]) {
        _cycle_counts[j] += cycles[i].size;
        _average_means[j] +=
            cycles[i].size * (cycles[i].peak1 + cycles[i].peak2) * 0.5; // weighted average

        _max_peaks[j] = max( max(cycles[i].peak1, cycles[i].peak2), _max_peaks[j] );
        _min_valleys[j] = min( min(cycles[i].peak1, cycles[i].peak2), _min_valleys[j] );
        _max_amps[j] = max(_max_amps[j], amplitude);

        _average_amps[j] += cycles[i].size * amplitude * 0.5;

        break;
      }
    }
  }

  for (i = 0; i < 13; i++) {
    if (_cycle_counts[i] > 0) {
      _average_means[i] /= _cycle_counts[i];
      _average_amps[i] /= _cycle_counts[i];
    }
    _max_amps[i] /= 2.;

    if (_cycle_counts[i] < 0.5) {
      _average_amps[i] = 0.;
      _max_amps[i] = 0.;
      _average_means[i] = 0.;
      _min_valleys[i] = 0.;
      _max_peaks[i] = 0.;
    }
  }

  printf("\n\n  Total Cycles = %g  NP=%ld max_cycle_amplitude=%g\n", total_cycles, _points.size(), max_cycle_amplitude);
}

void Rainflow::calculate(void) {
  vector<float> peaks;
  find_peaks(&peaks);

  vector<Cycle> cycles;
  find_cycles(peaks, &cycles);

  calculate_statistics(cycles);
}
