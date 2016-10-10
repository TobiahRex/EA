//+------------------------------------------------------------------+
//|                                                    RexCorpEA.mq4 |
//|                                                evsolod@gmail.com |
//|                            http://www.emet-trading-solutions.com |
//+------------------------------------------------------------------+
#property copyright "evsolod@gmail.com"
#property link      "http://www.emet-trading-solutions.com"
#property version   "4.00"
#property strict
#property description   "[Insert Description of Expert Advisor HERE]"
//#include <stderror.mhq>
//#include <stdlib.mhq>

//+------------------------------------------------------------------+
//|       INPUT PARAMETERS                                           |
//+------------------------------------------------------------------+

enum ENUM_QUANTITY      { DYNAMIC, STATIC };
enum ENUM_YESNO         { NO, YES };
enum ENUM_ONOFF         { OFF, ON };
enum ENUM_SECURITY_TYPE { FX_PAIR, FX_YEN_PAIR, STOCK };
enum ENUM_TP_TYPE       { Pips, PipsPerTrade, Off };

input string               TradeSettings           = "===========================================";
input int                  MaxOpenTrades           = 10;
input ENUM_YESNO           EndOfWeekCloseOut       = YES;
input string               EOWCO_TIME              = "16:00";
input string               QuantitySettings        = "===========================================";
input ENUM_QUANTITY        QuantityMode            = DYNAMIC;
input double               RiskPercent             = 1;
input int                  QuantityStatic          = 1;
input string               StopLossSettings        = "===========================================";
input int                  StopLossPips            = 50;
input string               TakeProfitSettings      = "===========================================";
input int                  TakeProfitPips          = 100;
input string               TrailStopSettings       = "===========================================";
input ENUM_ONOFF           TrailStopATR            = true;
input ENUM_ONOFF           TrailStopPreviousClose  = true;
input ENUM_ONOFF           TrailStopStandard       = true;
input int                  TrailStopSize           = 25;
input string               IndicatorSettings       = "===========================================";
input int                  EMA_FAST                = 8;
input int                  EMA_MED                 = 21;
input int                  EMA_SLOW                = 55;
input int                  SMA_FAST                = 89;
input int                  RSI_PERIOD              = 14;
input int                  RSI_V1EMA_Period        = 21;
input ENUM_MA_METHOD       RSI_V1EMA_Mode          = MODE_EMA;
input int                  RSI_V2SMA_Period        = 89;
input ENUM_MA_METHOD       RSI_V2SMA_Mode          = MODE_SMA;
input int                  RSI_SMA_Period          = 55;
input ENUM_MA_METHOD       RSI_SMA_Mode            = MODE_SMA;
input int                  RSI_EMA1_Period         = 5;
input ENUM_MA_METHOD       RSI_EMA1_Mode           = MODE_EMA;
input int                  RSI_EMA2_Period         = 21;
input ENUM_MA_METHOD       RSI_EMA2_Mode           = MODE_EMA;
input int                  RSI_MOMO_V1             = 5;
input ENUM_MA_METHOD       RSI_MOMO_V1_Mode        = MODE_EMA;
input int                  RSI_MOMO_V2             = 34;
input ENUM_MA_METHOD       RSI_MOMO_V2_Mode        = MODE_EMA;
input int                  RSI_MOMO_SMA            = 34;
input ENUM_MA_METHOD       RSI_MOMO_SMA_Mode       = MODE_SMA;
input int                  MACD_V1                 = 34;
input int                  MACD_V2                 = 55;
input int                  MACD_AVG                = 21;
extern int                 ATR_BackPeriod          = 5000;
extern int                 ATR_SecondaryTF         = 5;
extern int                 ATR_Period              = 11;
extern double              ATR_Factor              = 10;
input string               GeneralSettings         = "===========================================";
input int                  MagicNumber             = 20160113;
input int                  Slippage                = 30;

//+------------------------------------------------------------------+
//|       Global Variables                                           |
//+------------------------------------------------------------------+

double pips;
double EntryPrice = 0;
double ExitPrice  = 0;
double ATR        = 0;

//input string Password = ""; //Please Enter Your Password

//+------------------------------------------------------------------+
//|        Initialization                                            |
//+------------------------------------------------------------------+

int OnInit()
{
  //---Determine what a PIP is.
  pips = Point; //FXCM is a "5 Digit Broker" therefore, POINT == .00001 (NON JPY), .001 (JPY)
  if(Digits==3||Digits==5)
  pips*=10;
  //   #include<InitChecks.mqh>
  Comment("Expert Loaded Successfully");

  //---- initialization done
  return(0); // return "nothing";
}
//+------------------------------------------------------------------+
//|        De-Initialization                                         |
//+------------------------------------------------------------------+

void OnDeinit(const int reason)
{
  Comment(" ");
}

//+------------------------------------------------------------------+
//|         On Start                                                 |
//+------------------------------------------------------------------+

int start()
{
  //---Declare Variables

  return(0);
}
//+------------------------------------------------------------------+
//|         On Tick Function = "Calculate Every Tick"                |
//+------------------------------------------------------------------+

void OnTick() {

  // -------------------- Declaring Local Variables for "OnTick()" --------------------

  double vol;
  double StopLoss = 0;
  //   double TakeProfit = OrderOpenPrice() + (TakeProfitPips * pips);
  double TakeProfit = 0;
  double stopslevel = 0;
  int orders_counter = 0;
  int totalOrders = OrdersTotal();

  static datetime CandleTime = 0;

  // -------------------- Iterate over all Open orders --------------------

  for(int i = totalOrders - 1; i > -1 ; i--) {
    // Filter orders that have a ticket number.
    if(OrderSelect(i, SELECT_BY_POS) && OrderSymbol() == _Symbol && OrderMagicNumber() == MagicNumber && OrderCloseTime() == 0)         //+---Selects any trade that is currently open then initiates a series of MODIFICATIONS to that order per the following group of "if{}" statements
    {
      // Calculate profit target.
      TakeProfit = OrderOpenPrice() + (TakeProfitPips * pips);
      Comment("Take Profit = ", OrderTakeProfit());

      refreshRates();                                                                                                                  //+---This function is used when Expert Advisor has been calculating for a long time and needs data refreshing. Returns true if data are refreshed, otherwise returns false.

      stopslevel = MathMax(SymbolInfoInteger(_Symbol, SYMBOL_SPREAD), SymbolInfoInteger(_Symbol, SYMBOL_TRADE_STOPS_LEVEL)) * pips;  //+---Establishes WHERE the Stop should be plotted on the chart.
      Comment("Stops Level = ", stopslevel);

      // Modifications for BUY orders.
      if(OrderType() == OP_BUY)
      {
        EntryPrice = Ask;
        ExitPrice = Bid;

        if(EndOfWeekCloseOut && CheckEOWC())
        {
          if(OrderClose(OrderTicket(), OrderLots(), ExitPrice, Slippage, clrRed))
          {  Print("BUY OrderClose by EndOfWeekCloseOut is ok"); continue; }
          else
          Print("BUY OrderClose by EndOfWeekCloseOut failed with error: ", ErrorDescription(GetLastError()));
        }

        if(TakeProfitPips > 0 && OrderTakeProfit() == 0)
        {
          if(TakeProfit - EntryPrice > stopslevel)
          {
            if(OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), TakeProfit, 0))
            Print("BUY OrderModify by TakeProfit is ok");
            else
            Print("BUY OrderModify by TakeProfit failed with error: ", ErrorDescription(GetLastError()));
          }
        }

        if(StopLossPips > 0 && OrderStopLoss() == 0)
        {
          StopLoss = OrderOpenPrice() - (StopLossPips * pips);

          if(ExitPrice - StopLoss > stopslevel)
          {
            if(OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0))
            Print("BUY OrderModify by StopLoss is ok");
            else
            Print("BUY OrderModify by StopLoss failed with error: ", ErrorDescription(GetLastError()));
          }
        }

        if(TrailStopATR)
        {
          ATR  = iCustom(NULL, 0, "ATR_TRAILSTOP_V4", ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 0, 0);
          StopLoss = ATR;

          if(StopLoss > 0 &&
            //                 ExitPrice - StopLoss > stopslevel &&
            (StopLoss - OrderStopLoss() > 0
            ||
            OrderStopLoss() == 0)
          )
          {
            if(OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0))
            Print("BUY OrderModify by TrailStopATR is ok");
            else
            Print("BUY OrderModify by TrailStopATR failed with error: ", ErrorDescription(GetLastError()));
          }
        }

        if(TrailStopPreviousClose)
        {
          StopLoss = Close[1] - (TrailStopSize * pips);

          if(ExitPrice - StopLoss > stopslevel && (StopLoss - OrderStopLoss() > 0 || OrderStopLoss() == 0))
          {
            if(OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0))
            Print("BUY OrderModify by TrailStopPreviousClose is ok");
            else
            Print("BUY OrderModify by TrailStopPreviousClose failed with error: ", ErrorDescription(GetLastError()));
          }
        }

        if(TrailStopStandard)
        {
          StopLoss = ExitPrice - (TrailStopSize * pips);

          if(ExitPrice - StopLoss > stopslevel && (StopLoss - OrderStopLoss() > 0 || OrderStopLoss() == 0))
          {
            if(OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0))
            Print("BUY OrderModify by TrailStopStandard is ok");
            else
            Print("BUY OrderModify by TrailStopStandard failed with error: ", ErrorDescription(GetLastError()));
          }
        }

        orders_counter++;
      }

      // Modifications for SELL orders.
      if(OrderType() == OP_SELL)
      {
        EntryPrice = Bid;
        ExitPrice = Ask;
        if(EndOfWeekCloseOut && CheckEOWC())
        {
          if(OrderClose(OrderTicket(), OrderLots(), ExitPrice, Slippage, clrRed))
          {  Print("SELL OrderClose by EndOfWeekCloseOut is ok"); continue; }
          else
          Print("SELL OrderClose by EndOfWeekCloseOut failed with error: ", ErrorDescription(GetLastError()));
        }

        if(TakeProfitPips > 0 && OrderTakeProfit() == 0)
        {

          if(EntryPrice - TakeProfit < stopslevel)
          {
            if(OrderModify(OrderTicket(), OrderOpenPrice(), OrderStopLoss(), TakeProfit, 0))
            Print("SELL OrderModify by TakeProfit is ok");
            else
            Print("SELL OrderModify by TakeProfit failed with error: ", ErrorDescription(GetLastError()));
          }
        }

        if(StopLossPips > 0 && OrderStopLoss() == 0)
        {
          StopLoss = OrderOpenPrice() + (StopLossPips * pips);

          if((StopLoss - ExitPrice) > stopslevel)
          {
            if(OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0))
            Print("SELL OrderModify by StopLoss is ok");
            else
            Print("SELL OrderModify by StopLoss failed with error: ", ErrorDescription(GetLastError()));
          }
        }

        if(TrailStopATR)
        {
          ATR = iCustom(NULL, 0, "ATR_TRAILSTOP_V4", ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 1, 0);
          StopLoss = ATR;

          if(StopLoss > 0 &&
            //                  StopLoss - ExitPrice > stopslevel &&
            (OrderStopLoss() - StopLoss > 0
            ||
            OrderStopLoss() == 0))
            {
              if(OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0))
              Print("SELL OrderModify by TrailStopATR is ok");
              else
              Print("SELL OrderModify by TrailStopATR failed with error: ", ErrorDescription(GetLastError()));
            }
          }

          if(TrailStopPreviousClose)
          {
            StopLoss = Close[1] + (TrailStopSize * pips);

            if(StopLoss - ExitPrice > stopslevel && (OrderStopLoss() - StopLoss > 0 || OrderStopLoss() == 0))
            {
              if(OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0))
              Print("SELL OrderModify by TrailStopPreviousClose is ok");
              else
              Print("SELL OrderModify by TrailStopPreviousClose failed with error: ", ErrorDescription(GetLastError()));
            }
          }

          if(TrailStopStandard)
          {
            StopLoss = ExitPrice + (TrailStopSize * pips);

            if(StopLoss - ExitPrice > stopslevel && (OrderStopLoss() - StopLoss > 0 || OrderStopLoss() == 0))
            {
              if(OrderModify(OrderTicket(), OrderOpenPrice(), StopLoss, TakeProfit, 0))
              Print("SELL OrderModify by TrailStopStandard is ok");
              else
              Print("SELL OrderModify by TrailStopStandard failed with error: ", ErrorDescription(GetLastError()));
            }
          }

          orders_counter++;
        }
      }
    }


    if(CandleTime != Time[0])
    {
      CandleTime = Time[0];
      // -------------------- SEND TRADE Conditions --------------------

      if(orders_counter < MaxOpenTrades && !CheckEOWC())
      {
        if(GetSignal() == OP_BUY)
        {
          RefreshRates();
          EntryPrice = Ask;
          ATR = ATR  = iCustom(NULL, 0, "ATR_TRAILSTOP_V4", ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 0, 0);
          vol = GetVolume();
          Comment("Vol =", GetVolume());
          if(EntryPrice > ATR)
          {
            if(OrderSend(_Symbol, OP_BUY, vol, EntryPrice, Slippage, 0, 0, "", MagicNumber, 0, clrCyan) > 0)
            Print("BUY OrderSend is ok");
            else
            Print("BUY OrderSend failed with error: ", ErrorDescription(GetLastError()));
          }
        }

        if(GetSignal() == OP_SELL)
        {
          EntryPrice = Bid;
          ATR  = iCustom(NULL, 0, "ATR_TRAILSTOP_V4", ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 1, 0);
          vol = GetVolume();
          Comment("Vol =", GetVolume());
          if(EntryPrice < ATR)
          {
            if(OrderSend(_Symbol, OP_SELL, vol, EntryPrice, Slippage, 0, 0, "", MagicNumber, 0, clrPink) > 0)
            Print("SELL OrderSend is ok");
            else
            Print("SELL OrderSend failed with error: ", ErrorDescription(GetLastError()));
          }
        }
      }

    }
  }
  // -------------------- Utility Functions --------------------
  bool CheckEOWC()
  {
    bool result = false;

    datetime eowc_time = StringToTime(TimeToString(TimeCurrent(), TIME_DATE) + " " + EOWCO_TIME);

    if(DayOfWeek() == 5 && TimeCurrent() >= eowc_time) result = true;

    return(result);
  }

  // Calculate volume based on currency pair fn.
  double GetVolume()
  {
    double result = SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN);
    int LotDigits = 2;

    if(QuantityMode == STATIC)
    {
      result *= QuantityStatic;
    }

    if(QuantityMode == DYNAMIC)
    {
      result = (AccountFreeMargin() * (RiskPercent)) / SymbolInfoDouble(_Symbol, SYMBOL_TRADE_CONTRACT_SIZE);
    }

    if(SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN) == 0.01) LotDigits = 2;
    else if(SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN) == 0.1) LotDigits = 1;
    else if(SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN) == 1) LotDigits = 0;

    return(MathMax(NormalizeDouble(result, LotDigits), SymbolInfoDouble(_Symbol, SYMBOL_VOLUME_MIN)));
  }

  // calculate BUY and SELL signals.
  int GetSignal()
  {
    int result = -1;
    //static datetime SigCandleTime = 0;
    //  if(SigCandleTime != Time[0])
    //   {
    double MACD_Value       = iMACD(_Symbol, _Period, MACD_V1, MACD_V2, MACD_AVG, PRICE_CLOSE, MODE_MAIN, 0);
    double MACD_Average     = iMACD(_Symbol, _Period, MACD_V1, MACD_V2, MACD_AVG, PRICE_CLOSE, MODE_SIGNAL, 0);

    double RSI              = iCustom(_Symbol, _Period, "RSI", RSI_PERIOD, 0, 0);
    double RSI_Value        = iCustom(_Symbol, _Period, "RSI_MA_V2", RSI_PERIOD, RSI_V1EMA_Period, MODE_EMA, RSI_V2SMA_Period, MODE_SMA, RSI_SMA_Period , MODE_SMA, RSI_EMA1_Period, MODE_EMA, RSI_EMA2_Period, MODE_EMA, 1, 0);
    double RSI_SMA          = iCustom(_Symbol, _Period, "RSI_MA_V2", RSI_PERIOD, RSI_V1EMA_Period, MODE_EMA, RSI_V2SMA_Period, MODE_SMA, RSI_SMA_Period , MODE_SMA, RSI_EMA1_Period, MODE_EMA, RSI_EMA2_Period, MODE_EMA, 2, 0);
    double RSI_EMA1         = iCustom(_Symbol, _Period, "RSI_MA_V2", RSI_PERIOD, RSI_V1EMA_Period, MODE_EMA, RSI_V2SMA_Period, MODE_SMA, RSI_SMA_Period , MODE_SMA, RSI_EMA1_Period, MODE_EMA, RSI_EMA2_Period, MODE_EMA, 3, 0);
    double RSI_EMA2         = iCustom(_Symbol, _Period, "RSI_MA_V2", RSI_PERIOD, RSI_V1EMA_Period, MODE_EMA, RSI_V2SMA_Period, MODE_SMA, RSI_SMA_Period , MODE_SMA, RSI_EMA1_Period, MODE_EMA, RSI_EMA2_Period, MODE_EMA, 4, 0);

    double RSI_MOMO_Value   = iCustom(_Symbol, _Period, "RSI_MOMO", RSI_PERIOD, RSI_MOMO_V1, MODE_EMA, RSI_MOMO_V2, MODE_EMA, RSI_MOMO_SMA, MODE_SMA, 1, 0);
    double RSI_MOMO_MA      = iCustom(_Symbol, _Period, "RSI_MOMO", RSI_PERIOD, RSI_MOMO_V1, MODE_EMA, RSI_MOMO_V2, MODE_EMA, RSI_MOMO_SMA, MODE_SMA, 2, 0);

    double ema_med          = iMA(_Symbol, _Period, EMA_MED, 0, MODE_EMA, PRICE_CLOSE, 0);
    double ema_med_1        = iMA(_Symbol, _Period, EMA_MED, 0, MODE_EMA, PRICE_CLOSE, 1);
    double ema_med_2        = iMA(_Symbol, _Period, EMA_MED, 0, MODE_EMA, PRICE_CLOSE, 2);
    double ema_med_3        = iMA(_Symbol, _Period, EMA_MED, 0, MODE_EMA, PRICE_CLOSE, 3);

    double ema_fast         = iMA(_Symbol, _Period, EMA_FAST, 0, MODE_EMA, PRICE_CLOSE, 0);

    double ATR_BUY_Bias = iCustom(NULL, 0, "ATR_TRAILSTOP_V4", ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 0, 0);
    double ATR_SELL_Bias = iCustom(NULL, 0, "ATR_TRAILSTOP_V4", ATR_BackPeriod, ATR_SecondaryTF, ATR_Period, ATR_Factor, 1, 0);

    //|-----------Logic Conditions for BUY & SELL---------------------

    if(MACD_Value > MACD_Average &&
      (
        (
          RSI > RSI_Value &&
          RSI > RSI_SMA
        )
        ||
        (
          RSI_EMA1 > RSI_EMA2
          ||
          RSI > RSI_EMA2
        )
      )
      &&
      RSI_MOMO_Value > RSI_MOMO_MA &&


      Close[0] > ema_med &&
      Close[1] > ema_med_1 &&
      Close[2] > ema_med_2 &&

      ema_fast > ema_med &&
      ema_med > ema_med_1 &&
      ema_med_1 > ema_med_2 &&
      ema_med_2 > ema_med_3 &&

      Close[0] > ATR_BUY_Bias
    )
    {
      result = OP_BUY;
    }

    else

    if(MACD_Value < MACD_Average &&
      (
        (
          RSI < RSI_Value &&
          RSI < RSI_SMA
        )
        ||
        (
          RSI_EMA1 < RSI_EMA2
          ||
          RSI < RSI_EMA2
        )
      )
      &&
      RSI_MOMO_Value < RSI_MOMO_MA &&

      Close[0] < ema_med &&
      Close[1] < ema_med_1 &&
      Close[2] < ema_med_2 &&

      ema_fast < ema_med &&
      ema_med < ema_med_1 &&
      ema_med_1 < ema_med_2 &&
      ema_med_2 < ema_med_3 &&

      Close[0] < ATR_SELL_Bias
    )
    {
      result = OP_SELL;
    }

    //   string cmt  =  "macd_value: " + DoubleToString(macd_value, _Digits) + "\n" +
    //                  "macd_average: " + DoubleToString(macd_average, _Digits) + "\n" +
    //                  "rsi: " + DoubleToString(rsi, _Digits) + "\n" +
    //                  "rsi_value: " + DoubleToString(rsi_value, _Digits) + "\n" +
    //                  "rsi_sma: " + DoubleToString(rsi_sma, _Digits) + "\n" +
    //                  "rsi_v1: " + DoubleToString(rsi_v1, _Digits) + "\n" +
    //                  "rsi_v2: " + DoubleToString(rsi_v2, _Digits) + "\n" +
    //                  "rsi_momo_value: " + DoubleToString(rsi_momo_value, _Digits) + "\n" +
    //                  "rsi_momo_sma: " + DoubleToString(rsi_momo_sma, _Digits) + "\n" +
    //                  "ema_med: " + DoubleToString(ema_med, _Digits) + "\n" +
    //                  "ema_med_1: " + DoubleToString(ema_med_1, _Digits) + "\n" +
    //                  "ema_med_2: " + DoubleToString(ema_med_2, _Digits) + "\n" +
    //                  "ema_med_3: " + DoubleToString(ema_med_3, _Digits) + "\n" +
    //                  "ema_fast: " + DoubleToString(ema_fast, _Digits) + "\n" +
    //                  "result: " + IntegerToString(result);
    //
    //   Comment(cmt);
    //SigCandleTime = Time[0];
    //}
    return(result);

  }

  // -------------------- END of BUY & SELL Logic Conditions--------------------


  double GetATRTrailStop(int cmd)
  {
    double result = 0;

    if(cmd == OP_BUY) result = ATR_BUY;
    if(cmd == OP_SELL) result = ATR_SELL;

    return(NormalizeDouble(result, _Digits));
  }

  string ErrorDescription(int err)
  {
    string result = "";
    switch(err)
    {
      case 0:     result="ERR_NO_ERROR"; break;
      case 1:     result="ERR_NO_RESULT"; break;
      case 2:     result="ERR_COMMON_ERROR"; break;
      case 3:     result="ERR_INVALID_TRADE_PARAMETERS"; break;
      case 4:     result="ERR_SERVER_BUSY"; break;
      case 5:     result="ERR_OLD_VERSION"; break;
      case 6:     result="ERR_NO_CONNECTION"; break;
      case 7:     result="ERR_NOT_ENOUGH_RIGHTS"; break;
      case 8:     result="ERR_TOO_FREQUENT_REQUESTS"; break;
      case 9:     result="ERR_MALFUNCTIONAL_TRADE"; break;
      case 64:    result="ERR_ACCOUNT_DISABLED"; break;
      case 65:    result="ERR_INVALID_ACCOUNT"; break;
      case 128:   result="ERR_TRADE_TIMEOUT"; break;
      case 129:   result="ERR_INVALID_PRICE"; break;
      case 130:   result="ERR_INVALID_STOPS"; break;
      case 131:   result="ERR_INVALID_TRADE_VOLUME"; break;
      case 132:   result="ERR_MARKET_CLOSED"; break;
      case 133:   result="ERR_TRADE_DISABLED"; break;
      case 134:   result="ERR_NOT_ENOUGH_MONEY"; break;
      case 135:   result="ERR_PRICE_CHANGED"; break;
      case 136:   result="ERR_OFF_QUOTES"; break;
      case 137:   result="ERR_BROKER_BUSY"; break;
      case 138:   result="ERR_REQUOTE"; break;
      case 139:   result="ERR_ORDER_LOCKED"; break;
      case 140:   result="ERR_LONG_POSITIONS_ONLY_ALLOWED"; break;
      case 141:   result="ERR_TOO_MANY_REQUESTS"; break;
      case 145:   result="ERR_TRADE_MODIFY_DENIED"; break;
      case 146:   result="ERR_TRADE_CONTEXT_BUSY"; break;
      case 147:   result="ERR_TRADE_EXPIRATION_DENIED"; break;
      case 148:   result="ERR_TRADE_TOO_MANY_ORDERS"; break;
      case 149:   result="ERR_TRADE_HEDGE_PROHIBITED"; break;
      case 150:   result="ERR_TRADE_PROHIBITED_BY_FIFO"; break;
      default:    result="ERR_OTHER_ERROR #"+IntegerToString(err); break;
    }
    return(result);
  }
