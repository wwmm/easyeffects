#ifndef READ_KERNEL_HPP
#define READ_KERNEL_HPP

#include <samplerate.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sndfile.hh>
#include "gstpeconvolver.hpp"

namespace rk {

std::string log_tag = "convolver_plugin: ";

void read_file(_GstPeconvolver* peconvolver) {
    SndfileHandle file = SndfileHandle(peconvolver->kernel_path);

    std::cout << "Opened file: " << peconvolver->kernel_path << std::endl;
    std::cout << "rate: " << file.samplerate() << std::endl;
    std::cout << "channels: " << file.channels() << std::endl;

    // for now only stereo irs files are supported

    if (file.channels() == 2) {
        bool resample = false;
        float resample_ratio = 1.0, *buffer, *kernel;
        int total_frames_in, total_frames_out, frames_in, frames_out;

        frames_in = file.frames();
        total_frames_in = file.channels() * frames_in;

        buffer = new float[total_frames_in];

        file.readf(buffer, frames_in);

        if (file.samplerate() != peconvolver->rate) {
            resample = true;

            resample_ratio = (float)peconvolver->rate / file.samplerate();

            frames_out = ceil(file.frames() * resample_ratio);
            total_frames_out = file.channels() * frames_out;

            kernel = new float[total_frames_out];
            peconvolver->kernel_L = new float[frames_out];
            peconvolver->kernel_R = new float[frames_out];
            peconvolver->kernel_n_frames = frames_out;
        } else {
            frames_out = frames_in;
            total_frames_out = file.channels() * frames_out;

            kernel = new float[total_frames_in];
            peconvolver->kernel_L = new float[frames_out];
            peconvolver->kernel_R = new float[frames_out];
            peconvolver->kernel_n_frames = frames_out;
        }

        if (resample) {
            SRC_STATE* src_state =
                src_new(SRC_SINC_BEST_QUALITY, file.channels(), nullptr);

            SRC_DATA src_data;

            src_data.input_frames = frames_in;
            src_data.output_frames = frames_out;
            src_data.end_of_input = 1;
            src_data.src_ratio = resample_ratio;
            src_data.input_frames_used = 0;
            src_data.output_frames_gen = 0;
            src_data.data_in = buffer;
            src_data.data_out = kernel;

            src_process(src_state, &src_data);

            src_delete(src_state);
        } else {
            std::memcpy(kernel, buffer, total_frames_in * sizeof(float));
        }

        // deinterleave
        for (int n = 0; n < frames_out; n += 2) {
            peconvolver->kernel_L[n] = kernel[n];
            peconvolver->kernel_R[n] = kernel[n + 1];
        }

        delete[] buffer;
        delete[] kernel;
    } else {
    }
}

}  // namespace rk

#endif
