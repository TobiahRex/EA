const pips;
const takeProfitPips = 100;


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
  let takeProfit = 0;
  let stopsLevel = 0;
  let ordersCounter = 0;
  let candleTime = Date.now;
  let totalOrders = ordersTotal(); // returns array;

  for (let i = totalOrders - 1; i > -1; i--) {

    /*
    Filter numbers that have a ticket number.
    */
    if (orderSelect(i, SELECT_BY_POS) && orderSymbol() === _Symbol &&
    orderMagicNumber() === MagicNumber && orderCloseTime() === 0) {
      // Calculate amount of pips before taking profit.
      takeProfit = orderOpenPrice() + (takeProfitPips * pips);

    }

  }

}
