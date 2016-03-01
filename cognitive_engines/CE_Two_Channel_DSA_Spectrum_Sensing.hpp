#ifndef _CE_TWO_CHANNEL_DSA_SPECTRUM_SENSING_
#define _CE_TWO_CHANNEL_DSA_SPECTRUM_SENSING_

#include "ECR.hpp"
#include "CE.hpp"
#include "timer.h"

class CE_Two_Channel_DSA_Spectrum_Sensing : public Cognitive_Engine {
public:
  CE_Two_Channel_DSA_Spectrum_Sensing(int argc, char **argv);
  ~CE_Two_Channel_DSA_Spectrum_Sensing();
  virtual void execute(ExtensibleCognitiveRadio *ECR);

private:
  // member functions
  void measureNoiseFloor(ExtensibleCognitiveRadio *ECR);
  int PUisPresent(ExtensibleCognitiveRadio *ECR);

  // measured noise power and flag
  float noise_floor;
  int noise_floor_measured;

  // Multiplicative coeffiecient applied to meausured noise
  // power to determine channel threshold for PU occupancy.
  static constexpr float threshold_coefficient = 20.0;

  // Number of measurements taken for noise floor
  // and time between each measurement
  static constexpr unsigned int numMeasurements = 60;
  static constexpr float measurementDelay_ms = 400.0;

  // How long to sense spectrum when checking noise floor
  // or for PU in milliseconds.
  static constexpr float sensingPeriod_ms = 100.0;

  // How frequently to recheck for PU
  static constexpr float sensingFrequency_Hz = 1.0;

  // Settling time for USRP
  static constexpr float tune_settling_time_ms = 20.0;

  // time related variables
  static constexpr float desired_timeout_ms = 10.0;
  timer t1;
  int tx_is_on;

  // counter/threshold for receiver frequency synchronization
  int no_sync_counter;
  static constexpr int no_sync_threshold = 100;

  // frequencies used in scenario
  float fc;     // RF center frequency of USRP
  float fshift; // DSP shift applied to reduce interference from transmitter
                // during sensing
  static constexpr float freq_a = 770e6; // Channel center frequencies
  static constexpr float freq_b = 769e6;
  static constexpr float freq_x = 765e6;
  static constexpr float freq_y = 764e6;

  // DSP shift applied to reach the target channel's center frequency
  float rx_foff;
  float tx_foff;

  // USRP sample and FFT output buffers
  float _Complex buffer[512];
  float _Complex buffer_F[512];

  // fft plan for channel measurements
  fftplan fft;
};

#endif
