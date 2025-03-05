#pragma once

#include <cassert>
#include <cstdlib>
#include <vector>

namespace ky{
    template <typename F>
    inline F wrap(F value, F high = 1, F low = 0) {
      std::equal_to<F> eq;
      if (eq(high, low)) return low;
      if (value >= high) {
        F range = high - low;
        value -= range;
        if (value >= high) {
          value -= (F)(unsigned)((value - low) / range);
        }
      } else if (value < low) {
        F range = high - low;
        value += range;
        if (value < low) {
          value += (F)(unsigned)((high - value) / range);
        }
      }
      return value;
    }

    struct PlaybackRateObserver {
        float samplerate{1};
        PlaybackRateObserver* nextObserver{nullptr};
        virtual void onPlaybackRateChange(float samplerate);
        PlaybackRateObserver();
      };
      
      class PlaybackRateSubject {
        PlaybackRateObserver* list{nullptr};
        PlaybackRateSubject() {}
      
       public:
        PlaybackRateSubject(PlaybackRateSubject const&) = delete;
        void operator=(PlaybackRateSubject const&) = delete;
      
        void notifyObservers(float samplerate);
        void addObserver(PlaybackRateObserver* observer);
        static PlaybackRateSubject& instance();
      };
      
      // convenience function for setting playback rate
      inline void setPlaybackRate(float samplerate) {
        PlaybackRateSubject::instance().notifyObservers(samplerate);
      }
      
      class Ramp : public PlaybackRateObserver {
        float value = 0;
        float increment = 0;  // normalized frequency
      
       public:
        void frequency(float hertz) {
          assert(samplerate != 0.0f);
          increment = hertz / samplerate;
        }
      
        float operator()() {
          float v = value;
      
          value += increment;
      
          if (value >= 1.0) {
            value -= 1.0f;
          }
      
          return v;
        }
      };

    struct FloatVectorWrap : public std::vector<float> {
        float operator()(float index) {
          index = wrap(index, static_cast<float>(size()), 0.0f);
          size_t i = static_cast<size_t>(floor(index));
          size_t j = (1 + i) % size();
          float t = index - i;
          return operator[](i) * (1 - t) + t * operator[](j);
        }
      };

    class ClipPlayer {
        FloatVectorWrap data;
      
       public:
        void addSample(float f) { data.push_back(f); }
      
        // input on (0, 1)
        // linear interpolation
        float operator()(float phase) {
          if (data.empty()) return 0.0f;
          return data(phase * data.size());
        }
      };
}