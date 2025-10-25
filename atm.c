#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#else
#include <termios.h>
#include <unistd.h>
#endif

// Color definitions
#define COLOR_RESET   "\x1B[0m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_WHITE   "\x1B[37m"
#define COLOR_BRIGHT_RED     "\x1B[91m"
#define COLOR_BRIGHT_GREEN   "\x1B[92m"
#define COLOR_BRIGHT_YELLOW  "\x1B[93m"
#define COLOR_BRIGHT_CYAN    "\x1B[96m"

void enableConsoleColors() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}
#define MAX_ACCOUNTS 100
#define FILENAME_ACCOUNTS "accounts.txt"
#define FILENAME_TRANSACTIONS "transactions.txt"
#define MAX_DAILY_WITHDRAWAL 5000.00
#define MAX_SINGLE_WITHDRAWAL 2000.00
#define MAX_SINGLE_TRANSFER 3000.00
#define MAX_DAILY_TRANSACTIONS 10

typedef struct {
    int accountNumber;
    char name[50];
    int pin;
    float balance;
} Account;
typedef struct {
    int accountNumber;
    char date[20];
    float totalWithdrawn;
    int transactionCount;
} DailyLimit;

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
void changePIN(Account *acc, Account accounts[], int count);
void fundTransfer(Account *acc, Account accounts[], int count);
float getTodayWithdrawal(int accountNumber);
int getTodayTransactionCount(int accountNumber);
void saveDailyLimit(int accountNumber, float amount);
void incrementTransactionCount(int accountNumber);
int checkDailyLimit(int accountNumber, float amount, const char *type);

int main() {
    Account accounts[MAX_ACCOUNTS];
    Account currentAccount;
    int accountCount = 0;
    int choice;
    int loggedIn = 0;

    // Enable console colors
    enableConsoleColors();

    // Initialize files if they don't exist
    initializeFiles();

    // Load existing accounts
    accountCount = loadAccounts(accounts);

    clearScreen();
    printf("%s===================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%s     Welcome to ATM Simulation%s\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s            Version 6.0%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s===================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    pauseScreen();

    while (1) {
        clearScreen();
        if (!loggedIn) {
            displayMainMenu();
            printf("%sEnter your choice: %s", COLOR_WHITE, COLOR_RESET);
            scanf("%d", &choice);
            clearInputBuffer();

            switch (choice) {
                case 1:
                    clearScreen();
                    if (login(accounts, accountCount, &currentAccount)) {
                        loggedIn = 1;
                        printf("\n%sLogin successful! Welcome, %s!%s\n", COLOR_BRIGHT_GREEN, currentAccount.name, COLOR_RESET);
                        pauseScreen();
                    } else {
                        printf("\n%sLogin failed! Invalid account number or PIN.%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
                        pauseScreen();
                    }
                    break;
                case 2:
                    clearScreen();
                    printf("\n%sThank you for using our ATM.%s\n", COLOR_CYAN, COLOR_RESET);
                    printf("%sGoodbye!%s\n", COLOR_GREEN, COLOR_RESET);
                    printf("%sVersion 6.0%s\n\n", COLOR_YELLOW, COLOR_RESET);
                    printf("%sDeveloped by Masud%s\n", COLOR_MAGENTA, COLOR_RESET);
                    exit(0);
                default:
                    printf("\n%sInvalid choice! Please try again.%s\n", COLOR_RED, COLOR_RESET);
                    pauseScreen();
            }
        } else {
            displayUserMenu();
            printf("%sEnter your choice: %s", COLOR_WHITE, COLOR_RESET);
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
                    fundTransfer(&currentAccount, accounts, accountCount);
                    pauseScreen();
                    break;
                case 4:
                    clearScreen();
                    checkBalance(currentAccount);
                    pauseScreen();
                    break;
                case 5:
                    clearScreen();
                    viewTransactionHistory(currentAccount.accountNumber);
                    pauseScreen();
                    break;
                case 6:
                    clearScreen();
                    changePIN(&currentAccount, accounts, accountCount);
                    pauseScreen();
                    break;
                case 7:
                    clearScreen();
                    loggedIn = 0;
                    printf("\n%sLogged out successfully!%s\n", COLOR_GREEN, COLOR_RESET);
                    pauseScreen();
                    break;
                default:
                    printf("\n%sInvalid choice! Please try again.%s\n", COLOR_RED, COLOR_RESET);
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
            printf("%s*%s", COLOR_YELLOW, COLOR_RESET);
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
            printf("%s*%s", COLOR_YELLOW, COLOR_RESET);
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
    printf("\n%sPress Enter to continue...%s", COLOR_CYAN, COLOR_RESET);
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
        printf("%sError saving accounts!%s\n", COLOR_RED, COLOR_RESET);
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

    printf("\n%s=================== Login ===================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%sEnter account number: %s", COLOR_YELLOW, COLOR_RESET);
    scanf("%d", &accountNumber);
    clearInputBuffer();

    printf("%sEnter PIN: %s", COLOR_YELLOW, COLOR_RESET);
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

    printf("\n%s=============== Deposit ===============%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("\n%sEnter amount to deposit: %s$", COLOR_YELLOW, COLOR_RESET);
    scanf("%f", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf("\n%sInvalid amount! Amount must be positive.%s\n", COLOR_RED, COLOR_RESET);
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

    printf("\n%sDeposit successful!%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
    printf("\n%sNew balance: %s$%.2f%s\n", COLOR_CYAN, COLOR_BRIGHT_GREEN, acc->balance, COLOR_RESET);
}

void withdraw(Account *acc, Account accounts[], int count) {
    float amount;

    printf("\n%s=============== Withdraw ===============%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%sDaily Withdrawal Limit: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, MAX_DAILY_WITHDRAWAL, COLOR_RESET);
    printf("%sSingle Withdrawal Limit: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, MAX_SINGLE_WITHDRAWAL, COLOR_RESET);
    printf("%sAlready withdrawn today: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, getTodayWithdrawal(acc->accountNumber), COLOR_RESET);
    printf("%sTransactions today: %s%d/%d%s\n", COLOR_CYAN, COLOR_YELLOW, getTodayTransactionCount(acc->accountNumber), MAX_DAILY_TRANSACTIONS, COLOR_RESET);
    printf("%s========================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);

    printf("%sEnter amount to withdraw: %s$", COLOR_YELLOW, COLOR_RESET);
    scanf("%f", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf("\n%sInvalid amount! Amount must be positive.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    // Check transaction limits
    if (!checkDailyLimit(acc->accountNumber, amount, "Withdraw")) {
        return;
    }

    if (amount > acc->balance) {
        printf("\n%sInsufficient funds!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
        printf("%sCurrent balance: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, acc->balance, COLOR_RESET);
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

    printf("\n%sWithdrawal successful!%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
    printf("\n%sNew balance: %s$%.2f%s\n", COLOR_CYAN, COLOR_BRIGHT_GREEN, acc->balance, COLOR_RESET);
}

void checkBalance(Account acc) {
    printf("\n%s================ Balance Inquiry ================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%sAccount Holder: %s%s%s\n", COLOR_CYAN, COLOR_BRIGHT_YELLOW, acc.name, COLOR_RESET);
    printf("%sAccount Number: %s%d%s\n", COLOR_CYAN, COLOR_WHITE, acc.accountNumber, COLOR_RESET);
    printf("%sCurrent Balance: %s$%.2f%s\n", COLOR_CYAN, COLOR_BRIGHT_GREEN, acc.balance, COLOR_RESET);
    printf("%s================================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
}

void viewTransactionHistory(int accountNumber) {
    FILE *file = fopen(FILENAME_TRANSACTIONS, "r");
    char line[200];
    int found = 0;
    int currentAccNo;
    char type[50];
    float amount;
    char date[20];
    char time[20];

    printf("%s==========================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%sTransaction History for Account: %s%d%s\n", COLOR_CYAN, COLOR_BRIGHT_YELLOW, accountNumber, COLOR_RESET);
    printf("%s==========================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);

    if (file == NULL) {
        printf("%sNo transaction history available.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d %49s %f %19s %19s", &currentAccNo, type, &amount, date, time) == 5) {
            if (currentAccNo == accountNumber) {
                printf("%s%s %s%s - ", COLOR_CYAN, date, time, COLOR_RESET);

                // Color code based on transaction type
                if (strstr(type, "Deposit") || strstr(type, "Transfer_from")) {
                    printf("%s%s: %s$%.2f%s\n", COLOR_GREEN, type, COLOR_BRIGHT_GREEN, amount, COLOR_RESET);
                } else if (strstr(type, "Withdraw") || strstr(type, "Transfer_to")) {
                    printf("%s%s: %s$%.2f%s\n", COLOR_YELLOW, type, COLOR_BRIGHT_YELLOW, amount, COLOR_RESET);
                } else {
                    printf("%s%s: %s$%.2f%s\n", COLOR_WHITE, type, COLOR_WHITE, amount, COLOR_RESET);
                }
                found = 1;
            }
        }
    }

    if (!found) {
        printf("%sNo transactions found for this account.%s\n", COLOR_YELLOW, COLOR_RESET);
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
        printf("%sError saving transaction!%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    strftime(date, sizeof(date), "%Y-%m-%d", tm_info);
    strftime(time, sizeof(time), "%H:%M:%S", tm_info);

    fprintf(file, "%d %s %.2f %s %s\n", accNo, type, amount, date, time);
    fclose(file);
}

float getTodayWithdrawal(int accountNumber) {
    FILE *file = fopen(FILENAME_TRANSACTIONS, "r");
    char line[200];
    int accNo;
    char type[20];
    float amount;
    char transDate[20];
    char transTime[20];
    float totalWithdrawn = 0.0;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char today[20];
    strftime(today, sizeof(today), "%Y-%m-%d", tm_info);

    if (file == NULL) {
        return 0.0;
    }

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d %19s %f %19s %19s", &accNo, type, &amount, transDate, transTime) == 5) {
            if (accNo == accountNumber && strcmp(transDate, today) == 0) {
                if (strcmp(type, "Withdraw") == 0 || strncmp(type, "Transfer_to_", 12) == 0) {
                    totalWithdrawn += amount;
                }
            }
        }
    }

    fclose(file);
    return totalWithdrawn;
}

int getTodayTransactionCount(int accountNumber) {
    FILE *file = fopen(FILENAME_TRANSACTIONS, "r");
    char line[200];
    int accNo;
    char type[20];
    float amount;
    char transDate[20];
    char transTime[20];
    int count = 0;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char today[20];
    strftime(today, sizeof(today), "%Y-%m-%d", tm_info);

    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d %19s %f %19s %19s", &accNo, type, &amount, transDate, transTime) == 5) {
            if (accNo == accountNumber && strcmp(transDate, today) == 0) {
                count++;
            }
        }
    }

    fclose(file);
    return count;
}

int checkDailyLimit(int accountNumber, float amount, const char *type) {
    float todayWithdrawal = getTodayWithdrawal(accountNumber);
    int todayTransactions = getTodayTransactionCount(accountNumber);

    if (todayTransactions >= MAX_DAILY_TRANSACTIONS) {
        clearScreen();
        printf("\n%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
        printf("%sDAILY TRANSACTION LIMIT REACHED!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
        printf("%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
        printf("%sYou have reached the maximum number of%s\n", COLOR_RED, COLOR_RESET);
        printf("%stransactions allowed per day (%s%d%s).%s\n", COLOR_RED, COLOR_YELLOW, MAX_DAILY_TRANSACTIONS, COLOR_RED, COLOR_RESET);
        printf("%sCurrent transactions today: %s%d%s\n", COLOR_CYAN, COLOR_YELLOW, todayTransactions, COLOR_RESET);
        printf("%sPlease try again tomorrow.%s\n", COLOR_YELLOW, COLOR_RESET);
        return 0;
    }

    if (strcmp(type, "Withdraw") == 0) {
        if (amount > MAX_SINGLE_WITHDRAWAL) {
            clearScreen();
            printf("\n%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%sSINGLE WITHDRAWAL LIMIT EXCEEDED!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%sMaximum withdrawal per transaction: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, MAX_SINGLE_WITHDRAWAL, COLOR_RESET);
            printf("%sYour requested amount: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, amount, COLOR_RESET);
            pauseScreen();
            return 0;
        }
    } else if (strcmp(type, "Transfer") == 0) {
        if (amount > MAX_SINGLE_TRANSFER) {
            clearScreen();
            printf("\n%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%sSINGLE TRANSFER LIMIT EXCEEDED!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%sMaximum transfer per transaction: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, MAX_SINGLE_TRANSFER, COLOR_RESET);
            printf("%sYour requested amount: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, amount, COLOR_RESET);
            pauseScreen();
            return 0;
        }
    }

    if (strcmp(type, "Withdraw") == 0 || strcmp(type, "Transfer") == 0) {
        if (todayWithdrawal + amount > MAX_DAILY_WITHDRAWAL) {
            clearScreen();
            printf("\n%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%sDAILY WITHDRAWAL LIMIT EXCEEDED!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%sDaily withdrawal limit: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, MAX_DAILY_WITHDRAWAL, COLOR_RESET);
            printf("%sAlready withdrawn today: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, todayWithdrawal, COLOR_RESET);
            printf("%sRemaining limit: %s$%.2f%s\n", COLOR_CYAN, COLOR_GREEN, MAX_DAILY_WITHDRAWAL - todayWithdrawal, COLOR_RESET);
            printf("%sYour requested amount: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, amount, COLOR_RESET);
            return 0;
        }
    }

    return 1;
}

void changePIN(Account *acc, Account accounts[], int count) {
    int oldPin, newPin, confirmPin;

    printf("\n%s=============== Change PIN ===============%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);

    printf("%sEnter current PIN: %s", COLOR_YELLOW, COLOR_RESET);
    oldPin = getMaskedPIN();

    if (oldPin != acc->pin) {
        printf("\n%sIncorrect current PIN! PIN change failed.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    printf("%sEnter new PIN (4-6 digits): %s", COLOR_YELLOW, COLOR_RESET);
    newPin = getMaskedPIN();

    if (newPin < 1000 || newPin > 999999) {
        printf("\n%sInvalid PIN! PIN must be between 4-6 digits.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    if (newPin == oldPin) {
        printf("\n%sNew PIN cannot be the same as current PIN!%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    printf("%sConfirm new PIN: %s", COLOR_YELLOW, COLOR_RESET);
    confirmPin = getMaskedPIN();

    if (newPin != confirmPin) {
        printf("\n%sPINs do not match! PIN change failed.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    acc->pin = newPin;

    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == acc->accountNumber) {
            accounts[i].pin = newPin;
            break;
        }
    }

    saveAccounts(accounts, count);
    saveTransaction(acc->accountNumber, "PIN_Change", 0.00);

    printf("\n%sPIN changed successfully!%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
    printf("%sPlease remember your new PIN.%s\n", COLOR_CYAN, COLOR_RESET);
}

void fundTransfer(Account *acc, Account accounts[], int count) {
    int recipientAccountNumber;
    float amount;
    int recipientIndex = -1;
    char confirm;

    printf("\n%s=============== Fund Transfer ===============%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%sDaily Withdrawal Limit: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, MAX_DAILY_WITHDRAWAL, COLOR_RESET);
    printf("%sSingle Transfer Limit: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, MAX_SINGLE_TRANSFER, COLOR_RESET);
    printf("%sAlready withdrawn today: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, getTodayWithdrawal(acc->accountNumber), COLOR_RESET);
    printf("%sTransactions today: %s%d/%d%s\n", COLOR_CYAN, COLOR_YELLOW, getTodayTransactionCount(acc->accountNumber), MAX_DAILY_TRANSACTIONS, COLOR_RESET);
    pauseScreen();

    clearScreen();
    printf("%sEnter recipient account number: %s", COLOR_YELLOW, COLOR_RESET);
    scanf("%d", &recipientAccountNumber);
    clearInputBuffer();

    if (recipientAccountNumber == acc->accountNumber) {
        printf("\n%sError: Cannot transfer to your own account!%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == recipientAccountNumber) {
            recipientIndex = i;
            break;
        }
    }

    if (recipientIndex == -1) {
        printf("\n%sError: Recipient account not found!%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    printf("\n%sRecipient: %s%s%s\n", COLOR_CYAN, COLOR_BRIGHT_YELLOW, accounts[recipientIndex].name, COLOR_RESET);
    printf("%sRecipient Account: %s%d%s\n", COLOR_CYAN, COLOR_WHITE, recipientAccountNumber, COLOR_RESET);

    printf("\n%sEnter amount to transfer: %s$", COLOR_YELLOW, COLOR_RESET);
    scanf("%f", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf("\n%sInvalid amount! Amount must be positive.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    if (!checkDailyLimit(acc->accountNumber, amount, "Transfer")) {
        return;
    }

    if (amount > acc->balance) {
        printf("\n%sInsufficient funds!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
        printf("%sYour current balance: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, acc->balance, COLOR_RESET);
        printf("%sTransfer amount: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, amount, COLOR_RESET);
        return;
    }

    clearScreen();
    printf("\n%s--- Transfer Summary ---%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%sFrom: %s%s %s(%sAccount: %s%d%s)%s\n", COLOR_CYAN, COLOR_BRIGHT_YELLOW, acc->name, COLOR_CYAN, COLOR_RESET, COLOR_WHITE, acc->accountNumber, COLOR_CYAN, COLOR_RESET);
    printf("%sTo: %s%s %s(%sAccount: %s%d%s)%s\n", COLOR_CYAN, COLOR_BRIGHT_YELLOW, accounts[recipientIndex].name, COLOR_CYAN, COLOR_RESET, COLOR_WHITE, recipientAccountNumber, COLOR_CYAN, COLOR_RESET);
    printf("%sAmount: %s$%.2f%s\n", COLOR_CYAN, COLOR_BRIGHT_GREEN, amount, COLOR_RESET);
    printf("\n%sYour new balance will be: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, acc->balance - amount, COLOR_RESET);
    printf("\n%sConfirm transfer? (y/n): %s", COLOR_YELLOW, COLOR_RESET);
    scanf("%c", &confirm);
    clearInputBuffer();

    if (confirm != 'y' && confirm != 'Y') {
        clearScreen();
        printf("\n%sTransfer cancelled.%s\n", COLOR_YELLOW, COLOR_RESET);
        return;
    }

    acc->balance -= amount;
    accounts[recipientIndex].balance += amount;

    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == acc->accountNumber) {
            accounts[i].balance = acc->balance;
            break;
        }
    }

    saveAccounts(accounts, count);

    char transactionDesc[50];

    sprintf(transactionDesc, "Transfer_to_%d", recipientAccountNumber);
    saveTransaction(acc->accountNumber, transactionDesc, amount);

    sprintf(transactionDesc, "Transfer_from_%d", acc->accountNumber);
    saveTransaction(recipientAccountNumber, transactionDesc, amount);

    clearScreen();
    printf("\n%s========================================%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
    printf("%sTransfer completed successfully!%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
    printf("%s========================================%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
    printf("%sAmount transferred: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, amount, COLOR_RESET);
    printf("%sTo: %s%s %s(%sAccount: %s%d%s)%s\n", COLOR_CYAN, COLOR_BRIGHT_YELLOW, accounts[recipientIndex].name, COLOR_CYAN, COLOR_RESET, COLOR_WHITE, recipientAccountNumber, COLOR_CYAN, COLOR_RESET);
    printf("%sYour new balance: %s$%.2f%s\n", COLOR_CYAN, COLOR_BRIGHT_GREEN, acc->balance, COLOR_RESET);
}

void displayMainMenu() {
    printf("\n%s=================== Main Menu ===================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%s1.%s Login\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s2.%s Exit\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s=================================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
}

void displayUserMenu() {
    printf("\n%s=================== User Menu ===================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%s1.%s Deposit\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s2.%s Withdraw\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s3.%s Fund Transfer\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s4.%s Balance Inquiry\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s5.%s Transaction History\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s6.%s Change PIN\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s7.%s Logout\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s=================================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
