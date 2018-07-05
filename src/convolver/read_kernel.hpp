#ifndef READ_KERNEL_HPP
#define READ_KERNEL_HPP

#include <samplerate.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sndfile.hh>

namespace rk {

std::string log_tag = "convolver_plugin: ";

void read_file(const char* path,
               float*& kernel,
               int& kernel_size,
               const int& rate) {
    SndfileHandle file = SndfileHandle(path);

    std::cout << "Opened file: " << path << std::endl;
    std::cout << "rate: " << file.samplerate() << std::endl;
    std::cout << "channels: " << file.channels() << std::endl;

    // for now only stere irs are supported

    if (file.channels() == 2) {
        bool resample = false;
        float resample_ratio = 1.0;
        int frames_in = file.channels() * file.frames();
        float* buffer = new float[frames_in];

        file.readf(buffer, file.frames());

        if (file.samplerate() != rate) {
            resample = true;

            resample_ratio = (float)rate / file.samplerate();

            int frames_out =
                file.channels() * ceil(file.frames() * resample_ratio);

            kernel = new float[frames_out];
            kernel_size = frames_out;
        } else {
            kernel = new float[frames_in];
            kernel_size = frames_in;
        }

        if (resample) {
            SRC_STATE* src_state =
                src_new(SRC_SINC_BEST_QUALITY, file.channels(), nullptr);

            SRC_DATA src_data;

            src_data.input_frames = file.frames();
            src_data.output_frames = file.frames() * resample_ratio;
            src_data.end_of_input = 1;
            src_data.src_ratio = resample_ratio;
            src_data.input_frames_used = 0;
            src_data.output_frames_gen = 0;
            src_data.data_in = buffer;
            src_data.data_out = kernel;

            src_process(src_state, &src_data);

            src_delete(src_state);
        } else {
            std::memcpy(kernel, buffer, frames_in * sizeof(float));
        }

        delete[] buffer;
    } else {
    }
}

}  // namespace rk

#endif
