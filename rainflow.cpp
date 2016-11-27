#include <cstdlib>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <array>
#include <cfloat>
#include <sys/timeb.h>
#include <sys/types.h>
#include <ctime>

#define MAX 51000000

using namespace std;

struct Cycle {
    float peak1, peak2, size, amplitude;
};


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

        vector<float> find_peaks() const;
        vector<Cycle> find_cycles(vector<float> &peaks) const;
        void calculate_statistics(vector<Cycle> const &cycles);

    public:
        void read_data(const char *in_file_name);
        void calculate();
        void print_data(const char *out_file_name) const;
};

int main(int argc, char *argv[]) {
    char *out_file_name;
    if (argc == 1) {
        printf("Usage: rainflow <input file> [<output file>]\n");
        exit(EXIT_FAILURE);
    } else if (argc == 2) {
        out_file_name = (char *)"rainflow.out";
    } else {
        out_file_name = argv[2];
    }
    char *in_file_name = argv[1];

    printf("rainflow.cpp  Ver 2.8  June 24, 2014\n");
    printf("by Tom Irvine  Email: <tom@vibrationdata.com>\n");
    printf("ASTM E 1049-85 (2005) Rainflow Counting Method\n");

    const time_t start = time(0);

    Rainflow rf;
    rf.read_data(in_file_name);
    rf.calculate();
    rf.print_data(out_file_name);

    const time_t end = time(0);
    const double time = difftime(end, start);

    printf("Elapsed Time = %8.4g sec\n", time);
    return EXIT_SUCCESS;
}

void Rainflow::print_data(const char *out_file_name) const {
    FILE *fout = fopen(out_file_name, "w");

    if (fout == nullptr) {
        printf("\nFailed to open output file: %s\n", out_file_name);
        exit(EXIT_FAILURE);
    }

    fprintf(fout, "\n Amplitude = (peak-valley)/2 \n");
    fprintf(fout, "\n ");
    fprintf(fout, "\n          Range            Cycle       Ave     Max     Ave     Min      Max");
    fprintf(fout, "\n         (units)           Counts      Amp     Amp     Mean    Valley   Peak \n");

    for (int i = _bins.size() - 2; i >= 0; i--) {
        fprintf(fout, "  %8.4lf to %8.4lf\t%8.1lf\t%6.4g\t%6.4g\t%6.4g\t %6.4g\t %6.4g\n",
                _bins[i], _bins[i + 1], _cycle_counts[i], _average_amps[i], _max_amps[i],
                _average_means[i], _min_valleys[i], _max_peaks[i]);
    }

    fclose(fout);
}

void Rainflow::read_data(const char *in_file_name) {
    FILE *fin = fopen(in_file_name, "rb");

    if (fin == nullptr) {
        printf("\n Failed to open file: %s \n", in_file_name);
        exit(EXIT_FAILURE);
    }

    float point;
    while (fscanf(fin, "%f", &point) > 0) {
        _points.push_back(point);
    }

    fclose(fin);
}


vector<float> Rainflow::find_peaks() const {
    vector<float> peaks;
    peaks.push_back(_points[0]);

    for (unsigned i = 1; i < _points.size() - 1; i++) {
        const double slope1 = _points[i] - _points[i - 1];
        const double slope2 = _points[i + 1] - _points[i];

        if (slope1 * slope2 <= 0.0 && fabs(slope1) > 0.0) {
            peaks.push_back(_points[i]);
        }
    }
    peaks.push_back(_points.back());
    return peaks;
}

vector<Cycle> Rainflow::find_cycles(vector<float> &peaks) const {
    vector<Cycle> cycles;
    unsigned i = 0;

    while (i + 2 < peaks.size()) {
        const float delta1 = fabs(peaks[i] - peaks[i + 1]);
        const float delta2 = fabs(peaks[i + 1] - peaks[i + 2]);

        if (delta2 >= delta1 && delta1 > 0.0 && delta1 < FLT_MAX) {
            cycles.push_back(Cycle {
                .peak1 = peaks[i],
                .peak2 = peaks[i + 1],
                .size = (i == 0) ? 0.5f : 1.0f,
                .amplitude = delta1,
            });

            peaks.erase(peaks.begin() + i);
            if (i != 0) {
                peaks.erase(peaks.begin() + i);
            }

            i = 0;
        } else {
            i++;
        }
    }

    for (unsigned j = 0; j < peaks.size(); j++) { // OUT OF RANGE
        const float delta = fabs(peaks[j] - peaks[j + 1]);
        if (delta > 0.0 && delta < FLT_MAX) {
            cycles.push_back(Cycle {
                .peak1 = peaks[j],
                .peak2 = peaks[j + 1],
                .size = 0.5,
                .amplitude = delta,
            });
        }
    }

    return cycles;
}


void Rainflow::calculate_statistics(vector<Cycle> const &cycles) {
    float max_cycle_amplitude = 0.0;
    float total_cycles = 0.0;
    for (unsigned i = 0; i < cycles.size(); i++) {
        max_cycle_amplitude = max(max_cycle_amplitude, cycles[i].amplitude);
        total_cycles += cycles[i].size;
    }

    const static array<double, 14> BIN_PERCENTAGES = {0.0, 2.5, 5.0, 10.0, 15.0, 20.0, 30.0,
                                                      40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0};
    const size_t bin_count = BIN_PERCENTAGES.size();
    _bins.resize(bin_count);
    for (unsigned i = 0; i < bin_count; i++) {
        _bins[i] = BIN_PERCENTAGES[i] * max_cycle_amplitude / 100.0;
    }

    _cycle_counts.assign(bin_count, 0.0);
    _average_means.assign(bin_count, 0.0);
    _max_peaks.assign(bin_count, -FLT_MAX);
    _min_valleys.assign(bin_count, FLT_MAX);
    _max_amps.assign(bin_count, -FLT_MAX);
    _average_amps.assign(bin_count, 0.0);

    for (unsigned i = 0; i < cycles.size(); i++) {
        const float amplitude = cycles[i].amplitude;

        for (unsigned j = 0; j < bin_count - 1; j++) {
            if (amplitude >= _bins[j] && amplitude <= _bins[j + 1]) {
                _cycle_counts[j] += cycles[i].size;
                _average_means[j] += cycles[i].size * (cycles[i].peak1 + cycles[i].peak2) * 0.5; // Weighted average.

                _max_peaks[j] = max(max(cycles[i].peak1, cycles[i].peak2), _max_peaks[j]);
                _min_valleys[j] = min(min(cycles[i].peak1, cycles[i].peak2), _min_valleys[j]);
                _max_amps[j] = max(_max_amps[j], amplitude);

                _average_amps[j] += cycles[i].size * amplitude * 0.5;

                break;
            }
        }
    }

    for (unsigned i = 0; i < bin_count - 1; i++) {
        if (_cycle_counts[i] > 0) {
            _average_means[i] /= _cycle_counts[i];
            _average_amps[i] /= _cycle_counts[i];
        }
        _max_amps[i] /= 2.0;

        if (_cycle_counts[i] < 0.5) {
            _average_amps[i] = 0.0;
            _max_amps[i] = 0.0;
            _average_means[i] = 0.0;
            _min_valleys[i] = 0.0;
            _max_peaks[i] = 0.0;
        }
    }

    // This print should go ouside the computing member functions.
    printf("\n\n  Total Cycles = %g  NP=%lu max_cycle_amplitude=%g\n",
           total_cycles, _points.size(), max_cycle_amplitude);
}

void Rainflow::calculate() {
    vector<float> peaks = find_peaks();
    const vector<Cycle> cycles = find_cycles(peaks);
    calculate_statistics(cycles);
}
