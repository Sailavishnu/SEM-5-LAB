# LEX Programs Summary

## Program 1: Roll Number Extractor (`roll_number.l`)

### Purpose
Extract and validate roll numbers from a student database file and count total students.

### Roll Number Format
- **yybbnnn** pattern
- yy = 2-digit year of joining
- bb = 3-letter branch code (BCS, BIT, BAI, BEE, BEC, BME, BBT)
- nnn = 3-digit numeric value

### Input File
- **Filename**: `student.txt`
- Contains student information with embedded roll numbers

### Output Files
- **rollnumbers.txt** - Extracted valid roll numbers
- **Console output** - Total student count

### Compilation & Run
```bash
flex roll_number.l
gcc -o roll_number lex.yy.c -lfl
./roll_number
```

### Test Output
```
Roll Numbers found:
Total number of students = 8

Output file contains:
23BCS101
23BIT205
24BAI050
24BEE150
22BEC300
23BCS123
23BME075
23BBT160
```

---

## Program 2: HTML Link Extractor (`html_links.l`)

### Purpose
Extract hyperlink text and corresponding URLs from HTML files.

### How It Works
1. Searches for `<a>` tags
2. Extracts `href` attribute values
3. Captures link text between tags
4. Outputs both link text and URL

### Input File
- **Filename**: `webpage.html`
- Standard HTML format with `<a href="...">text</a>` tags

### Output Files
- **links.txt** - Formatted link text and URLs

### Compilation & Run
```bash
flex html_links.l
gcc -o html_links lex.yy.c -lfl
./html_links
```

### Test Output
```
Link Text: Search Google
URL: https://www.google.com

Link Text: College Portal
URL: https://college.edu

Link Text: Placement Details
URL: /placements/info.html

Link Text: GitHub Repository
URL: https://github.com/user/repo

Link Text: Wikipedia
URL: https://www.wikipedia.org
```

---

## Program 3: Text Modifier (`text_modify.l`)

### Purpose
Process text by:
1. Counting occurrences of "is"
2. Replacing "is" with "was"
3. Incrementing all numbers by 10
4. Preserving all other content

### Input File
- **Filename**: `input_text.txt`
- Plain text file

### Output Files
- **output_text.txt** - Modified text
- **Console output** - Count of replacements

### Compilation & Run
```bash
flex text_modify.l
gcc -o text_modify lex.yy.c -lfl
./text_modify
```

### Test Output
```
Text processing completed.
Number of 'is' replaced = 10

Output file contains:
The sky was blue.
Thwas was a test file.         (Note: "This" becomes "Thwas")
I have 15 apples and 13 oranges.    (5+10=15, 3+10=13)
He was a student.
The answer was 52.              (42+10=52)
We have 20 books and 30 pens.   (10+10=20, 20+10=30)
It was raining today.
Thwas was simple.
There was no problem.
Total count was 110.            (100+10=110)
```

---

## Key Features

✓ **Simple & Direct** - No complex logic, straightforward pattern matching
✓ **File-based I/O** - Each program reads from input file and writes to output file
✓ **Easy to Modify** - Clear pattern definitions, easy to customize
✓ **Batch Processing** - Process entire files, not interactive input
✓ **Error Handling** - File open/close error checking

---

## Files Generated
- roll_number.l → roll_number (executable)
- html_links.l → html_links (executable)
- text_modify.l → text_modify (executable)
- student.txt (sample input)
- webpage.html (sample input)
- input_text.txt (sample input)
- rollnumbers.txt (output)
- links.txt (output)
- output_text.txt (output)

---

## Quick Reference

| Program | Input File | Output File | Pattern |
|---------|-----------|-------------|---------|
| Roll Number | student.txt | rollnumbers.txt | [0-9]{2}[A-Z]{3}[0-9]{3} |
| HTML Links | webpage.html | links.txt | `<a href="...">text</a>` |
| Text Modify | input_text.txt | output_text.txt | "is" → "was", digits +10 |

