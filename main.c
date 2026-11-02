/* ====================================================================
   PROJECT : Electricity Bill Calculator Using C Programming
   TYPE    : Menu-driven console application
   FEATURES: Structures, arrays, pointers, strings, file handling,
             switch-case, loops, functions and input validation
   FILE    : All records are saved permanently in "bills.txt"
   ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- Constants (easy to change in one place) ---------- */
#define FILE_NAME      "bills.txt"
#define MAX_CUSTOMERS  200
#define NAME_SIZE      50
#define METER_SIZE     20
#define MONTH_SIZE     20

#define RATE_SLAB1     3.0f     /* 0   - 100 units : Rs. 3 per unit */
#define RATE_SLAB2     5.0f     /* 101 - 200 units : Rs. 5 per unit */
#define RATE_SLAB3     7.0f     /* 201 - 300 units : Rs. 7 per unit */
#define RATE_SLAB4     8.0f     /* above 300 units : Rs. 8 per unit */
#define FIXED_CHARGE   50.0f    /* fixed charge added to every bill */

/* ---------- Structure to store one customer's bill record ---------- */
struct Customer
{
    int   customerId;
    char  name[NAME_SIZE];
    char  meterNumber[METER_SIZE];
    char  month[MONTH_SIZE];
    int   previousReading;
    int   currentReading;
    int   units;
    float energyCharge;
    float fixedCharge;
    float totalBill;
};

/* ---------- Function declarations (prototypes) ---------- */
int   readLine(char *buffer, int size);
void  readString(const char *prompt, char *dest, int size);
void  readInt(const char *prompt, int *value);
int   askYesNo(const char *prompt);
void  printLine(char ch, int count);

float calculateEnergyCharge(int units);
void  calculateBill(struct Customer *c);

int   loadCustomers(struct Customer list[]);
int   saveCustomer(const struct Customer *c);
int   saveAllCustomers(const struct Customer list[], int count);
int   findById(const struct Customer list[], int count, int id);

void  showMenu(void);
void  printBill(const struct Customer *c);
void  addCustomer(void);
void  viewAllBills(void);
void  searchCustomer(void);
void  deleteCustomer(void);
void  billSummary(void);

/* ====================================================================
   main() : shows the menu again and again until the user selects Exit
   ==================================================================== */
int main(void)
{
    int choice;

    do
    {
        showMenu();
        readInt("Enter your choice (1-6): ", &choice);

        switch (choice)
        {
            case 1: addCustomer();    break;
            case 2: viewAllBills();   break;
            case 3: searchCustomer(); break;
            case 4: deleteCustomer(); break;
            case 5: billSummary();    break;
            case 6:
                printf("\nAll records are saved in %s.\n", FILE_NAME);
                printf("Thank you for using the Electricity Bill Calculator. Goodbye!\n");
                break;
            default:
                printf("\nInvalid choice! Please enter a number from 1 to 6.\n");
        }
    } while (choice != 6);

    return 0;
}

/* ====================================================================
   INPUT HELPER FUNCTIONS
   ==================================================================== */

/* Reads one line from the keyboard into buffer.
   Returns 1 if the line was read properly, 0 if it was too long. */
int readLine(char *buffer, int size)
{
    int ch;

    if (fgets(buffer, size, stdin) == NULL)      /* keyboard input closed */
    {
        printf("\nInput ended. Exiting program.\n");
        exit(0);
    }

    if (strchr(buffer, '\n') == NULL)            /* line was longer than size */
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;                                    /* throw away the extra part */
        return 0;
    }

    buffer[strcspn(buffer, "\n")] = '\0';        /* remove the newline */
    return 1;
}

/* Asks for a text value. Keeps asking until it is not empty. */
void readString(const char *prompt, char *dest, int size)
{
    int i, blank;

    while (1)
    {
        printf("%s", prompt);

        if (!readLine(dest, size))
        {
            printf("Input too long! Maximum %d characters allowed.\n", size - 1);
            continue;
        }

        blank = 1;                               /* check if only spaces */
        for (i = 0; dest[i] != '\0'; i++)
        {
            if (!isspace((unsigned char)dest[i]))
                blank = 0;
            if (dest[i] == '|')                  /* '|' is used in the file */
                dest[i] = ' ';
        }

        if (blank)
        {
            printf("Input cannot be empty. Please try again.\n");
            continue;
        }
        return;
    }
}

/* Asks for a whole number. Keeps asking until the input is valid. */
void readInt(const char *prompt, int *value)
{
    char buffer[50];
    char extra;
    int  number;

    while (1)
    {
        printf("%s", prompt);

        if (!readLine(buffer, sizeof(buffer)) || strlen(buffer) > 9)
        {
            printf("Invalid input! Please enter a smaller whole number.\n");
            continue;
        }

        /* sscanf returns 1 only if exactly one number and nothing else */
        if (sscanf(buffer, "%d %c", &number, &extra) == 1)
        {
            *value = number;
            return;
        }
        printf("Invalid input! Please enter a whole number (digits only).\n");
    }
}

/* Asks a yes/no question. Returns 1 for yes and 0 for no. */
int askYesNo(const char *prompt)
{
    char answer[20];

    while (1)
    {
        printf("%s", prompt);

        if (readLine(answer, sizeof(answer)) && strlen(answer) == 1)
        {
            if (tolower((unsigned char)answer[0]) == 'y') return 1;
            if (tolower((unsigned char)answer[0]) == 'n') return 0;
        }
        printf("Please type only y or n.\n");
    }
}

/* Prints a line made of the same character (used for decoration) */
void printLine(char ch, int count)
{
    int i;
    for (i = 0; i < count; i++)
        printf("%c", ch);
    printf("\n");
}

/* ====================================================================
   BILL CALCULATION
   ==================================================================== */

/* Progressive slab system: each slab is charged at its own rate */
float calculateEnergyCharge(int units)
{
    float charge;

    if (units <= 100)
        charge = units * RATE_SLAB1;
    else if (units <= 200)
        charge = 100 * RATE_SLAB1 + (units - 100) * RATE_SLAB2;
    else if (units <= 300)
        charge = 100 * RATE_SLAB1 + 100 * RATE_SLAB2 + (units - 200) * RATE_SLAB3;
    else
        charge = 100 * RATE_SLAB1 + 100 * RATE_SLAB2 + 100 * RATE_SLAB3
                 + (units - 300) * RATE_SLAB4;

    return charge;
}

/* Fills units, energyCharge, fixedCharge and totalBill using a pointer */
void calculateBill(struct Customer *c)
{
    c->units        = c->currentReading - c->previousReading;
    c->energyCharge = calculateEnergyCharge(c->units);
    c->fixedCharge  = FIXED_CHARGE;
    c->totalBill    = c->energyCharge + c->fixedCharge;
}

/* ====================================================================
   FILE HANDLING FUNCTIONS
   Each record is one line:
   id|name|meter|previous|current|units|month|energy|fixed|total
   ==================================================================== */

/* Reads all records from bills.txt into the array. Returns the count. */
int loadCustomers(struct Customer list[])
{
    FILE *fp;
    int count = 0;

    fp = fopen(FILE_NAME, "r");
    if (fp == NULL)                 /* file does not exist yet */
        return 0;

    while (count < MAX_CUSTOMERS &&
           fscanf(fp, "%d|%49[^|]|%19[^|]|%d|%d|%d|%19[^|]|%f|%f|%f",
                  &list[count].customerId,
                  list[count].name,
                  list[count].meterNumber,
                  &list[count].previousReading,
                  &list[count].currentReading,
                  &list[count].units,
                  list[count].month,
                  &list[count].energyCharge,
                  &list[count].fixedCharge,
                  &list[count].totalBill) == 10)
    {
        count++;
    }

    fclose(fp);
    return count;
}

/* Adds one record at the end of the file. Returns 1 if successful. */
int saveCustomer(const struct Customer *c)
{
    FILE *fp = fopen(FILE_NAME, "a");

    if (fp == NULL)
        return 0;

    fprintf(fp, "%d|%s|%s|%d|%d|%d|%s|%.2f|%.2f|%.2f\n",
            c->customerId, c->name, c->meterNumber,
            c->previousReading, c->currentReading, c->units,
            c->month, c->energyCharge, c->fixedCharge, c->totalBill);

    fclose(fp);
    return 1;
}

/* Rewrites the whole file from the array (used after deleting). */
int saveAllCustomers(const struct Customer list[], int count)
{
    FILE *fp;
    int i;

    fp = fopen(FILE_NAME, "w");
    if (fp == NULL)
        return 0;

    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%d|%s|%s|%d|%d|%d|%s|%.2f|%.2f|%.2f\n",
                list[i].customerId, list[i].name, list[i].meterNumber,
                list[i].previousReading, list[i].currentReading, list[i].units,
                list[i].month, list[i].energyCharge, list[i].fixedCharge,
                list[i].totalBill);
    }

    fclose(fp);
    return 1;
}

/* Linear search by Customer ID. Returns the index, or -1 if not found. */
int findById(const struct Customer list[], int count, int id)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (list[i].customerId == id)
            return i;
    }
    return -1;
}

/* ====================================================================
   DISPLAY FUNCTIONS
   ==================================================================== */

void showMenu(void)
{
    printf("\n");
    printLine('=', 44);
    printf("      ELECTRICITY BILL CALCULATOR\n");
    printLine('=', 44);
    printf("  1. Add Customer and Generate Bill\n");
    printf("  2. View All Bills\n");
    printf("  3. Search Customer\n");
    printf("  4. Delete Customer Record\n");
    printf("  5. Bill Summary\n");
    printf("  6. Exit\n");
    printLine('=', 44);
}

/* Prints one customer's bill as a receipt */
void printBill(const struct Customer *c)
{
    printf("\n");
    printLine('=', 44);
    printf("         ELECTRICITY BILL RECEIPT\n");
    printLine('=', 44);
    printf("Customer ID        : %d\n", c->customerId);
    printf("Customer Name      : %s\n", c->name);
    printf("Meter Number       : %s\n", c->meterNumber);
    printf("Billing Month      : %s\n", c->month);
    printLine('-', 44);
    printf("Previous Reading   : %d\n", c->previousReading);
    printf("Current Reading    : %d\n", c->currentReading);
    printf("Units Consumed     : %d\n", c->units);
    printLine('-', 44);
    printf("Electricity Charge : Rs. %.2f\n", c->energyCharge);
    printf("Fixed Charge       : Rs. %.2f\n", c->fixedCharge);
    printLine('-', 44);
    printf("TOTAL BILL         : Rs. %.2f\n", c->totalBill);
    printLine('=', 44);
}

/* ====================================================================
   MENU OPTION FUNCTIONS
   ==================================================================== */

/* Option 1 : take details, calculate the bill, save it and show it */
void addCustomer(void)
{
    struct Customer list[MAX_CUSTOMERS];
    struct Customer c;
    int count;

    count = loadCustomers(list);

    if (count >= MAX_CUSTOMERS)
    {
        printf("\nStorage is full! Cannot add more customers.\n");
        return;
    }

    printf("\n--- Add Customer and Generate Bill ---\n");

    /* Customer ID : must be positive and not already used */
    while (1)
    {
        readInt("Enter Customer ID (positive number): ", &c.customerId);

        if (c.customerId <= 0)
            printf("Customer ID must be greater than 0.\n");
        else if (findById(list, count, c.customerId) != -1)
            printf("This Customer ID already exists! Enter a different ID.\n");
        else
            break;
    }

    readString("Enter Customer Name: ", c.name, NAME_SIZE);
    readString("Enter Meter Number: ", c.meterNumber, METER_SIZE);
    readString("Enter Billing Month (e.g. March 2026): ", c.month, MONTH_SIZE);

    /* Previous reading : must not be negative */
    while (1)
    {
        readInt("Enter Previous Meter Reading: ", &c.previousReading);
        if (c.previousReading < 0)
            printf("Meter reading cannot be negative.\n");
        else
            break;
    }

    /* Current reading : must not be less than the previous reading */
    while (1)
    {
        readInt("Enter Current Meter Reading : ", &c.currentReading);
        if (c.currentReading < 0)
            printf("Meter reading cannot be negative.\n");
        else if (c.currentReading < c.previousReading)
            printf("Current reading cannot be less than previous reading (%d).\n",
                   c.previousReading);
        else
            break;
    }

    calculateBill(&c);

    if (saveCustomer(&c))
    {
        printBill(&c);
        printf("Record saved successfully.\n");
    }
    else
    {
        printf("\nError: Could not open %s for writing.\n", FILE_NAME);
    }
}

/* Option 2 : show every saved record in a table */
void viewAllBills(void)
{
    struct Customer list[MAX_CUSTOMERS];
    int count, i;

    count = loadCustomers(list);

    printf("\n--- All Customer Bills ---\n");

    if (count == 0)
    {
        printf("No records found.\n");
        return;
    }

    printLine('-', 78);
    printf("%-6s %-20s %-14s %-14s %6s %11s\n",
           "ID", "Name", "Meter No", "Month", "Units", "Total (Rs)");
    printLine('-', 78);

    for (i = 0; i < count; i++)
    {
        printf("%-6d %-20.20s %-14.14s %-14.14s %6d %11.2f\n",
               list[i].customerId, list[i].name, list[i].meterNumber,
               list[i].month, list[i].units, list[i].totalBill);
    }

    printLine('-', 78);
    printf("Total records: %d\n", count);
}

/* Option 3 : search by Customer ID or Meter Number */
void searchCustomer(void)
{
    struct Customer list[MAX_CUSTOMERS];
    char meter[METER_SIZE];
    int count, option, id, index, i, found;

    count = loadCustomers(list);

    printf("\n--- Search Customer ---\n");

    if (count == 0)
    {
        printf("No records found.\n");
        return;
    }

    printf("1. Search by Customer ID\n");
    printf("2. Search by Meter Number\n");
    readInt("Enter your option (1-2): ", &option);

    switch (option)
    {
        case 1:
            readInt("Enter Customer ID: ", &id);
            index = findById(list, count, id);
            if (index == -1)
                printf("\nCustomer record not found.\n");
            else
                printBill(&list[index]);
            break;

        case 2:
            readString("Enter Meter Number: ", meter, METER_SIZE);
            found = 0;
            for (i = 0; i < count; i++)
            {
                if (strcmp(list[i].meterNumber, meter) == 0)
                {
                    printBill(&list[i]);
                    found = 1;
                }
            }
            if (!found)
                printf("\nCustomer record not found.\n");
            break;

        default:
            printf("\nInvalid option!\n");
    }
}

/* Option 4 : delete a record by Customer ID (after confirmation) */
void deleteCustomer(void)
{
    struct Customer list[MAX_CUSTOMERS];
    int count, id, index, i;

    count = loadCustomers(list);

    printf("\n--- Delete Customer Record ---\n");

    if (count == 0)
    {
        printf("No records found.\n");
        return;
    }

    readInt("Enter Customer ID to delete: ", &id);
    index = findById(list, count, id);

    if (index == -1)
    {
        printf("\nCustomer record not found.\n");
        return;
    }

    printf("\nRecord found: ID %d, Name: %s, Meter: %s, Month: %s\n",
           list[index].customerId, list[index].name,
           list[index].meterNumber, list[index].month);

    if (!askYesNo("Are you sure you want to delete this record? (y/n): "))
    {
        printf("Delete cancelled. Record is safe.\n");
        return;
    }

    /* shift all later records one place to the left */
    for (i = index; i < count - 1; i++)
        list[i] = list[i + 1];
    count--;

    if (saveAllCustomers(list, count))
        printf("Record deleted successfully.\n");
    else
        printf("Error: Could not update %s.\n", FILE_NAME);
}

/* Option 5 : total customers, total units and total amount */
void billSummary(void)
{
    struct Customer list[MAX_CUSTOMERS];
    int   count, i;
    long  totalUnits  = 0;
    float totalAmount = 0;

    count = loadCustomers(list);

    for (i = 0; i < count; i++)
    {
        totalUnits  += list[i].units;
        totalAmount += list[i].totalBill;
    }

    printf("\n");
    printLine('=', 44);
    printf("               BILL SUMMARY\n");
    printLine('=', 44);
    printf("Total Customers        : %d\n", count);
    printf("Total Units Consumed   : %ld\n", totalUnits);
    printf("Total Bill Amount      : Rs. %.2f\n", totalAmount);
    printLine('=', 44);
}
