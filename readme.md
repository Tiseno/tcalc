# tcalc
Command line calculator.

# Dependencies
```
sudo apt-get install libreadline-dev
```

### Features
* Operations `+`, `-`, `*`, `/`, `%`, `^`
* Constants `PI`, `TAU`, `E`, `PHI`, `USD_SEK`, `SEK_USD`, `INF`, `CPW`, `WPC`, `C`.
* Functions from the cmath library `cos`, `sin`, `tan`, `acos`, `asin`, `atan`, `cosh`, `sinh`, `tanh`, `acosh`, `asinh`, `atanh`, `sqrt`, `cbrt`, `exp_e`, `exp_two`, `log_e`, `log_two`, `log_ten`, `tgamma`, `fact`, `lgamma`, `abs`, `ceil`, `floor`, `trunc`, `round`.
* Referencing previous result as value or as expression using `!I`, `#I` where I is the line number and `!!`, `##` the last executed line.
* History using readline

### TODO
- [ ] functions with multiple arguments
- [ ] pretty print correctly with precedences
- [ ] declaring custom functions and symbols
- [ ] caching
- [ ] fix float imprecision
- [ ] persisted history
- [ ] relative referencing of previous lines, i.e. where `##0` means previous line and `##1` the line before

### Bugs
- [ ] !!2 does not parse correctly
- [ ] "cos2(cos2)" is not handled correctly, results in "cos 2 cos 2"

