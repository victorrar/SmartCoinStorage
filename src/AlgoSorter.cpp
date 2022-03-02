//
// Created by Victor on 01.03.2022.
//

#include <esp32-hal-log.h>
#include <bits/stdc++.h>

#include "AlgoSorter.h"

std::vector<AlgoSorter *> AlgoSorter::instances;

void AlgoSorter::AddCoin(CoinDescriptor descriptor) {
    if (isInit) {
        log_e("Already init");
        return;
    }


    descriptors.push_back(descriptor);
}

void AlgoSorter::Init() {
    std::sort(descriptors.begin(),
              descriptors.end(),
              [](CoinDescriptor i1, CoinDescriptor i2) {
                  return i1.adcValue < i2.adcValue;
              });


    isInit = true;
}

CoinSorter::CoinDescriptor *AlgoSorter::Predict(uint64_t time, uint16_t adc) {

    if (!isInit) {
        log_e("Not init");
        return nullptr;
    }

    CoinDescriptor *minDescriptor = nullptr;
    double minMetric = INFINITY;


    for (auto &item: descriptors) {
        double sqAdc = pow(adc - item.adcValue, 2);
        double sqTime = 0.0001 * pow(time - item.time, 2);
        double metric = 0.75 * sqAdc + 0.25 * sqTime;
        if (metric < minMetric) {
            minMetric = metric;
            minDescriptor = &item;
        }
    }

    return minDescriptor;
}

AlgoSorter::AlgoSorter() {
    AlgoSorter::instances.push_back(this);
}

AlgoSorter::~AlgoSorter() {
    std::remove(AlgoSorter::instances.begin(), AlgoSorter::instances.end(), this);
}
