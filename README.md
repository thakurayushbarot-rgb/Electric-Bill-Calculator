# Electricity Bill Calculator Using C Programming

A beginner-friendly, menu-driven console application written in standard C.
It calculates electricity bills using a progressive slab system and stores all
records permanently in a text file (`bills.txt`).

## Features
1. Add customer and generate bill (units are calculated automatically)
2. View all bills in a table
3. Search a customer by Customer ID or Meter Number
4. Delete a customer record (with confirmation)
5. Bill summary (total customers, total units, total amount)
6. Exit

## Tariff (progressive slabs)
| Units            | Rate per unit |
|------------------|---------------|
| First 100 units  | Rs. 3         |
| Next 100 units   | Rs. 5         |
| Next 100 units   | Rs. 7         |
| Above 300 units  | Rs. 8         |
| Fixed charge     | Rs. 50 per bill |

Example: 250 units = (100 x 3) + (100 x 5) + (50 x 7) + 50 = **Rs. 1200**

## C concepts used
Variables and data types, if-else, switch-case, loops (do-while, while, for),
functions, arrays, structures, pointers, strings, file handling
(`fopen`, `fclose`, `fprintf`, `fscanf`) and input validation.

## How to compile and run
```
gcc main.c -o bill
./bill          # Linux / macOS
bill.exe        # Windows
```
Run the program from inside the project folder so that `bills.txt` is created there.

## Input validation
- Customer ID must be a positive number and must not already exist
- Meter readings cannot be negative
- Current reading cannot be less than the previous reading
- Letters or invalid menu choices are rejected and asked again

## Files
| File       | Purpose                                       |
|------------|-----------------------------------------------|
| main.c     | Complete source code                          |
| bills.txt  | Created automatically; stores all records     |
| README.md  | This file                                     |

## Record format in bills.txt
```
id|name|meter|previous|current|units|month|energy|fixed|total
101|Rahul Sharma|MT-4501|1000|1250|250|March 2026|1150.00|50.00|1200.00
```

## Limitations
Console only, maximum 200 records, one bill per Customer ID, no password login.
