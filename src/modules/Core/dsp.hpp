#pragma once
#include <rack.hpp>
using namespace rack;

struct SimpleSlewLimiter {
	float rise{0.f};
    float fall{0.f};

    void configure(float rise, float fall) {
        assert (rise >= 0.f);
        assert (fall >= 0.f);
		this->rise = rise;
		this->fall = fall;
    }

    bool slewing() { return rise + fall > 0.f; }

    float next(float sample, float last_sample, float sampleTime){
        if (0.f == last_sample) return sample;
        return clamp(sample,
            last_sample - (fall * sampleTime),
            last_sample + (rise * sampleTime)
        );
    }
};
