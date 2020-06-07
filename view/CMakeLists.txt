cmake_minimum_required(VERSION 3.5.1)
project(view)

set(CMAKE_AUTOMOC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Poco_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/common)

option(ENABLE_TESTS "Build all tests." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(UNIX)
	add_definitions(-fPIC)
endif()

set(SOURCE_FILES
        src/gui_processor.cpp
        src/custom_chart_widget.cpp
        src/custom_menu.cpp
        src/custom_chart_view.cpp
        src/gui_nodes/gui_tree_node.cpp
        src/view_model/currencies_view_model.cpp
        src/view_model/strategies_view_model.cpp
        src/view_model/trading_view_model.cpp
        src/view_model/base_item_model.cpp
        src/view_model/balance_view_model.cpp
        src/view_model/trading_account_orders_view_model.cpp
        src/view_model/trading_market_orders_view_model.cpp
        src/dialogs/create_strategy_dialog.cpp
        src/dialogs/bb_settings_dialog.cpp
        src/dialogs/rsi_settings_dialog.cpp
        src/dialogs/sma_settings_dialog.cpp
        src/dialogs/ema_settings_dialog.cpp
        src/dialogs/moving_average_crossing_settings_dialog.cpp
        src/dialogs/create_trade_configuration_dialog.cpp
        src/dialogs/telegram_dialog.cpp
        src/dialogs/stop_loss_dialog.cpp
		src/dialogs/intervals_dialog.cpp
        src/context_menu/strategy_context_menu_handler.cpp
        src/context_menu/trade_config_context_menu_handler.cpp
        src/dialogs/bb_advanced_settings_dialog.cpp
        src/dialogs/stochastic_oscillator_settings_dialog.cpp
        src/dialogs/license_dialog.cpp
		src/dialogs/about_dialog.cpp)

set(UI_FILES
        ui/main_menu.ui
        ui/chart_manual.ui
        ui/create_trade_configuration_dialog.ui
        ui/bb_settings_dialog.ui
        ui/sma_settings_dialog.ui
        ui/create_strategy_dialog.ui
        ui/ema_settings_dialog.ui
        ui/create_rsi_settings_dialog.ui
        ui/moving_average_crossing_settings_dialog.ui
        ui/telegram_dialog.ui
        ui/stop_loss_dialog.ui
		ui/intervals_dialog.ui
        ui/bb_advanced_settings_dialog.ui
        ui/stochastic_oscillator_dialog.ui
        ui/license_dialog.ui
		ui/about_dialog.ui)

set(INCLUDE_FILES
        include/gui_processor.h
        include/custom_menu.h
        include/custom_chart_widget.h
        include/custom_chart_view.h
        include/gui_nodes/gui_tree_node.h
        include/view_model/currencies_view_model.h
        include/view_model/trading_view_model.h
        include/view_model/strategies_view_model.h
        include/view_model/base_item_model.h
        include/view_model/balance_view_model.h
        include/view_model/trading_account_orders_view_model.h
        include/view_model/trading_market_orders_view_model.h
        include/dialogs/create_strategy_dialog.h
        include/dialogs/create_trade_configuration_dialog.h
        include/dialogs/bb_settings_dialog.h
        include/dialogs/sma_settings_dialog.h
        include/dialogs/ema_settings_dialog.h
        include/dialogs/rsi_settings_dialog.h
        include/dialogs/moving_average_crossing_settings_dialog.h
        include/dialogs/bb_advanced_settings_dialog.h
        include/dialogs/telegram_dialog.h
        include/dialogs/stop_loss_dialog.h
		include/dialogs/intervals_dialog.h
        include/dialogs/stochastic_oscillator_dialog.h
        include/context_menu/trade_config_context_menu_handler.h
        include/context_menu/strategy_context_menu_handler.h
        include/validators/integer_validator.h
        include/validators/float_validator.h
        include/dialogs/license_dialog.h
		include/dialogs/about_dialog.h)

set(RESOURCES resources/resources.qrc)
qt5_wrap_ui(UI_GENERATED_FILES ${UI_FILES})
qt5_wrap_cpp(INCLUDE_GENERATED_FILES ${INCLUDE_FILES})

add_library(${PROJECT_NAME} STATIC ${INCLUDE_FILES} ${SOURCE_FILES} ${UI_GENERATED_FILES} ${INCLUDE_GENERATED_FILES} )