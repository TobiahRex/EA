let pips;
const TakeProfitPips = 100;
const EOWCO_TIME = '16:00';

let EntryPrice = 0;
let ExitPrice = 0;
let stopLoss = 0;
let ATR;


const onInit = () => {
  pips = Point;
  if (Digits === 3 || Digits === 5) {
    pips *= 10;
  }
  console.info("Expert Loaded Successfully");
  return;
}

// const onDeinit = reason => {
//   console.log(' ');
// }

// const start = () => {
//
// }

const onTick = () => {

  let volume;
  let stopLoss = 0;
  let TakeProfit = 0;
  let StopsLevel = 0;
  let OrdersCounter = 0;
  let candleTime = Date.now;
  let totalOrders = OrdersTotal(); // returns array;

  // -------------------- Iterate over all Open Orders --------------------

  for (let i = totalOrders - 1; i > -1; i--) {

    // Filter Orders that have a ticket number.
    if (OrderSelect(i, SELECT_BY_POS) && OrderSymbol() === _Symbol &&
    OrderMagicNumber() === MagicNumber && OrderCloseTime() === 0) {

      /* Calculate profit target:
      find the current open price.
      add the users desired profit target to that price.
      */
      TakeProfit = OrderOpenPrice() + (TakeProfitPips * pips);
      console.log('Take Profit = ', OrderTakeProfit());

      refreshRate();

      /*
      Calculate stop level:
      Find the max value between the current spread,
      or the current stop.
      NOTE: this may need to be refactored.
      */

      /* SYMBOL_TRADE_STOPS_LEVEL =
      minimal indention in points for Stop Orders.
      IOW, it's the minimum value that the price has to cross over the stop loss,
      for the stop loss to trigger.
      NOTE: this should be changed to find a way for the trade to not close,
      unless the current bar's close is less than the stop loss.
      */
      StopsLevel = Math.max(
        SymbolInfoInteger(_Symbol, SYMBOL_SPREAD),  // SymbolInfoInteger provides details on the currently selected symbol.  The second argument returns only those specific variables.
        SymbolInfoInteger(_Symbol, SYMBOL_TRADE_STOPS_LEVEL)
      ) * pips;


      // Modifications for Buy Orders.
      if (OrderType() === OP_BUY) {
        EntryPrice = Ask;
        ExitPrice = Bid;

        // If end of the week close out option has been selected.
        if (EndofWeekCloseOut && checkEOWC()) {
          if (OrderClose(OrderTicket(), OrderLots(), ExitPrice, Slippage, clrRed)){
            console.log('BUY to Close by EndOfWeekCloseOut COMPLETED.');
          } else {
            console.log('BUY to Close by EndOfWeekCloseOut ERROR: ', ErrorDescription(GetLastError()));
          }
        }

        if (TakeProfitPips > 0 && OrderTakeProfit() === 0) {
          if ((TakeProfit - EntryPrice) > StopsLevel) {
            if (OrderModify(OrderTicker(), OrderOpenPrice(), OrderStopLoss(), TakeProfit, 0)){
              console.log('BUY Modified by TakeProfit COMPLETED.');
            } else {
              console.log('BUY Modified by TakeProfit ERROR: ', ErrorDescription(GetLastError()));
            }
          }
        }

        // This check verifies that the price is within the minimum level (StopsLevel)
        // of tolerance to execute a closing Order.
        if (StopLoss > 0 && OrderStopLoss() === 0) {
          StopLoss = OrderOpenPrice() - (StopLossPips * pips);

          if (ExitPrice - StopLoss > StopsLevel) {
            if (OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0)) {
              console.log('BUY Modified by StopLoss COMPLETED');
            } else {
              console.log('BUY Modified by StopLoss ERROR: ', ErrorDescription(GetLastError()));
            }
          }
        }

        if (TrailStopATR) {
          ATR = iCustom(null, 0, 'ATR_TRAILSTOP_V4', ATR_BackPeriod, ATR_SecondaryTF, ATR_period, ATR_factor, 0, 0);
          StopLoss = ATR;

          if (StopLoss > 0 &&  (StopLoss - OrderStopLoss() > 0 || OrderStopLoss() === 0)) {
            if (OrderModify(OrderTicker(), OrderOpenPrice(), StopLoss, TakeProfit, 0)) {
              console.log('BUY Modified by TrailStopATR COMPLETED.');
            } else {
              console.log('BUY Modified by TrailStopATR ERROR: ', ErrorDescription(GetLastError()));
            }
          }

          if (TrailStopPreviousClose) {
            StopLoss = Close[1] - (TrailStopSize * pips);

            if (ExitPrice - StopLoss > StopsLevel && (StopLoss - OrderStopLoss() > 0) || OrderStopLoss() === 0) {
              if (OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0)) {
                console.log('BUY Modified by TrailStopPreviousClose COMPLETED');
              } else {
                console.log('BUY Modified by TrailStopPreviousClose ERROR: ', ErrorDescription(GetLastError()));
              }
            }
          }

          if (TrailStopStandard) {
            StopLoss = ExitPrice - (TrailStopSize * pips);

            if ((ExitPrice - StopLoss) > StopsLevel && (StopLoss - OrderStopLoss() > 0 || OrderStopLoss() === 0)) {
              if (OrderModify(OrderTicker(), OrderOpenPrice(), StopLoss, TakeProfit, 0)) {
                console.log('BUY Modified by TrailStopStandard COMPLETED');
              } else {
                console.log('BUY Modified by TrailStopStandared ERROR: ', ErrorDescription(GetLastError()));
              }
            }
          }

          orders_counter++;
        }

      } // if Order with ticket number is a BUY Order.

      if (OrderType() === OP_SELL) {
        EntryPrice = Bid;
        ExitPrice = Ask;

        if (EndOfWeekCloseOut && CheckEOWC()) {
          if (OrderClose(OrderTicket(), OrderLots(), ExitPrice, Slippage, clrRed)) {
            console.log('SELL Closed by EndOfWeekCloseOut COMPLETED');
          } else {
            console.log('SELL Close by EndOfWeekCloseOut ERROR: ', ErrorDescription(GetLastError()));
          }
        }

        if (TakeProfitPips > 0 && OrderTakeProfit() === 0) {
          if (EntryPrice - TakeProfit < StopSLevel) {
            if (OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), TakeProfit, 0)) {
              console.log('SELL Modified by TakeProfit COMPLETED');
            } else {
              console.log('SELL Modified by TakeProfit ERROR: ', ErrorDescription(GetLastError()));
            }
          }
        }

        if (StopLossPips > 0 && OrderStopLoss() === 0) {
          StopLoss = OrderOpenPrice() + (StopLossPips * pips);

          if ((StopLoss - ExitPrice) > StopsLevel) {
            if (OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0)) {
              console.log('SELL Modified by StopLoss COMPLETED');
            } else {
              console.log('SELL Modified by StopLoss ERROR');
            }
          }
        }

        if (TrailStopATR) {
          ATR = iCustom(NULL, 0, 'ATR_TRAILSTOP_V4', ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 1, 0);
          StopLoss = ATR;

          if (StopLoss > 0 && (OrderStopLoss() - StopLoss > 0) || OrderStopLoss() === 0) {
            if (OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0)) {
              console.log('SELL Modified by TrailStopATR COMPLETED');
            } else {
              console.log('SELL Modification by TrailStopATR ERROR: ', ErrorDescription(GetLastError()));
            }
          }

          if (TrailStopPreviousClose) {
            StopLoss = Close[1] + (TrailStopSize * pips);

            if ((StopLoss - ExitPrice) > StopsLevel && (OrderStopLoss() - StopLoss) > 0 || OrderStopLoss() === 0) {
              if (OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0)) {
                console.log('SELL Modified by TrailStopPreviousClose COMPLETED');
              } else {
                console.log('SELL Modification by TrailStopPreviousClose ERROR: ', ErrorDescription(GetLastError()));
              }
            }
          }

          if (TrailStopStandard) {
            StopLoss = ExitPrice + (TrailStopSize * pips);

            if ((StopLoss - ExitPrice) > StopsLevel && (OrderStopLoss() - StopLoss) > 0 || OrderStopLoss() === 0) {
              if (OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0)) {
                console.log('SELL Modified by TrailStopStandard COMPLETED');
              } else {
                console.log('SELL Modified by TrailStopStanddard ERROR: ', ErrorDescription(GetLastError()));
              }
            }
          }
        }

        orders_counter++
      }

    }  // Orders with ticket number;

  } // end of for loop;

  if (CandleTime !== Time[0]) {
    CandleTime = Time[0];

    if (orders_counter < MaxOpenTrades && !CheckEOWC()) {
      if (GetSignal() === OP_BUY) {
        RefreshRates();
        EntryPrice = Ask;
        ATR = iCustom(NULL, 0, 'ATR_TRAILSTOP_V4', ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 0, 0);
        vol = GetVolume();
        Comment('Volume = ', GetVolume());

        if (EntryPrice > ATR) {
          if (OrderSend(_Symbol, OP_BUY, vol, EntryPrice, Slippage, 0, 0, '', MagicNumber, 0, clrCyan) > 0) {
            console.log('BUY order SENT!');
          } else {
            console.log('BUY order Send ERROR: ', ErrorDescription(GetLastError()));
          }
        }
      }

      if (GetSignal() === OP_SELL) {
        EntryPrice = Bid;
        ATR = iCustom(NULL, 0, 'ATR_TRAILSTOP_V4', ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 0, 0);
        vol = GetVolume();
        console.log('Volume = ', GetVolume());
        if (EntryPrice > ATR) {
          if (OrderSend(_Symbol, OP_SELL, vol, EntryPrice, Slippage, 0, 0, '', MagicNumber, 0, clrPink) > 0) {
            console.log('SELL order SENT!');
          } else {
            console.log('SELL order send ERROR: ', ErrorDescription(GetLastError()));
          }
        }
      }

    }
  }

// Check End of the Week Close out schedule.

  const checkEOWC = () => {

    let eowc_time = new Date()
    eowc_time = eowc_time.splice(0, 14) + ' ' + EOWC_TIME;

    if (DayOfWeek() === 5 && TimeCurrent() >= eowc_time) return(true);
    return(false);
  }

  const getVolume = () => {
    let result = SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN);
    let LotDigits = 2;

    if (quantityMode === STATIC) {
      result *= quantityStatic;
    }

    if (quantityMode === DYNAMIC) {
      result = (AccountFreeMargin() * RiskPercent) / SymbolInfoDouble(_Symbol, SYMBOL_TRADE_CONTRACT_SIZE);
    }

    if (SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN) === 0.01) {
      lotDigits = 2;
    } else if (SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN === 0.1)) {
      lotDigits = 1;
    } else if (SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN) === 1) {
      lotDigits = 0;
    }

    return ( Math.max(NormalizeDouble(result, lotDigits), SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN)))
  }

  const getSignal = () => {
    let result = -1;

    MACDvalue = iMACD(_Symbol, _Period, MACD_V1, MACD_V2, MACD_AVG, PRICE_CLOSE, MODE_MAIN, 0);
    MACDaverage = iMACD(_Symbol, _Period, MACD_V1, MACD_V2, MACD_AVG, PRICE_CLOSE, MODE_SIGNAL, 0);



  }


}
