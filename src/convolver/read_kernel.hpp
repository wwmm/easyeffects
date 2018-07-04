#ifndef READ_KERNEL_HPP
#define READ_KERNEL_HPP

#include <speex/speex_resampler.h>
#include <iostream>
#include <sndfile.hh>

namespace rk {

std::string log_tag = "convolver_plugin: ";

void read_file(const char* path, const int& rate) {
    SndfileHandle file = SndfileHandle(path);

    if (file.channels() == 2) {
        float* buffer_in = new float[file.channels() * file.frames()];

        file.read(buffer_in, file.channels() * file.frames());

        std::cout << rate << std::endl;

        float resample_ratio = 1.0;

        if (file.samplerate() != rate) {
            resample_ratio = rate / file.samplerate();
            std::cout << log_tag + "sample rate mismatch" << std::endl;
        }

        printf("Opened file '%s'\n", path);
        printf("    Sample rate : %d\n", file.samplerate());
        printf("    Channels    : %d\n", file.channels());

        puts("");
    } else {
    }
}

}  // namespace rk

#endif
