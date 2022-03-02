#ifndef SMARTCOINSTORAGE_ALGOSORTER_H
#define SMARTCOINSTORAGE_ALGOSORTER_H

#include <esp_attr.h>
#include <esp32-hal-timer.h>
#include "CoinSorter.h"

class AlgoSorter : public CoinSorter {
public:
    AlgoSorter();

    virtual ~AlgoSorter();

    void AddCoin(CoinDescriptor descriptor) override;

    void Init();

    CoinDescriptor *Predict(uint64_t time, uint16_t adc) override;

private:
    static std::vector<AlgoSorter *> instances;
};


#endif //SMARTCOINSTORAGE_ALGOSORTER_H
