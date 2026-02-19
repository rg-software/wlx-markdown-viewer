# MarkdownView Feature Test

This file tests all active features. Each section shows what you should see.

---

## 1. Headings

# Heading 1
## Heading 2
### Heading 3
#### Heading 4
##### Heading 5
###### Heading 6

---

## 2. Emphasis

*italic* and _also italic_

**bold** and __also bold__

***bold italic*** and ___also bold italic___

---

## 3. Literal Underscore / Asterisk (NEW — default on)

These should render WITHOUT any italic/bold formatting:

- file_name
- my_variable_name
- CONFIG_MAX_SIZE
- get_user_data()
- 2*3 = 6
- a*b + c*d
- path\to\some_file.txt
- HTTP_STATUS_OK = 200

To disable: add `--no-literal-underscore-asterisk` to HoedownArgs.

---

## 4. Strikethrough (--strikethrough)

~~this text is crossed out~~

---

## 5. Highlight (--highlight)

==this text is highlighted==

---

## 6. Superscript (--superscript)

E = mc^2

x^n + y^n = z^n

---

## 7. Autolink (--autolink)

https://github.com

http://example.com/some/path?query=value

user@example.com

---

## 8. Hard Wrap (--hard-wrap)

This line ends here.
This should be on a new line (hard wrap active).
And this too.

---

## 9. Tables (--all-block includes tables)

| Name            | Type    | Default | Description              |
|-----------------|---------|---------|--------------------------|
| file_name       | string  | none    | Path to the config file  |
| max_retry_count | integer | 3       | Maximum retry attempts   |
| enable_logging  | boolean | true    | Enable debug logging     |
| api_base_url    | string  | none    | Base URL for API calls   |

---

## 10. Fenced Code Blocks (--all-block)

```c
/* C code example */
int get_user_data(int user_id) {
    if (user_id < 0) {
        return ERROR_INVALID_ID;
    }
    return fetch_from_db(user_id);
}
```

```python
# Python example
def calculate_area(width, height):
    return width * height

MAX_SIZE = 1024
result = calculate_area(MAX_SIZE, MAX_SIZE)
```

```json
{
    "api_base_url": "https://example.com",
    "max_retry_count": 3,
    "enable_logging": true
}
```

---

## 11. Inline Code

Use `get_user_data()` to fetch user records.

Set `MAX_BUFFER_SIZE = 4096` in your config.

The variable `my_flag` controls this behavior.

---

## 12. Footnotes (--all-block)

This is a statement with a footnote.[^1]

Here is another claim that needs a source.[^note]

[^1]: This is the first footnote.
[^note]: This is a named footnote with more detail.

---

## 13. Blockquote

> This is a blockquote.
> It can span multiple lines.
>
> And multiple paragraphs.

---

## 14. Lists

### Unordered

- Item one
- Item two
  - Nested item
  - Another nested item
- Item three

### Ordered

1. First step
2. Second step
   1. Sub-step A
   2. Sub-step B
3. Third step

---

## 15. Links and Images

[GitHub](https://github.com)

[Link with title](https://example.com "Example Site")

---

## 16. Horizontal Rule

---

***

___

---

## 17. Smartypants (UseSmartypants=1)

Straight quotes become "curly quotes" and 'single quotes'.

Dashes: -- becomes en-dash, --- becomes em-dash.

Ellipsis: ... becomes …

---

## 18. Mixed: Literal Underscores in Tables

| Function          | Returns   | Notes                    |
|-------------------|-----------|--------------------------|
| get_user_id()     | int       | Returns -1 on error      |
| parse_config()    | bool      | Reads from config_file   |
| MAX_BUFFER_SIZE   | constant  | Default value is 4096    |

---

## 19. Mixed: Code and Emphasis Together

The function `calculate_area(width, height)` returns **width × height**.

Use `CONFIG_MAX_SIZE` to set the **maximum** allowed size.

This *word* is italic but this_word_here is literal.

---

## 20. Inline HTML

<kbd>Ctrl</kbd> + <kbd>C</kbd> to copy.

<mark>This text uses a mark tag.</mark>

---

*End of feature test.*
