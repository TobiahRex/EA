# Plan
1. Create a backlog of testing data. 10 years of `PSAR` settings at `.0009 * .2` for 1 hr.
	- 30m & 1hr chart
		- `GBPNZD`
		- `EURNZD`
		- `GBPJPy`
		- 10 years of `PSAR` settings at `.009 * .2` for 1 min
			- 30 min & 1 hr chart

2. The idea is to calculate current market vector acceleration (VA). **Not** just speed.
	- Current VA should be taken from taking the limit of change between 1 hr all the way down to 1 min.
	- The speed will then be cross referenced back to the database of price tests we've conducted.
	- We'll find the most optimal settings for our indicators based on those 10 years of data.
	- Then adjust the settings accordingly.

3. The Stoploss
	- Take a profit amount for each trade. Something small. Think semi-high frequency.
	- Evaluate best stop loss on historical price action speeds.
	  
## Example

If `PSAR` is .009 at .2 and the market speed is `+15 pips/1m` then the highest probability of success is having a 10 pip stop loss & profit target should be 13 pips.
