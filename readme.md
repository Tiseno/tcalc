# tcalc
Command line calculator.

# Dependencies
```
sudo apt-get install libreadline-dev
```

### Features
* Defined constants `PI`, `TAU`, `E`, `PHI`, `USD_SEK`, `SEK_USD`, `INF`, `CPW`, `WPC`, `C`.
* Built in functions from the cmath library `cos`, `sin`, `tan`, `acos`, `asin`, `atan`, `cosh`, `sinh`, `tanh`, `acosh`, `asinh`, `atanh`, `sqrt`, `cbrt`, `exp`, `exp2`, `log`, `log10`, `tgamma`, `lgamma`, `abs`, `ceil`, `floor`, `trunc`, `round`.
* Referencing previous result as value or as expression using `!I`, `#I` where I is the line number and `!!`, `##` the last executed line.
* History using readline

### TODO
- [x] symbols mapping to built in constants
- [x] reference to previous lines
- [x] execution history with up/down
- [x] UTF8 support
- [x] colors
- [x] CLI arguments as input an evaluate it
- [x] reference to previous lines (expression)
- [x] reference to the line before
- [x] evaluate constants in eval
- [x] symbols mapping to built in functions
- [ ] functions with multiple arguments
- [ ] pretty print correctly with precedences
- [ ] declaring custom functions and symbols
- [ ] caching
- [ ] fix float imprecision
- [ ] persisted history

### Bugs
- [ ] !!2 does not parse correctly
- [ ] -1 does not parse as -1
- [ ] "cos2(cos2)" is not handled correctly, results in "cos 2 cos 2"

