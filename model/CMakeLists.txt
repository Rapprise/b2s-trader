cmake_minimum_required(VERSION 3.5.1)
project(model)

option(ENABLE_TESTS "Build all tests." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-std=gnu++17)
add_definitions(-fPIC)

set(INCLUDE_FILES
	include/trade_configuration.h
	include/holders/strategies_settings_holder.h
	include/holders/trade_configs_holder.h
	include/holders/trade_orders_holder.h
	include/holders/trade_signaled_strategy_market_holder.h
	include/orders/orders_matching.h
	include/orders/orders_profit.h
	include/settings/app_settings.h
	include/settings/buy_settings.h
	include/settings/coin_settings.h
	include/settings/sell_settings.h
	include/settings/stock_exchange_settings.h
	include/settings/strategies_settings/bollinger_bands_advanced_settings.h
	include/settings/strategies_settings/bollinger_bands_settings.h
	include/settings/strategies_settings/custom_strategy_settings.h
	include/settings/strategies_settings/ema_settings.h
	include/settings/strategies_settings/ma_crossing_settings.h
	include/settings/strategies_settings/rsi_settings.h
	include/settings/strategies_settings/sma_settings.h
	include/settings/strategies_settings/stochastic_oscillator_settings.h
	include/settings/strategies_settings/strategy_settings.h
	include/settings/strategies_settings/strategy_settings_factory.h
	include/settings/strategies_settings/strategy_settings_visitor.h)

set(SOURCE_FILES
    src/trade_configuration.cpp
    src/orders/orders_profit.cpp
    src/orders/orders_matching.cpp
    src/holders/strategies_settings_holder.cpp
    src/holders/trade_configs_holder.cpp
    src/holders/trade_orders_holder.cpp
	src/holders/trade_signaled_strategy_market_holder.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${INCLUDE_FILES} ${SOURCE_FILES})