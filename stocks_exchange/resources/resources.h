/*
 * Copyright (c) 2020, Rapprise.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef STOCKS_EXCHANGE_RESOURCES_H
#define STOCKS_EXCHANGE_RESOURCES_H

#include <string>

namespace auto_trader {
namespace stock_exchange {
namespace resources {

namespace bittrex {

const std::string BITTREX_URL_INTERNATIONAL = "https://international.bittrex.com/";
const std::string BITTREX_URL = "https://api.bittrex.com/";
const std::string BITTREX_ADDRESS = "https://bittrex.com/";
const std::string BITTREX_PUBLIC_URL_API = "api/v1.1/public/";
const std::string BITTREX_MARKET_URL_API = "api/v1.1/market/";
const std::string BITTREX_ACCOUNT_API = "api/v1.1/account";
const std::string BITTREX_MARKET_OPEN_ORDERS = "api/v1.1/market/getopenorders";
const std::string BITTREX_API_V_0_MARKET = "Api/v2.0/pub/market/";
const std::string MARKET_HISTORY_API = "getmarkethistory?market";
const std::string BITTREX_GET_TICKS = "GetTicks?";
const std::string BITTREX_MARKET_NAME = "marketName";
const std::string BITTREX_ORDER_BOOK = "getorderbook";
const std::string BITTREX_TICK_INTERVAL = "tickInterval";
const std::string BITTREX_CURRENCIES = "getcurrencies";
const std::string BITTREX_SELL_LIMIT = "selllimit";
const std::string BITTREX_BUY_LIMIT = "buylimit";
const std::string BITTREX_TICKER = "getticker";
const std::string BITTREX_CANCEL = "cancel";
const std::string BITTREX_MARET = "market";
const std::string BITTREX_OPEN_ORDERS = "getopenorders";
const std::string BITTREX_GET_BALANCE = "getbalances";
const std::string BITTREX_GET_ORDER = "getorder";
const std::string BITTREX_ASK_TICK = "Ask";
const std::string BITTREX_BID_TICK = "Bid";
const std::string MARKET_HISTORY_TIMESTAMP = "T";

const std::string MARKET_HISTORY_CLOSE_POSITION = "C";
const std::string MARKET_HISTORY_OPEN_POSITION = "O";
const std::string MARKET_HISTORY_HIGH_POSITION = "H";
const std::string MARKET_HISTORY_LOW_POSITION = "L";
const std::string MARKET_HISTORY_VOLUME = "BV";
const std::string BITTREX_ORDER_UUID = "OrderUuid";

const std::string BITTREX_BALANCE_CURRENCY = "Currency";
const std::string BITTREX_BALANCE_VALUE = "Balance";
const std::string BITTREX_PRICE_KEYWORD = "Price";
const std::string BITTREX_QUANTITY_KEYWORD = "Quantity";
const std::string BITTREX_RATE_KEYWORD = "Rate";
const std::string BITTREX_EXCHANGE_KEYWORD = "Exchange";
const std::string BITTREX_TYPE_KEYWORD = "Type";
const std::string BITTREX_ORDER_TYPE_KEYWORD = "OrderType";
const std::string BITTREX_DATE_OPENED_KEYWORD = "Opened";
const std::string BITTREX_OPEN_ORDERS_BUY_KEYWORD = "buy";
const std::string BITTREX_OPEN_ORDERS_SELL_KEYWORD = "sell";
const std::string BITTREX_IS_CANCEL_ORDER_KEYWORD = "CancelInitiated";

const std::string BITTREX_HEADER_APISIGN = "apisign";

}  // namespace bittrex

namespace binance {

const std::string BINANCE_URL = "https://api.binance.com/";
const std::string BINANCE_KLINES = "api/v1/klines?";
const std::string BINANCE_TIME = "api/v1/time";
const std::string BINANCE_TRADE_LIST = "api/v1/depth?";
const std::string BINANCE_ORDER = "api/v3/order?";
const std::string BINANCE_OPEN_ORDERS = "api/v3/openOrders?";
const std::string BINANCE_CANCEL_ORDER = "api/v3/order?";
const std::string BINANCE_ACCOUNT_INFO = "api/v3/account?";
const std::string BINANCE_ALL_ORDERS = "api/v3/allOrders?";
const std::string BINANCE_SERVER_TIME = "api/v1/time";
const std::string BINANCE_CURRENTY_TICK = "api/v3/ticker/bookTicker?";
const std::string BINANCE_EXCHANGE_INFO = "api/v1/exchangeInfo";

const std::string BINANCE_INTERVAL = "interval";
const std::string BINANCE_SERVER_TIME_VALUE = "serverTime";
const std::string BINANCE_SIDE = "side";
const std::string BINANCE_BALANCE_ARRAY_BLOCK = "balances";
const std::string BINANCE_ORDER_TYPE_NAME = "type";
const std::string BINANCE_SIGNATURE = "signature";
const std::string BINANCE_CURRENCY_FREE = "free";
const std::string BINANCE_ORDER_TYPE_LIMIT = "LIMIT";
const std::string BINANCE_ORDER_TYPE_MARKET = "MARKET";
const std::string BINANCE_TIME_IN_FORCE = "timeInForce";
const std::string BINANCE_GTC = "GTC";
const std::string BINANCE_ORIG_CLIENT_ORDER_ID = "origClientOrderId";
const std::string BINANCE_CLIENT_ORDER_ID = "clientOrderId";
const std::string BINANCE_ORDER_ID = "orderId";
const std::string BINANCE_CURRENCY_PRICE_FIELD = "price";
const std::string BINANCE_X_MBX_APIKEY = "X-MBX-APIKEY";
const std::string BINANCE_IS_WORKING_ORDER = "isWorking";
const std::string BINANCE_ORDER_TYPE = "side";
const std::string BINANCE_OPEN_ORDER_QUANTITY = "qty";
const std::string BINANCE_ORDER_ORIG_QUANTITY = "origQty";
const std::string BINANCE_RECIEVE_WINDOW = "recvWindow";
const std::string BINANCE_BID_PRICE_KEYWORD = "bidPrice";
const std::string BINANCE_ASK_PRICE_KEYWORD = "askPrice";
const std::string BINANCE_EXCHANGE_INFO_SYMBOLS = "symbols";
const std::string BINANCE_EXCHANGE_INFO_FILTERS = "filters";
const std::string BINANCE_EXCHANGE_INFO_FILTER_TYPE_KEYWORD = "filterType";
const std::string BINANCE_LOT_SIZE_KEYWORD = "LOT_SIZE";
const std::string BINANCE_LOT_SIZE_MIN_QUANTITY_KEYWORD = "minQty";
const std::string BINANCE_LOT_SIZE_MAX_QUANTITY_KEYWORD = "maxQty";
const std::string BINANCE_LOT_SIZE_STEP_SIZE_KEYWORD = "stepSize";

const int BINANCE_RECIEVE_WINDOW_VALUE = 50000;
const int BINANCE_TIMESTAMP_INDEX = 0;
const int BINANCE_OPEN_PRICE_INDEX = 1;
const int BINANCE_HIGH_PRICE_INDEX = 2;
const int BINANCE_LOW_PRICE_INDEX = 3;
const int BINANCE_CLOSE_PRICE_INDEX = 4;
const int BINANCE_VOLUME_INDEX = 5;
const int BINANCE_CLOSE_TIME_INDEX = 6;
const int BINANCE_INSUFFICIENT_BALANCE_CODE = -2010;

}  // namespace binance

namespace kraken {

const std::string KRAKEN_API_URI = "https://api.kraken.com";
const std::string KRAKEN_SLASH_ZERO_SLASH_REQUEST = "/0/";
const std::string KRAKEN_PUBLIC_METHODS = "public";
const std::string KRAKEN_PRIVATE_METHODS = "private";
const std::string KRAKEN_MARKET_HISTORY = "OHLC";
const std::string KRAKEN_MARKET_OPENED_ORDERS = "Depth";
const std::string KRAKEN_NEW_ORDER_KEYWORD = "AddOrder";
const std::string KRAKEN_TRADE_BALANCE_KEYWORD = "TradeBalance";
const std::string KRAKEN_ACCOUNT_BALANCE = "Balance";
const std::string KRAKEN_MARKET_TYPE = "ordertype";
const std::string KRAKEN_ORDER_TYPE = "type";
const std::string KRAKEN_BUY_ORDER = "buy";
const std::string KRAKEN_SELL_ORDER = "sell";
const std::string KRAKEN_OPEN_ORDER_DESCRIPTION = "descr";
const std::string KRAKEN_OPEN_ORDER_QUANTITY = "vol";
const std::string KRAKEN_ORDER_UUID_KEYWORD = "txid";
const std::string KRAKEN_CANCEL_ORDER_KEYWORD = "CancelOrder";
const std::string KRAKEN_OPEN_POSITIONS_KEYWORD = "OpenPositions";
const std::string KRAKEN_GET_CLOSED_ORDERS_KEYWORD = "ClosedOrders";
const std::string KRAKEN_OPEN_ORDERS_KEYWORD = "OpenOrders";
const std::string KRAKEN_TICKER_KEYWORD = "Ticker";
const std::string KRAKEN_CURRENCY_PAIR_KEYWORD = "pair";
const std::string KRAKEN_ORDER_STATUS_KEYWORD = "status";
const std::string KRAKEN_INTERVAL_KEYWORD = "interval";
const std::string KRAKEN_GET_OPENED_ORDERS_LIST_KEYWORD = "open";
const std::string KRAKEN_GET_CLOSED_ORDERS_LIST_KEYWORD = "closed";
const std::string KRAKEN_NONCE = "nonce";
const std::string KRAKEN_TICKER_TODAY_OPEN_PRICE = "o";
const std::string KRAKEN_PRICE_TICKER_ASK = "a";
const std::string KRAKEN_PRICE_TICKER_BID = "b";
const std::string KRAKEN_ORDER_CANCELED_STATUS = "canceled";
const std::string KRAKEN_ORDER_OPEN_TIME_KEYWORD = "opentm";
const std::string KRAKEN_TAKE_PROFIT = "take-profit";

const std::string KRAKEN_API_KEY_HEADER = "API-Key";
const std::string KRAKEN_PRIVATE_KEY_HEADER = "API-Sign";

const int KRAKEN_PRICE_TIKER_INDEX = 0;

const int KRAKEN_TIME_INDEX = 0;
const int KRAKEN_OPEN_PRICE_INDEX = 1;
const int KRAKEN_HIGH_PRICE_INDEX = 2;
const int KRAKEN_LOW_PRICE_INDEX = 3;
const int KRAKEN_CLOSE_PRICE_INDEX = 4;
const int KRAKEN_VOLUME_INDEX = 6;

}  // namespace kraken

namespace poloniex {

const std::string POLONIEX_PUBLIC_ENDPOINT = "https://poloniex.com/public";
const std::string POLONIEX_PRIVATE_ENDPOINT = "https://poloniex.com/tradingApi";

const std::string POLONIEX_RETURN_TICKER = "returnTicker";
const std::string POLONIEX_CURRENCY_TICK_KEYWORD = "baseVolume";
const std::string POLONIEX_CURRENCY_PAIR_KEYWORD = "currencyPair";
const std::string POLONIEX_CHART_DATA_KEYWORD = "returnChartData";
const std::string POLONIEX_OPEN_ORDERS_KEYWORD = "returnOpenOrders";
const std::string POLONIEX_TICKER_KEYWORD = "returnOrderBook";
const std::string POLONIEX_TRADE_HISTORY_KEYWORD = "returnTradeHistory";
const std::string POLONIEX_GET_BALANCE_KEYWORD = "returnBalances";
const std::string POLONIEX_GET_ORDER_TRADES_KEYWORD = "returnOrderTrades";
const std::string POLONIEX_GET_ORDER_STATUS_KEYWORD = "returnOrderStatus";
const std::string POLONIEX_CANCEL_ORDER_KEYWORD = "cancelOrder";
const std::string POLONIEX_DEPTH_KEYWORD = "depth";

const std::string POLONIEX_START_TIME_KEYWORD = "start";
const std::string POLONIEX_END_TIME_KEYWORD = "end";
const std::string POLONIEX_BUY_KEYWORD = "buy";
const std::string POLONIEX_SELL_KEYWORD = "sell";
const std::string POLONIEX_ALL_MARKETS = "all";
const std::string POLONIEX_TIME_INTERVAL_KEYWORD = "period";
const std::string POLONIEX_TIMESTAMP_KEYWORD = "nonce";
const std::string POLONIEX_ORDER_NUMBER_ID = "orderNumber";

const std::string POLONIEX_HEADER_API_KEY_KEYWORD = "Key";
const std::string POLONIEX_HEADER_SIGNATURE_KEYWORD = "Sign";
const std::string POLONIEX_CONTENT_TYPE_HEADER = "Content-Type";

const std::string POLONIEX_CURRENCY_PAIR_ALL_FIELD = "all";
const std::string POLONIEX_HIGH_PRICE = "high";
const std::string POLONIEX_LOW_PRICE = "low";
const std::string POLONIEX_OPEN_PRICE = "open";
const std::string POLONIEX_CLOSE_PRICE = "close";
const std::string POLONIEX_VOLUME_PRICE = "volume";
const std::string POLONIEX_DATE = "date";
const std::string POLONIEX_GLOBAL_TRADE_ID = "globalTradeID";
const std::string POLONIEX_CLIENT_ORDER_ID = "clientOrderId";

const int THREE_MONTH_IN_SECOND = 7776000;
}  // namespace poloniex

namespace huobi {

const std::string HUOBI_PUBLIC_URL = "https://api.huobi.pro/market";
const std::string HUOBI_PRO_API_URL = "api.huobi.pro";
const std::string HUOBI_HTTPS_API_URL = "https://api.huobi.pro";
const std::string HUOBI_CURRENCY_TICK_KEYWORD = "depth";
const std::string HUOBI_ORDERS_REQUEST = "v1/order/orders";
const std::string HUOBI_OPEN_ORDERS_REQUEST = "v1/order/openOrders";
const std::string HUOBI_ACCOUNT_REQUEST = "v1/account/accounts";
const std::string HUOBI_MARKET_OPEN_ORDERS_KEYWORD = "history/trade";
const std::string HUOBI_GET_MARKET_HISTORY_KEYWORD = "history/kline";
const std::string HUOBI_GET_BALANCE_FIRST_PART = "/account/accounts";
const std::string HUOBI_TIMESTAMP_URL = "v1/common/timestamp";
const std::string HUOBI_COMMON_SYMBOLS = "v1/common/symbols";
const std::string HUOBI_GET_BALANCE_SECOND_PART = "balance";
const std::string HUOBI_PLACE_KEYWORD = "place";
const std::string HUOBI_MERKET_HISTORY_PERIOD_KEYWORD = "period";
const std::string HUOBI_CURRENCY_TICK_STEP = "step0";
const std::string HUOBI_SUBMIT_CANCEL = "submitcancel";
const std::string HUOBI_LIST_KEYWORD = "list";
const std::string HUOBI_CURRENCY_KEYWORD = "currency";
const std::string HUOBI_BUY_LIMIT = "buy-limit";
const std::string HUOBI_SELL_LIMIT = "sell-limit";
const std::string HUOBI_ID_KEYWORD = "id";
const std::string HOUBI_ACCOUNT_ID = "account-id";
const std::string HUOBI_SIGNATURE = "Signature";
const std::string HUOBI_CREATED_AT = "created-at";
const std::string HUOBI_POST = "POST";
const std::string HUOBI_GET = "GET";
const std::string HUOBI_AMOUNT_PRECISION = "amount-precision";
const std::string HUOBI_PRICE_PRECISION = "price-precision";

constexpr char HUOBI_CONTENT_TYPE[] = "Content-Type:application/json;charset=UTF-8";

const std::string HUOBI_API_KEY = "AccessKeyId";
const std::string HUOBI_SIGNATURE_METHOD = "SignatureMethod=HmacSHA256";
const std::string HUOBI_SIGNATURE_VERSION = "SignatureVersion=2";
const std::string HUOBI_TIMESTAMP = "Timestamp";

const std::string HUOBI_STATUS_OK = "ok";
const std::string HUOBI_ERROR_CODE_KEY = "err-code";

const std::string HUOBI_CURRENCY_TICK_BLOCK = "tick";
const std::string HUOBI_ORDERS_DATA_KEYWORD = "data";
const std::string HUOBI_TIMESTAMP_KEYWORD = "ts";
const std::string HUOBI_ORDER_TIME_KEYWORD = "direction";

const std::string HUOBI_CANDLE_TIMESTAMP = "id";
const std::string HUOBI_CANDLE_OPEN_PRICE = "open";
const std::string HUOBI_CANDLE_CLOSE_PRICE = "close";
const std::string HUOBI_CANDLE_LOW_PRICE = "low";
const std::string HUOBI_CANDLE_HIGH_PRICE = "high";
const std::string HUOBI_CANDLE_VOLUME_PRICE = "vol";

const short HUOBI_MARKET_OPENED_ORDERS = 50;
}  // namespace huobi

namespace words {

const std::string BUY_SIDE = "BUY";
const std::string SELL_SIDE = "SELL";
const std::string BUY_SIDE_LOWER_CASE = "buy";
const std::string SELL_SIDE_LOWER_CASE = "sell";
const std::string SYMBOL = "symbol";
const std::string TIMESTAMP = "timestamp";
const std::string TIME = "time";
const std::string SUCCESS = "success";
const std::string ERROR_WORD = "error";
const std::string MESSAGE = "message";
const std::string MESSAGE_SHORT = "msg";
const std::string CODE = "code";
const std::string RESULT = "result";
const std::string CURRENCY = "Currency";
const std::string SIZE = "size";
const std::string UUID = "uuid";
const std::string QUANTITY = "quantity";
const std::string RATE = "rate";
const std::string API_KEY = "apikey";
const std::string PRICE = "price";
const std::string STATUS = "status";
const std::string CANCELED = "canceled";
const std::string MARKET = "market";
const std::string VOLUME = "volume";
const std::string BALANCE_ASSET = "asset";
const std::string NONCE = "nonce";
const std::string HASH = "hash";
const std::string TYPE = "type";
const std::string BOTH = "both";
const std::string BIDS = "bids";
const std::string ASKS = "asks";
const std::string CANCELED_UPERCASE = "CANCELED";
const std::string COUNT = "count";
const std::string COMMAND = "command";
const std::string AMOUNT = "amount";
const std::string TIME_T_SYMBOL = "T";

}  // namespace words

namespace symbols {

const std::string AND = "&";
const std::string DASH = "-";
const std::string EMPTY_STR = "";
const std::string EQUAL = "=";
const std::string SLASH = "/";
const std::string REVERSE_SLASH = "\\";
const std::string SPACE = " ";
const std::string QUESTION = "?";
const std::string UNDER_LINE = "_";
const std::string DOT = ".";
const std::string NEW_LINE_SYMBOL = "\n";
const std::string COLON = ":";
const std::string COMMA = ",";
const std::string DOUBLE_QUOTES = "\"";

const char LEFT_CURLY_BRACE = '{';
const char RIGHT_CURLY_BRACE = '}';
const char LEFT_SQUARE_BRACKET = '[';

}  // namespace symbols

namespace messages {

const std::string BINANCE_INSUFFICIENT_BALANCE_MESSAGE =
    "Invalid response exception raised : Account has insufficient balance for requested action.";
const std::string BINANCE_UNKNOWN_ORDER_SENT =
    "Invalid response exception raised : Unknown order sent.";
const std::string FAILED_TO_UPLOAD_MESSAGE = "Failed to upload:";

const std::string BITTREX_INSUFFICIENT_FUNDS =
    "Invalid response exception raised : INSUFFICIENT_FUNDS";
const std::string BITTREX_INVALID_ORDER = "INVALID_ORDER";

const std::string HUOBI_UNKNOWN_ORDER_STATE =
    "Invalid response exception raised : order-orderstate-error";
const std::string HUOBI_INVALID_BALANCE =
    "Invalid response exception raised : account-frozen-balance-insufficient-error";

}  // namespace messages

namespace numbers {

const int ONE = 1;
const int FIRST_ARRAY_INDEX = 0;
const int SECOND_ARRAY_INDEX = 1;
const int MAX_MARKET_ORDERS_COUNT = 60;

}  // namespace numbers

namespace keywords {
const std::string STOCK_EXCHANGE_TYPE = "Stock exchange type";
}

}  // namespace resources
}  // namespace stock_exchange
}  // namespace auto_trader

#endif  // STOCKS_EXCHANGE_RESOURCES_H
