#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#define MAX_ACCOUNTS 100
#define FILENAME_ACCOUNTS "accounts.txt"
#define FILENAME_TRANSACTIONS "transactions.txt"

typedef struct {
    int accountNumber;
    char name[50];
    int pin;
    float balance;
} Account;

// Function prototypes
void initializeFiles();
int loadAccounts(Account accounts[]);
void saveAccounts(Account accounts[], int count);
int login(Account accounts[], int count, Account *currentAccount);
void deposit(Account *acc, Account accounts[], int count);
void withdraw(Account *acc, Account accounts[], int count);
void checkBalance(Account acc);
void viewTransactionHistory(int accountNumber);
void saveTransaction(int accNo, const char *type, float amount);
void displayMainMenu();
void displayUserMenu();
void clearInputBuffer();
void clearScreen();
void pauseScreen();
int getMaskedPIN();

int main() {
    Account accounts[MAX_ACCOUNTS];
    Account currentAccount;
    int accountCount = 0;
    int choice;
    int loggedIn = 0;

    // Initialize files if they don't exist
    initializeFiles();

    // Load existing accounts
    accountCount = loadAccounts(accounts);

    clearScreen();
    printf("===================================\n");
    printf(" Welcome to ATM Simulation System\n");
    printf("            version 2.1.1"          "\n");
    printf("===================================\n");
    pauseScreen();

    while (1) {
        clearScreen();
        if (!loggedIn) {
            displayMainMenu();
            printf("Enter your choice: ");
            scanf("%d", &choice);
            clearInputBuffer();

            switch (choice) {
                case 1:
                    clearScreen();
                    if (login(accounts, accountCount, &currentAccount)) {
                        loggedIn = 1;
                        printf("\n Login successful! Welcome, %s!\n", currentAccount.name);
                        pauseScreen();
                    } else {
                        printf("\n Login failed! Invalid account number or PIN.\n");
                        pauseScreen();
                    }
                    break;
                case 2:
                    clearScreen();
                    printf("\nThank you for using our ATM. Goodbye! \n");
                    exit(0);
                default:
                    printf("\n Invalid choice! Please try again.\n");
                    pauseScreen();
            }
        } else {
            displayUserMenu();
            printf("Enter your choice: ");
            scanf("%d", &choice);
            clearInputBuffer();

            switch (choice) {
                case 1:
                    clearScreen();
                    deposit(&currentAccount, accounts, accountCount);
                    pauseScreen();
                    break;
                case 2:
                    clearScreen();
                    withdraw(&currentAccount, accounts, accountCount);
                    pauseScreen();
                    break;
                case 3:
                    clearScreen();
                    checkBalance(currentAccount);
                    pauseScreen();
                    break;
                case 4:
                    clearScreen();
                    viewTransactionHistory(currentAccount.accountNumber);
                    pauseScreen();
                    break;
                case 5:
                    clearScreen();
                    loggedIn = 0;
                    printf("\n Logged out successfully!\n");
                    pauseScreen();
                    break;
                default:
                    printf("\n Invalid choice! Please try again.\n");
                    pauseScreen();
            }
        }
    }

    return 0;
}

int getMaskedPIN() {
    char pinStr[20];
    int pinLength = 0;
    int ch;

#ifdef _WIN32
    // Windows implementation using conio.h
    while (1) {
        ch = _getch();

        if (ch == 13) { // Enter key
            pinStr[pinLength] = '\0';
            break;
        } else if (ch == 8 && pinLength > 0) { // Backspace
            pinLength--;
            printf("\b \b");
        } else if (ch >= '0' && ch <= '9' && pinLength < 10) {
            pinStr[pinLength++] = ch;
            printf("*");
        }
    }
#else
    // Unix/Linux implementation using termios
    struct termios oldSettings, newSettings;

    // Disable echoing
    tcgetattr(STDIN_FILENO, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

    while (1) {
        ch = getchar();

        if (ch == '\n' || ch == '\r') { // Enter key
            pinStr[pinLength] = '\0';
            break;
        } else if (ch == 127 && pinLength > 0) { // Backspace
            pinLength--;
            printf("\b \b");
            fflush(stdout);
        } else if (ch >= '0' && ch <= '9' && pinLength < 10) {
            pinStr[pinLength++] = ch;
            printf("*");
            fflush(stdout);
        }
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
#endif

    printf("\n");
    return atoi(pinStr);
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pauseScreen() {
    printf("\nPress Enter to continue...");
    getchar();
}

void initializeFiles() {
    FILE *file;

    // Create accounts file if it doesn't exist
    file = fopen(FILENAME_ACCOUNTS, "a");
    if (file != NULL) {
        fclose(file);
    }

    // Create transactions file if it doesn't exist
    file = fopen(FILENAME_TRANSACTIONS, "a");
    if (file != NULL) {
        fclose(file);
    }
}

int loadAccounts(Account accounts[]) {
    FILE *file = fopen(FILENAME_ACCOUNTS, "r");
    int count = 0;

    if (file == NULL) {
        return 0;
    }

    while (fscanf(file, "%d %49s %d %f",
                  &accounts[count].accountNumber,
                  accounts[count].name,
                  &accounts[count].pin,
                  &accounts[count].balance) == 4) {
        count++;
        if (count >= MAX_ACCOUNTS) break;
    }

    fclose(file);
    return count;
}

void saveAccounts(Account accounts[], int count) {
    FILE *file = fopen(FILENAME_ACCOUNTS, "w");

    if (file == NULL) {
        printf(" Error saving accounts!\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d %s %d %.2f\n",
                accounts[i].accountNumber,
                accounts[i].name,
                accounts[i].pin,
                accounts[i].balance);
    }

    fclose(file);
}

int login(Account accounts[], int count, Account *currentAccount) {
    int accountNumber, pin;

    printf("\n ===============Login===============\n");
    printf("Enter account number: ");
    scanf("%d", &accountNumber);
    clearInputBuffer();

    printf("Enter PIN: ");
    pin = getMaskedPIN();

    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == accountNumber && accounts[i].pin == pin) {
            *currentAccount = accounts[i];
            return 1;
        }
    }

    return 0;
}

void deposit(Account *acc, Account accounts[], int count) {
    float amount;

    printf("\n===============Deposit===============\n");
    printf("\nEnter amount to deposit: ");
    scanf("%f", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf("\nInvalid amount! Amount must be positive.\n");
        return;
    }

    acc->balance += amount;

    // Update the account in the accounts array
    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == acc->accountNumber) {
            accounts[i].balance = acc->balance;
            break;
        }
    }

    // Save updated accounts
    saveAccounts(accounts, count);

    // Save transaction
    saveTransaction(acc->accountNumber, "Deposit", amount);

    printf("\nDeposit successful! \n\nNew balance: $%.2f\n", acc->balance);
}

void withdraw(Account *acc, Account accounts[], int count) {
    float amount;

    printf("\n===============Withdraw===============\n");
    printf("Enter amount to withdraw: ");
    scanf("%f", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf(" Invalid amount! Amount must be positive.\n");
        return;
    }

    if (amount > acc->balance) {
        printf("\n\nInsufficient funds!\n\n Current balance: $%.2f\n", acc->balance);
        return;
    }

    acc->balance -= amount;

    // Update the account in the accounts array
    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == acc->accountNumber) {
            accounts[i].balance = acc->balance;
            break;
        }
    }

    // Save updated accounts
    saveAccounts(accounts, count);

    // Save transaction
    saveTransaction(acc->accountNumber, "Withdraw", amount);

    printf("\n\nWithdrawal successful! \n\nNew balance: $%.2f\n", acc->balance);
}

void checkBalance(Account acc) {
    printf("\n===============Balance Inquiry===============\n");
    printf("Account Holder: %s\n", acc.name);
    printf("Account Number: %d\n", acc.accountNumber);
    printf("Current Balance: $%.2f\n", acc.balance);
}

void viewTransactionHistory(int accountNumber) {
    FILE *file = fopen(FILENAME_TRANSACTIONS, "r");
    char line[200];
    int found = 0;
    int currentAccNo;
    char type[20];
    float amount;
    char date[20];
    char time[20];

    printf("==========================================\n");
    printf(" Transaction History for Account: %d\n", accountNumber);
    printf("==========================================\n");

    if (file == NULL) {
        printf(" No transaction history available.\n");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        // FIXED: Read with separate date and time fields
        if (sscanf(line, "%d %19s %f %19s %19s", &currentAccNo, type, &amount, date, time) == 5) {
            if (currentAccNo == accountNumber) {
                printf("%s %s - %s: $%.2f\n", date, time, type, amount);
                found = 1;
            }
        }
    }

    if (!found) {
        printf("No transactions found for this account.\n");
    }

    fclose(file);
}

void saveTransaction(int accNo, const char *type, float amount) {
    FILE *file = fopen(FILENAME_TRANSACTIONS, "a");
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char date[20];
    char time[20];

    if (file == NULL) {
        printf(" Error saving transaction!\n");
        return;
    }

    // FIXED: Format date and time separately
    strftime(date, sizeof(date), "%Y-%m-%d", tm_info);
    strftime(time, sizeof(time), "%H:%M:%S", tm_info);

    // Save with space-separated format
    fprintf(file, "%d %s %.2f %s %s\n", accNo, type, amount, date, time);
    fclose(file);
}

void displayMainMenu() {
    printf("\n ===============Main Menu===============\n");
    printf("1. Login\n");
    printf("2. Exit\n");
}

void displayUserMenu() {
    printf("\n ===============User Menu===============\n");
    printf("1. Deposit\n");
    printf("2. Withdraw\n");
    printf("3. Balance Inquiry\n");
    printf("4. Transaction History\n");
    printf("5. Logout\n");
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
