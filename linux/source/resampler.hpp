// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
// AiloEMU: streaming linear stereo resampler. GPL-2.0-or-later.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>
class Resampler {
    std::vector<int16_t> pending;
    double phase=0,rate=0;
public:
    void clear(){pending.clear();phase=0;rate=0;}
    std::vector<int16_t> convert(const std::vector<int16_t>& source,double sourceRate){
        if(sourceRate<8000||sourceRate>192000)return {};
        if(rate!=sourceRate){clear();rate=sourceRate;}
        if(sourceRate==48000)return source;
        pending.insert(pending.end(),source.begin(),source.end());
        std::vector<int16_t> result;size_t frames=pending.size()/2;
        const double step=sourceRate/48000.0;
        while(phase+1<frames){
            size_t i=size_t(phase);double frac=phase-i;
            for(size_t channel=0;channel<2;channel++){
                double sample=pending[2*i+channel]*(1-frac)+pending[2*(i+1)+channel]*frac;
                result.push_back(int16_t(std::clamp(std::lround(sample),-32768L,32767L)));
            }
            phase+=step;
        }
        size_t consumed=std::min(size_t(phase),frames);
        pending.erase(pending.begin(),pending.begin()+consumed*2);phase-=consumed;
        return result;
    }
};
