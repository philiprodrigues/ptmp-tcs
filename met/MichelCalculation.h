#ifndef MICHELCALCULATION_H
#define MICHELCALCULATION_H

#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <fstream>


struct TriggerCandidate{
    int adjacency;
    int adcpeak;
    int adcsum;
    int tspan;
    int first_ch;
    int last_ch;
    double tstart;
    double first_time;
    double last_time;
    int apanum;
};

struct TP{
  unsigned int channel;
  unsigned int tstart;
  unsigned int tspan;
  unsigned int adcsum;
  unsigned int adcpeak;
  unsigned int flags;
};

template<typename T>
std::vector<std::vector<T> > get_windows(const std::vector<T>& the_thing,
                                         const size_t window_size);

double mean(const std::vector<double>& data);

double cov (const std::vector<double>& data1,
            const std::vector<double>& data2);

double stdev(const std::vector<double>& data);

std::vector<double>  calc_covariance(const std::vector<TP>& hits,
                                     const int _n_window_size);

template<typename W>
void cut(std::vector<W>& data,  double frac);

void OrderPoints(size_t start_index,
                 const std::vector<TP>& TPs,
                 const double& d_cutoff,
                 std::vector<size_t>& ordered_TPs_index,
                 std::vector<double>& average_TP_distance_neighbors,
                 std::vector<double>& average_TP_distance);

void ProcessTPs( std::vector< TP> &TPs,
           std::vector<size_t> &ordered_TPs,
           std::vector<double> &average_TP_distance ,
           std::vector<double> &average_TP_distance_neighbors,
           const double& d_cutoff );

template<typename S>
S calc_mean(const std::vector<S>& data);

std::vector<double> do_smooth_mean( const std::vector<double>& dq,
                                   const double n_window_size,
                                   const int window_cutoff,
                                   const double p_above);

std::vector<double> calc_smooth_mean(const std::vector<TP> &TPs,
                                     const std::vector<size_t> ordered_TPs,
                                     const double n_window_size,
                                     const int window_cutoff,
                                     const double p_above);

unsigned int nCk( unsigned int n, unsigned int k );

double coeff(double k, double N);

double do_smooth_derive(const std::vector<double>& f,
                        const std::vector<double>& x,
                        int N);

std::vector<double> calc_smooth_derive(const std::vector<double>&            average_TP_distance,
                                       const std::vector<double>& tmeans,
                                       const int s);

std::vector<double> calc_slope(const std::vector<TP>& hits,
                               const int n_window_size);

bool CalcTruncated( const std::vector<TP>& TPs,
                   const std::vector<size_t> &ordered_TPs,
                   const std::vector<double> &average_TP_distance,
                   std::vector< double> &truncated_mean,
                   std::vector< double> &truncated_dqds,
                   std::vector< double> &covariance,
                   std::vector< double> &slope,
                   size_t edge_fix,
                   int covariance_window,
                   int n_window_size,
                   double p_above,
                   int window_cutoff );

size_t find_max(const std::vector<double>& data);

bool BoundaryFromTQMaxQ(const std::vector<TP>& TPs,
                   const std::vector<double> &truncated_mean,
                   const std::vector<size_t> &ordered_TPs,
                   const size_t maxDistance,
                   int &boundary );

bool RequireBoundaryInLowCov(const std::vector<TP>& TPs,
                             const std::vector< double> &covariance,
                             const int boundary,
                             double maxCovarianceAtStart);

#endif /* MICHELCALCULATION_H */
