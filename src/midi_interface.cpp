#include "midi_interface.hpp"

#include <algorithm>
#include <string.h>

size_t convert_from_midi_data(uint8_t* buffer, ssize_t size) {
    size_t result_size = 0;
    for (size_t i = 0; i < size; i += 8) {
        uint8_t data_arr[8] = {0};
        size_t data_size = std::min(sizeof(data_arr), size - i);
        memcpy(data_arr, &buffer[i], data_size);

        uint8_t bits_7 = data_arr[0];

        for (size_t j = 0; j < data_size - 1; j++) {
            data_arr[j] = data_arr[j + 1] | ((bits_7 >> j) << 7);
        }

        memcpy(&buffer[result_size], data_arr, data_size - 1);
        result_size += data_size - 1;
    }

    return result_size;
}

size_t convert_to_midi_data(uint8_t* buffer, ssize_t size) {
    uint8_t* obuffer = NULL;
    size_t result_size = 0;

    // I think this malloc is ok to do here.
    obuffer = reinterpret_cast<uint8_t*>(malloc(size * sizeof(*buffer)));
    if (NULL == obuffer) {
        return 0;
    }

    memcpy(obuffer, buffer, size * sizeof(*buffer));

    for (size_t i = 0; i < size; i += 7) {
        uint8_t data_arr[7] = {0};
        size_t data_size = std::min(sizeof(data_arr), size - i);
        memcpy(data_arr, &obuffer[i], data_size);

        uint8_t bits_7 = 0;

        for (size_t j = 0; j < data_size; j++) {
            bits_7 |= (data_arr[j] >> 7) << j;
            data_arr[j] &= 0x7F;
        }

        buffer[result_size++] = bits_7;
        memcpy(&buffer[result_size], data_arr, data_size);
        result_size += data_size;
    }

    free(obuffer);

    return result_size;
}
