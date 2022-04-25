# calc
Command line calculator.

### Features
* Defined constants `PI`, `TAU`, `E`, `PHI`, `USD_SEK`, `SEK_USD`, `INF`, `CPW`, `WPC`, `C`.
* Built in functions from the cmath library `cos`, `sin`, `tan`, `acos`, `asin`, `atan`, `cosh`, `sinh`, `tanh`, `acosh`, `asinh`, `atanh`, `sqrt`, `cbrt`, `exp`, `exp2`, `log`, `log10`, `tgamma`, `lgamma`, `abs`, `ceil`, `floor`, `trunc`, `round`.
* Referencing previous result as value or as expression using `!I`, `#I` where I is the line number and `!!`, `##` the last executed line.

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
- [ ] functions with more than one argument
- [ ] pretty print with precedences
- [ ] ability to define functions and symbols
- [ ] automatically cache results from executions and functions
- [ ] use something more precis than floats
- [ ] custom operators
- [ ] complex number support
- [ ] persisted history
- [ ] pull fresh currency constants from the web
- [ ] all conversion constants on the form XXX_XXX

### Bugs
- [x] "(" is parsed as valid character for symbol; cos(5x) parses as "cos(" "5" "x)"
- [ ] !!2 does not parse correctly
- [ ] -1 does not parse as -1
- [ ] "cos2(cos2)" pretty prints as "cos 2 cos 2"

