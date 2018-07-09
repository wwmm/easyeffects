#ifndef READ_KERNEL_HPP
#define READ_KERNEL_HPP

#include <samplerate.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sndfile.hh>
#include "gstpeconvolver.hpp"
#include "util.hpp"

namespace rk {

std::string log_tag = "convolver: ";

bool read_file(_GstPeconvolver* peconvolver) {
    if (peconvolver->kernel_path == nullptr) {
        util::warning(log_tag + "irs file path is null");

        return false;
    }

    SndfileHandle file = SndfileHandle(peconvolver->kernel_path);

    if (file.channels() == 0 || file.frames() == 0) {
        util::warning(log_tag + "irs file does not exists or it is empty: " +
                      peconvolver->kernel_path);

        return false;
    }

    util::debug(log_tag + "irs file: " + peconvolver->kernel_path);
    util::debug(log_tag + "irs rate: " + std::to_string(file.samplerate()) +
                " Hz");
    util::debug(log_tag + "irs channels: " + std::to_string(file.channels()));
    util::debug(log_tag + "irs frames: " + std::to_string(file.frames()));

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
        } else {
            frames_out = frames_in;
            total_frames_out = file.channels() * frames_out;
        }

        // allocate arrays

        kernel = new float[total_frames_out];
        peconvolver->kernel_L = new float[frames_out];
        peconvolver->kernel_R = new float[frames_out];
        peconvolver->kernel_n_frames = frames_out;

        // resample if necessary

        if (resample) {
            util::debug(log_tag + "resampling irs to " +
                        std::to_string(peconvolver->rate) + " Hz");

            SRC_STATE* src_state =
                src_new(SRC_SINC_BEST_QUALITY, file.channels(), nullptr);

            SRC_DATA src_data;

            // from https://github.com/x42/convoLV2/blob/master/convolution.cc

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
            util::debug(log_tag + "irs file does not need resampling");

            std::memcpy(kernel, buffer, total_frames_in * sizeof(float));
        }

        // deinterleave
        for (int n = 0; n < frames_out; n++) {
            peconvolver->kernel_L[n] = kernel[2 * n];
            peconvolver->kernel_R[n] = kernel[2 * n + 1];
        }

        // auto gain

        float rms = 0.0f, target_rms = powf(10.0f, -20.0f / 10.0f);

        for (int n = 0; n < frames_out; n++) {
            rms += peconvolver->kernel_L[n] * peconvolver->kernel_L[n];
            rms += peconvolver->kernel_R[n] * peconvolver->kernel_R[n];
        }

        rms /= (2.0f * frames_out);
        rms = sqrtf(rms);

        float autogain = rms / target_rms;

        util::debug(log_tag +
                    "irs rms: " + std::to_string(10.0f * log10f(rms)));
        util::debug(log_tag + "target irs rms: " + std::to_string(-20.0f));
        util::debug(log_tag + "autogain: " + std::to_string(autogain));

        for (int n = 0; n < frames_out; n++) {
            peconvolver->kernel_L[n] /= autogain;
            peconvolver->kernel_R[n] /= autogain;
        }

        delete[] buffer;
        delete[] kernel;
    } else {
        util::warning(log_tag + "only stereo impulse responses are supported." +
                      "The impulse file was not loaded!");
    }

    return true;
}

}  // namespace rk

#endif
