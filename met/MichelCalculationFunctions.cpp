#include "MichelCalculationFunctions.h"

template<typename T>
std::vector<std::vector<T> > get_windows(const std::vector<T>& the_thing,
                                         const size_t window_size)
{
  
  // given a vector of values return a vector of the same length
  // with each element being a vector of the values of the local neighbors
  // of the element at position i in the original vector
  // input  : [0,1,2,3,4,5,6,...,...,N-3,N-2,N-1] (input vector of size N)
  // output  (assuming a value of 'w' below == 3):
  // 0th element: [0]
  // 1st element: [0,1,2]
  // 2nd element: [0,1,2,3,4]
  // jth element: [j-w,j-w+1,..,j+w-2,j+w-1]
  
  std::vector<std::vector<T> > data;
  
  auto w = window_size + 2;
  w = (unsigned int)((w - 1)/2);
  auto num = the_thing.size();
  
  data.reserve(num);
  
  for(size_t i = 1; i <= num; ++i) {
    std::vector<T> inner;
    inner.reserve(20);
    // if we are at the beginning of the vector (and risk accessing -1 elements)
    if(i < w)
      {
        for(size_t j = 0; j < 2 * (i%w) - 1; ++j)
          inner.push_back(the_thing[j]);
      }
    // if we are at the end of the vector (and risk going past it)
    else if (i > num - w + 1)
      {
        for(size_t j = num - 2*((num - i)%w)-1 ; j < num; ++j)
          inner.push_back(the_thing[j]);
      }
    // if we are in the middle of the waveform
    else
      {
        for(size_t j = i - w; j < i + w - 1; ++j)
          inner.push_back(the_thing[j]);
      }
    data.emplace_back(inner);
  }

  return data;

}
double mean(const std::vector<double>& data)
{
  if(data.size() == 0) std::cerr << "Unable to perform mean" << std::endl;

  double result = 0.0;

  for(const auto& d : data)
    result += d;

  return (result / ((double)data.size()));
}
double cov (const std::vector<double>& data1,
            const std::vector<double>& data2)
{
  if(data1.size() == 0) std::cerr << "Unable to perform cov" << std::endl;
  if(data2.size() == 0) std::cerr << "Unable to perform cov" << std::endl;

  double result = 0.0;
  auto   mean1  = mean(data1);
  auto   mean2  = mean(data2);

  for(size_t i = 0; i < data1.size(); ++i)
    result += (data1[i] - mean1)*(data2[i] - mean2);

  return result/((double)data1.size());

}
double stdev(const std::vector<double>& data)
{
  if(data.size() == 0) std::cerr << "Unable to perform stdev" << std::endl;

  double result = 0.0;
  auto    avg   = mean(data);
  for(const auto& d: data)
    result += (d - avg)*(d - avg);

  return sqrt(result/((double)data.size()));
}

//modify to take trigger primitives, modify the second for loop
std::vector<double>  calc_covariance(const std::vector<TP>& hits,
                                     const int _n_window_size)
{
  std::vector<double> R;
  R.reserve(hits.size());
  
  std::vector<double> X;
  std::vector<double> Y;
  X.reserve(_n_window_size);
  Y.reserve(_n_window_size);
  
  for(const auto& window : get_windows(hits,_n_window_size) ) {
    
      //change this part
    for(const auto& hit : window) {
      X.push_back(hit.channel); Y.push_back(hit.tstart);
    }
    
    auto c  = cov(X,Y);
    auto sX = stdev(X);
    auto sY = stdev(Y);
    auto r  = c/(sX * sY);
    
      /*if(true){
    //if(_verbosity <= msg::kDEBUG) {
      std::stringstream ss;
      ss << "c: "  << c << std::endl
         << "sX: " << sX <<  std::endl
         << "sY: " << sY <<  std::endl
         << "r: "  << r <<  std::endl;
          std::cout <<ss.str()<<std::endl;
          //Print(msg::kDEBUG,__FUNCTION__,ss.str());
    }*/
    if(isnan(r)) r = 0.0;
    R.push_back(r);

    
    // if(R.size() != 1 && R.size() != hits.size())
    //        if(isnan(r)) Print(msg::kEXCEPTION,__FUNCTION__,"Covariance is nan not on edge");
    
    X.clear(); Y.clear();
  }
  //first and last points will be nan. Lets set them equal to the points just above and below
  R.at(0)            = R.at(1);
  R.at(R.size() - 1) = R.at(R.size() - 2);

  return R;
}


template<typename W>
void cut(std::vector<W>& data,  double frac)
{
    auto size = data.size();
    //calculate number of elements to be kept
    int to_stay = floor(frac*size);
    
    // sort the array based on charge
     // so that high-charge hits are removed
    std::sort(data.begin(),data.end(),
          [](const W& a, const W& b)->bool
          {
          return a <b;
          });
    // erase all elements after the last one to be kept
    data.erase(data.begin(), data.begin() + to_stay);
    data.erase(data.end() - to_stay, data.end());

}

void OrderPoints(size_t start_index,
                const std::vector<TP>& TPs,
                const double& d_cutoff,
                std::vector<size_t>& ordered_TPs_index,
                std::vector<double>& average_TP_distance_neighbors,
                std::vector<double>& average_TP_distance)
{
    ordered_TPs_index.clear();
    average_TP_distance.clear();
    
    if(TPs.empty() ) return;
    if (start_index >= TPs.size())std::cerr << "Unable to order points " <<std::endl;
    
    ordered_TPs_index.reserve(TPs.size());
    ordered_TPs_index.push_back(start_index);
    average_TP_distance_neighbors.reserve( TPs.size() );
    average_TP_distance.reserve(TPs.size() );
    
    //first average distance is zero
    average_TP_distance.push_back(0);
    
    std::vector<bool> used_v(TPs.size(),false);
    used_v[start_index] = true;
    while (ordered_TPs_index.size() < TPs.size() ){
        double min_dist = 9999;
        size_t min_index = -9999;
        for (size_t TP_index=0; TP_index<TPs.size(); ++TP_index ){
            //if used skip
            if(used_v[TP_index]) continue;
            
            //take a reference of this hit & last hit
            auto const& this_step = TPs[TP_index];
            auto const& last_step = TPs[ordered_TPs_index.back() ];
            
            if( (this_step.channel - last_step.channel) * (this_step.channel - last_step.channel ) > min_dist) continue;
            if( (this_step.tstart - last_step.tstart ) * (this_step.tstart - last_step.tstart ) > min_dist) continue;
            
            double sq_dist = (this_step.channel - last_step.channel) * (this_step.channel - last_step.channel ) + (this_step.tstart - last_step.tstart ) * (this_step.tstart - last_step.tstart );
            if(sq_dist > min_dist) continue;
            
            min_dist = sq_dist;
            min_index = TP_index;
        }
        if (min_dist > d_cutoff) break;
        
        ordered_TPs_index.push_back(min_index);
        average_TP_distance_neighbors.push_back( sqrt(min_dist) );
        average_TP_distance.push_back( average_TP_distance.back() + average_TP_distance_neighbors.back() );
        used_v[min_index] = true;
    }
        
    
}

void ProcessTPs( std::vector< TP> &TPs, std::vector<size_t> &ordered_TPs, std::vector<double> &average_TP_distance , std::vector<double> &average_TP_distance_neighbors,const double& d_cutoff ){
    size_t min_index = -9999;
    size_t max_index = -9999;
    double min_wire  = 99999;
    double max_wire  = -99999;
    
    
    //this might not be needed for TPs
    for(size_t i=0; i<TPs.size(); ++i) {
        if(TPs[i].channel < min_wire) {
            min_wire  = TPs[i].channel;
            min_index = i;
        }
        if(TPs[i].channel > max_wire) {
            max_wire  = TPs[i].channel;
            max_index = i;
        }
    }
    //std::vector<TriggerCandidate> ordered_TPs
    //std::vector<double> average_TP_distance, average_TP_distance_neighbors;
    OrderPoints(min_index,TPs,d_cutoff, ordered_TPs,average_TP_distance_neighbors, average_TP_distance);
    
    std::vector<TP> ordered_tp;
    ordered_tp.reserve(ordered_TPs.size() );
    for (auto const& TP_index : ordered_TPs )
        //ordered_tp.emplace_back(TPs[TP_index]);
        ordered_tp.push_back(TPs.at(TP_index) );
    std::swap(ordered_tp,TPs);
    
    for (size_t i=0; i<ordered_TPs.size(); ++i){
        ordered_TPs[i]=i;
    }
    
}

template<typename S>
S calc_mean(const std::vector<S>& data){
    if(!data.size() )std::cerr << "Unable to calculate calc_mean " <<std::endl;

    auto sum = S{0.0};
    for(const auto& d: data) sum+= d;
    return sum / ( (S) data.size() );
    
}

std::vector<double> do_smooth_mean( const std::vector<double>& dq,
                                      const double n_window_size,
                                      const int window_cutoff,
                                      const double p_above)
{
    std::vector<double> truncatedQ;
    
    for(auto window : get_windows(dq, n_window_size) ){
        if(window.size() > (size_t)window_cutoff)
            cut(window,p_above);
        truncatedQ.push_back(calc_mean(window));
    }
    return truncatedQ;
}


std::vector<double> calc_smooth_mean(const std::vector<TP> &TPs,
                                     const std::vector<size_t> ordered_TPs,
                                     const double n_window_size,
                                     const int window_cutoff,
                                     const double p_above)
{
    std::vector<double> charge;
    charge.reserve(ordered_TPs.size());
    for(const auto& o: ordered_TPs)
        charge.push_back(TPs.at(o).adcsum);
    return do_smooth_mean(charge, n_window_size,window_cutoff, p_above);
}




unsigned int nCk( unsigned int n, unsigned int k )
{
  if (k > n) return 0;
  if (k * 2 > n) k = n-k;
  if (k == 0) return 1;

  int result = n;
  for( unsigned int i = 2; i <= k; ++i ) {
    result *= (n-i+1);
    result /= i;
  }
  return result;
}

double coeff(double k, double N)
{
  auto m = (N - 3.0)/2.0;
  return 1.0/pow(2,2*m+1) * (nCk(2*m,m-k+1) - nCk(2*m,m-k-1));
}

double do_smooth_derive(const std::vector<double>& f,
                        const std::vector<double>& x,
                        int N)
{

  // N should def be odd.
  auto M   = int{(N - 1)/2};
  auto tot = double{0.0};

  for(int k = 0; k < M; ++k)
    tot += coeff(k+1,N) * (f[k+M] - f[M - 1 - k])/(x[k + M] - x[M - 1 - k]) * 2 * (k+1);

  return tot;

}

std::vector<double> calc_smooth_derive(const std::vector<double>& average_TP_distance,
                                       const std::vector<double>& tmeans,
                                       const int s)
{
    std::vector<double> tdqds;
    tdqds.reserve(tmeans.size() );
    if(!tmeans.size() )return tdqds;
    
    for(int o = 0; o<s; ++o)tdqds.push_back(0.0);
    for(int i = s; i < (int)tmeans.size() - s + 1; ++i) {
      std::vector<double> f(tmeans.begin() + i - s, tmeans.begin() + i + s);
      std::vector<double> x(average_TP_distance.begin() + i - s , average_TP_distance.begin() + i + s );
      tdqds.push_back(do_smooth_derive(f,x,2*s+1));
    }
     for(int o = 0; o < s - 1; ++o) tdqds.push_back(0.0);

    return tdqds;

}

std::vector<double> calc_slope(const std::vector<TP>& hits,
                               const int n_window_size)
{
  std::vector<double> S;
  S.reserve(hits.size());
  
  std::vector<double> X;
  std::vector<double> Y;
  X.reserve(n_window_size);
  Y.reserve(n_window_size);
  
  for(const auto& window : get_windows(hits,n_window_size) ) {
    for(const auto& hit : window) {
      X.push_back(hit.channel); Y.push_back(hit.tstart);
    }
    
    //http://mathworld.wolfram.com/LeastSquaresFitting.html
    auto c   = cov(X,Y);
    auto sX  = stdev(X);
    if(sX == 0.0) {c = 0.0; sX = 1.0;} //might have a problem with floating point error here
    auto b   = c/(sX*sX) ;
    
    S.push_back(b);
    X.clear(); Y.clear();
  }
  S.at(0)            = S.at(1);
  S.at(S.size() - 1) = S.at(S.size() - 2);
  
  
    
  return S;
    
}


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
                    int window_cutoff )
{
    if ( TPs.size() == 0) return false;
    truncated_mean.reserve(TPs.size() );
    truncated_dqds.reserve(TPs.size() );
    covariance.reserve(TPs.size() );
    slope.reserve(TPs.size() );
    
    truncated_mean = calc_smooth_mean(TPs,
                                      ordered_TPs,
                                      n_window_size,
                                      window_cutoff,
                                      p_above);
    
    if( truncated_mean.size() < edge_fix) return false;
    for( size_t i=0; i< edge_fix; i++)  {
        truncated_mean.at(i)= truncated_mean[edge_fix];
        truncated_mean.at(truncated_mean.size() -i -1 ) = truncated_mean[truncated_mean.size() - edge_fix];
        
    }
    int dir_window = covariance_window;
    covariance = calc_covariance(TPs, dir_window);
    slope = calc_slope(TPs, dir_window);
    int s = 3;
    truncated_dqds = calc_smooth_derive(average_TP_distance,truncated_mean,s);
    return true;
}
size_t find_max(const std::vector<double>& data)
{

  auto the_max = double{0.0};
  size_t idx = -99999;

  for(size_t i = 0; i < data.size(); ++i) {
    if(data[i] > the_max) {
      the_max = data[i]; idx = i;
    }
  }

  return idx;
}

bool BoundaryFromTQMaxQ(const std::vector<TP>& TPs, const std::vector<double> &truncated_mean, const std::vector<size_t> &ordered_TPs, const size_t maxDistance, int &boundary )
{

  if (TPs.size() == 0) return false;
  
  
  size_t candidate_loc = find_max(truncated_mean); // Truncated max... hopefully not degenerate
  
  size_t right = 0;
  if (ordered_TPs.size() >= (1+candidate_loc) )
      right = ordered_TPs.size() - 1 - candidate_loc;
  size_t left  = candidate_loc;
  
  size_t  iMin = 0;
  size_t  iMax = 0;
  
  if(right >= maxDistance) iMax  = maxDistance   + candidate_loc;
  if(left  >= maxDistance) iMin  = candidate_loc - maxDistance;
  
  if(right < maxDistance)  iMax  = TPs.size() - 1;
  if(left  < maxDistance)  iMin  = 0;
  
  // holder for hit with largest charge -> this will identify the location
  // of the hit that defines the michel boundary
  auto k   = 0.0;
  size_t idx = 0;
  
  for(size_t w = iMin; w <= iMax; ++w) {
      auto c = TPs.at(ordered_TPs[w]).adcsum;
    
    // if this hit has more charge than any other
    if(c > k) { k = c; idx = w; }
  }
  

  boundary    = ordered_TPs[idx];
  
  return true;
}

bool RequireBoundaryInLowCov(const std::vector<TP>& TPs,
                             const std::vector< double> &covariance,
                             const int boundary,
                             double _maxCovarianceAtStart = 0.9)
{

  if (TPs.size() == 0) return false;

  if(covariance.size() < TPs.size()) {
      std::cerr <<"Covariance vector size less than num hits, run CalcTruncated" <<std::endl;
  }

  //const auto& covariance = covariance;
  const auto& idx = boundary;

  // move on only if avg. covariance for hits near the "start" is less than some value
  // idx is the position of the reconstructed michel start
  double avg_covariance = 0;
  int    counts = 0;

  if ( idx < 3)
    return false;

  for (size_t i = (idx-3); i < (idx+4); i++){ // -3 and +4 is hardcoded ok fine
    // make sure we fall in the vector's range
    if ( i < covariance.size() ){
      avg_covariance += fabs(covariance[i]);
      counts += 1;
    }
  }

  // make sure we have at least 1 point!
  if (avg_covariance == 0)
    return false;

  // if so, check that the average covariance is below
  // the max allowed covariance
  avg_covariance /= counts;

  if (avg_covariance > _maxCovarianceAtStart)
    return false;

  return true;
}

