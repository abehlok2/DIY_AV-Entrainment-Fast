#pragma once
#include <vector>
#include <string>
#include <utility>

std::vector<double> sineWave(double freq, const std::vector<double>& t, double phase = 0.0);
std::vector<double> sineWaveVarying(const std::vector<double>& freqArray,
                                    const std::vector<double>& t,
                                    double sampleRate = 44100.0);

std::vector<double> adsrEnvelope(const std::vector<double>& t,
                                 double attack = 0.01,
                                 double decay = 0.1,
                                 double sustainLevel = 0.8,
                                 double release = 0.1);

std::vector<double> createLinearFadeEnvelope(double totalDuration,
                                             double sampleRate,
                                             double fadeDuration,
                                             double startAmp,
                                             double endAmp,
                                             const std::string& fadeType = "in");

std::vector<double> linenEnvelope(const std::vector<double>& t,
                                  double attack = 0.1,
                                  double release = 0.1);

std::pair<std::vector<double>, std::vector<double>> pan2(const std::vector<double>& signal,
                                                         double pan = 0.0);

std::vector<double> bandpassFilter(const std::vector<double>& data,
                                   double center,
                                   double Q,
                                   double fs);

std::vector<double> bandrejectFilter(const std::vector<double>& data,
                                     double center,
                                     double Q,
                                     double fs);

std::vector<double> lowpassFilter(const std::vector<double>& data,
                                  double cutoff,
                                  double fs);

std::vector<double> pinkNoise(int n);
std::vector<double> brownNoise(int n);

std::vector<double> trapezoidEnvelopeVectorized(const std::vector<double>& tInCycle,
                                                const std::vector<double>& cycleLen,
                                                const std::vector<double>& rampPercent,
                                                const std::vector<double>& gapPercent);

std::vector<double> applyFilters(const std::vector<double>& signalSegment,
                                 double fs);

