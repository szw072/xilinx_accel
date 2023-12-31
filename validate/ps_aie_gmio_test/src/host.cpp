/**
* Copyright (C) 2022 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/
#include <boost/filesystem.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "experimental/xrt_system.h"
#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"

#define NBYTES_DATA 4

#define WIDTH 8
#define HEIGHT 8
#define SIZE (WIDTH * HEIGHT)

#define SAMPLES 256

static void printHelp() {
    std::cout << "usage: %s <options>\n";
    std::cout << "  -p <path>\n";
    std::cout << "  -d <device> \n";
    std::cout << "  -s <supported>\n";
    std::cout << "  -h <help>\n";
}

int main(int argc, char** argv) {
    const int input_size_in_bytes = SIZE * sizeof(float);
    const int output_size_in_bytes = SIZE * sizeof(float);
    const int input_size_allocated = ((input_size_in_bytes / 4096) + ((input_size_in_bytes % 4096) > 0)) * 4096;
    const int output_size_allocated = ((output_size_in_bytes / 4096) + ((output_size_in_bytes % 4096) > 0)) * 4096;

    std::string dev_id = "0";
    std::string test_path;
    std::string b_file = "/ps_aie.xclbin";
    bool flag_s = false;
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-p") == 0) || (strcmp(argv[i], "--path") == 0)) {
            test_path = argv[i + 1];
        } else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--device") == 0)) {
            dev_id = argv[i + 1];
        } else if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--supported") == 0)) {
            flag_s = true;
        } else if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
            printHelp();
            return 1;
        }
    }

    if (test_path.empty()) {
        std::cout << "ERROR : please provide the platform test path to -p option\n";
        return EXIT_FAILURE;
    }
    auto binaryFile = boost::filesystem::path(test_path) / b_file;
    std::ifstream infile(binaryFile.string());
    if (flag_s) {
        if (!infile.good()) {
            std::cout << "\nNOT SUPPORTED" << std::endl;
            return EOPNOTSUPP;
        } else {
            std::cout << "\nSUPPORTED" << std::endl;
            return EXIT_SUCCESS;
        }
    }

    if (!infile.good()) {
        std::cout << "\nNOT SUPPORTED" << std::endl;
        return EOPNOTSUPP;
    }

    auto num_devices = xrt::system::enumerate_devices();

    xrt::device device;
    auto pos = dev_id.find(":");
    if (pos == std::string::npos) {
        uint32_t device_index = stoi(dev_id);
        if (device_index >= num_devices) {
            std::cout << "The device_index provided using -d flag is outside the range of "
                         "available devices\n";
            return EXIT_FAILURE;
        }
        device = xrt::device(device_index);
    } else {
        device = xrt::device(dev_id);
    }

    auto uuid = device.load_xclbin(binaryFile.string());
    auto aie_kernel = xrt::kernel(device, uuid.get(), "aie_kernel");
    auto out_bo = xrt::bo(device, output_size_allocated, aie_kernel.group_id(2));
    auto out_bomapped = out_bo.map<float*>();
    memset(out_bomapped, 0, output_size_in_bytes);

    auto in_boA = xrt::bo(device, input_size_allocated, aie_kernel.group_id(0));
    auto in_bomappedA = in_boA.map<float*>();
    auto in_boB = xrt::bo(device, input_size_allocated, aie_kernel.group_id(1));
    auto in_bomappedB = in_boB.map<float*>();

    // setting input data
    float* golden = (float*)malloc(output_size_in_bytes);
    for (int i = 0; i < SIZE; i++) {
        in_bomappedA[i] = rand() % SIZE;
        in_bomappedB[i] = rand() % SIZE;
    }
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            golden[i * WIDTH + j] = 0;
            for (int k = 0; k < WIDTH; k++)
                golden[i * WIDTH + j] += in_bomappedA[i * WIDTH + k] * in_bomappedB[k + WIDTH * j];
        }
    }

    in_boA.sync(XCL_BO_SYNC_BO_TO_DEVICE, input_size_in_bytes, 0);
    in_boB.sync(XCL_BO_SYNC_BO_TO_DEVICE, input_size_in_bytes, 0);

    int rval;

    auto run = aie_kernel(in_boA, in_boB, out_bo, input_size_in_bytes, output_size_in_bytes);
    run.wait();

    out_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE, output_size_in_bytes, 0);

    int match = 0;
    for (int i = 0; i < SIZE; i++) {
        if (out_bomapped[i] != golden[i]) {
            printf(
                "ERROR: Test failed! Error found in sample %d: golden: %f, "
                "hardware: %f\n",
                i, golden[i], out_bomapped[i]);
            match = 1;
            break;
        }
    }

    std::cout << "TEST " << (match ? "FAILED" : "PASSED") << std::endl;
    return (match ? EXIT_FAILURE : EXIT_SUCCESS);
}
