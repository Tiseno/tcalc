operators = ['-','+','*','/','^']
rn = (n) => Math.floor(Math.random() * n)
ro = () => operators[rn(operators.length)]
rpn = () => 1 + rn(9)
re = () => rpn() + Array(10).fill(0).map(() => ro() + rpn()).join("")
Array(10).fill(0).forEach(() => console.log(re()))
