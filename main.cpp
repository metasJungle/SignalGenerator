#include <iostream>
#include <cstring>
#include <complex>
#include <vector>
#include <unistd.h>
#include <../include/libhackrf/hackrf.h>

typedef std::complex<float> complex_t;
complex_t _table[0xffff + 1];

void Initializer()
{
    const float scale = 1.0f / 128.0f;

    // create a lookup table for complex values
    for (uint32_t i = 0; i <= 0xffff; ++i)
    {
        float re = (float(i & 0xff) - 127.5f) * scale,
              im = (float(i >> 8) - 127.5f) * scale;

        _table[i] = complex_t(re, im);
    }
}

static int rx_callback(hackrf_transfer *transfer);

int rx_callback(hackrf_transfer *transfer)
{

    // std::cout << "transfer->buffer_length: " << transfer->buffer_length << std::endl;
    // for (size_t i = 0; i < 10; i++)
    // {
    //     std::cout << transfer->buffer[i] << " ";
    // }
    // std::cout << std::endl;

    const uint8_t *buffer = transfer->buffer;
    uint32_t size = transfer->valid_length;

    uint16_t *p = (uint16_t *)buffer;
    size_t i, len = size / sizeof(uint16_t);

    std::vector<complex_t> values;
    for (i = 0; i < len; ++i)
    {
        uint16_t val = p[i];
        complex_t comp = _table[val];
        values.push_back(comp);
    }

    std::cout << "New Packet loading..." << std::endl;

    for (size_t i = 10000; i < 10010; i++)
    {
        std::cout << values[i].real() << " " << values[i].imag() << std::endl;
    }

    return 0;
}

static int tx_callback(hackrf_transfer *transfer);

int tx_callback(hackrf_transfer *transfer)
{
    int len_char = 262144;
    transfer->valid_length = len_char;
    transfer->buffer_length = len_char;

    int len_short = len_char / 2;
    uint16_t *p = new uint16_t[len_short];



    for (size_t i = 0; i < len_short; i++)
    {
        // p[i] = 65534;
        p[i] = rand()*60000;
    }
    memcpy(transfer->buffer, p, len_char);
    delete p;
    return 0;
}
int main()
{

    Initializer();

    int err = HACKRF_SUCCESS;
    err = hackrf_init();

    std::cout << "hackrf_init(): " << hackrf_error_name((hackrf_error)err) << std::endl;

    // hackrf_device_list_t* hackrf_list = new hackrf_device_list_t();
    // hackrf_list = hackrf_device_list();

    // for (size_t i = 0; i < hackrf_list->devicecount; i++)
    // {
    //     std::cout << hackrf_list->usb_device_index << std::endl;
    // }

    hackrf_device *device;
    err = hackrf_open(&device);
    std::cout << "hackrf_open(): " << hackrf_error_name((hackrf_error)err) << std::endl;

    uint32_t sampling_Rate = 500e3;
    err = hackrf_set_sample_rate_manual(device, sampling_Rate, 1);
    std::cout << "hackrf_set_sample_rate_manual(): " << hackrf_error_name((hackrf_error)err) << std::endl;

    uint64_t freq = 100e6;
    err = hackrf_set_freq(device, freq);
    std::cout << "hackrf_set_freq(): " << hackrf_error_name((hackrf_error)err) << std::endl;

    /* start hacking */
    hackrf_sample_block_cb_fn hackrf_rx_callback;

    // err = hackrf_start_rx(device, rx_callback, NULL);
    // std::cout << "hackrf_start_rx(): " << hackrf_error_name((hackrf_error)err) << std::endl;

    err = hackrf_start_tx(device, tx_callback, NULL);
    std::cout << "hackrf_start_tfx(): " << hackrf_error_name((hackrf_error)err) << std::endl;

    int sleepCnt = 0;
    int sleepCntLim = 1000;
    while (hackrf_is_streaming(device) == HACKRF_TRUE)
    {
        usleep(10000);
        if (++sleepCnt == sleepCntLim)
            break;

        //std::cout << "Sleeping 100 microseconds." << std::endl;
    }

    err = hackrf_stop_rx(device);
    std::cout << "hackrf_stop_rx(): " << hackrf_error_name((hackrf_error)err) << std::endl;

    std::cout << "Sleeping finished." << std::endl;

    err = hackrf_close(device);
    std::cout << "hackrf_close(): " << hackrf_error_name((hackrf_error)err) << std::endl;

    return 0;
}