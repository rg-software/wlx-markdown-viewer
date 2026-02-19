# Test: Literal Underscore and Asterisk

Mid-word `_` and `*` are always treated as literal characters (no flag needed).

## Renders as literal (mid-word, alphanumeric on both sides)

- file_name stays as file_name
- my_variable_name stays as my_variable_name
- test_function_2 stays as test_function_2
- 2*3 stays as 2*3
- a*b*c stays as a*b*c

## Emphasis still works when intended

- *this is italic*
- **this is bold**
- _this is also italic_
- __this is also bold__
- This *word* is emphasized
- This _word_ is also emphasized

## Edge cases (should still trigger emphasis)

- _leading underscore becomes italic_
- *leading asterisk becomes italic*
- Multiple * asterisks * spaced * out
- Multiple _ underscores _ spaced _ out
