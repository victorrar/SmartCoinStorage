//
// Created by Victor on 23.02.2022.
//

#ifndef SMARTCOINSTORAGE_COINDETECTOR_H
#define SMARTCOINSTORAGE_COINDETECTOR_H

#include <functional>

typedef void (*HookFunc)(void);


class CoinDetector {
public:

    struct Config {
        int gpioStart;
        int gpioMeasure;
        int gpioEnd;
        uint8_t timerId;
        HookFunc hookStart;
        HookFunc hookEnd;
        HookFunc hookTimer;
        std::function<void(uint64_t micros, uint16_t adc)> callbackSuccess;
        std::function<void()> callbackFail;

    };

    enum class State {
        COIN_NOT_IN_CHANNEL,
        COIN_PASS_START_SENSOR,
        COIN_PASS_MEASURE_SENSOR,
        COIN_PASS_END_SENSOR,
    };

    explicit CoinDetector(const Config &config);

    void IRAM_ATTR itrStart();

    void IRAM_ATTR itrEnd();

    void IRAM_ATTR itrTimer();

    static const EventBits_t BIT_START = (1 << 0);
    static const EventBits_t BIT_END = (1 << 1);
    static const EventBits_t BIT_FAIL = (1 << 2);

    EventGroupHandle_t eventHandle;

    volatile State state = State::COIN_NOT_IN_CHANNEL;
    hw_timer_t *coinTimer;
    volatile uint64_t lastMicros;
protected:
    [[noreturn]] static void workerTask(void *pvParameters);

    Config config;
    volatile uint16_t lastADC;

    void init();


private:


    TaskHandle_t workerHandle;

};


#endif //SMARTCOINSTORAGE_COINDETECTOR_H
