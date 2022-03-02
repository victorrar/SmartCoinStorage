//
// Created by Victor on 01.03.2022.
//

#ifndef SMARTCOINSTORAGE_COINSORTER_H
#define SMARTCOINSTORAGE_COINSORTER_H


#include <cstdint>
#include <vector>

class CoinSorter {
public:
    struct CoinDescriptor {
        const char *displayName;
        uint32_t value;
        uint16_t adcValue;
        uint64_t time;
    };

    CoinSorter() : descriptors() {}

    virtual void AddCoin(CoinDescriptor descriptor) = 0;

    virtual CoinDescriptor *Predict(uint64_t time, uint16_t adc) = 0;

protected:
    std::vector<CoinDescriptor> descriptors;
    bool isInit = false;
};


#endif //SMARTCOINSTORAGE_COINSORTER_H
