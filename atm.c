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
#define COLOR_BRIGHT_RED    "\x1B[91m"
#define COLOR_BRIGHT_GREEN  "\x1B[92m"
#define COLOR_BRIGHT_YELLOW "\x1B[93m"
#define COLOR_BRIGHT_CYAN   "\x1B[96m"

void enableConsoleColors() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}
#define MAX_LOGIN_ATTEMPTS 3
#define LOCKOUT_DURATION 300  // 5 minutes in seconds
#define FILENAME_LOCKOUT "lockout.txt"
#define MAX_ACCOUNTS 100
#define FILENAME_ACCOUNTS "accounts.txt"
#define FILENAME_TRANSACTIONS "transactions.txt"
#define MAX_DAILY_WITHDRAWAL 5000.00
#define MAX_SINGLE_WITHDRAWAL 2000.00
#define MAX_SINGLE_TRANSFER 3000.00
#define MAX_DAILY_TRANSACTIONS 10

// Define Account structure first
typedef struct {
    int accountNumber;
    char name[50];
    int pin;
    float balance;
    int isAdmin;  // New field to identify admin account
} Account;

// Forward declarations of all functions
void displayMainMenu();
void displayUserMenu();
void displayAdminMenu();
void clearInputBuffer();
void clearScreen();
void pauseScreen();
int getMaskedPIN();
void changePIN(Account *acc, Account accounts[], int count);
void fundTransfer(Account *acc, Account accounts[], int count);
void generateReceipt(Account acc);
void adminLockAccount(Account *admin, Account accounts[], int count);
void adminUnlockAccount(Account *admin, Account accounts[], int count);
typedef struct {
    int accountNumber;
    char date[20];
    float totalWithdrawn;
    int transactionCount;
} DailyLimit;

// Login security record for lockout handling
typedef struct {
    int accountNumber;
    int failedAttempts;
    time_t lockoutTime;
    int isLocked;
} LoginSecurity;

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
// Lockout / login security prototypes
void initializeLockoutFile();
int isAccountLocked(int accountNumber);
void recordFailedAttempt(int accountNumber);
void unlockAccount(int accountNumber);
void resetLoginAttempts(int accountNumber);
int getRemainingLockoutTime(int accountNumber);
void checkAndAutoUnlock(int accountNumber);
void displayLockoutStatus(int accountNumber);

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
void generateReceipt(Account acc);

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
    printf("%s            Version 8.0%s\n", COLOR_GREEN, COLOR_RESET);
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
                    int loginResult = login(accounts, accountCount, &currentAccount);
                    if (loginResult == 1) {  // Normal user login
                        loggedIn = 1;
                        printf("\n%sLogin successful! Welcome, %s!%s\n", COLOR_BRIGHT_GREEN, currentAccount.name, COLOR_RESET);
                        pauseScreen();
                    } else if (loginResult == 2) {  // Admin login
                        loggedIn = 2;  // Special state for admin
                        printf("\n%sAdmin login successful! Welcome, Administrator!%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
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
                    printf("%sVersion 8.0%s\n\n", COLOR_YELLOW, COLOR_RESET);
                    printf("%sDeveloped by Masud%s\n", COLOR_MAGENTA, COLOR_RESET);
                    exit(0);
                default:
                    printf("\n%sInvalid choice! Please try again.%s\n", COLOR_RED, COLOR_RESET);
                    pauseScreen();
            }
        } else if (loggedIn == 2) {  // Admin menu
            displayAdminMenu();
            printf("%sEnter your choice: %s", COLOR_WHITE, COLOR_RESET);
            scanf("%d", &choice);
            clearInputBuffer();

            switch (choice) {
                case 1:
                    clearScreen();
                    adminLockAccount(&currentAccount, accounts, accountCount);
                    pauseScreen();
                    break;
                case 2:
                    clearScreen();
                    adminUnlockAccount(&currentAccount, accounts, accountCount);
                    pauseScreen();
                    break;
                case 3:
                    clearScreen();
                    loggedIn = 0;
                    printf("\n%sAdmin logged out successfully!%s\n", COLOR_GREEN, COLOR_RESET);
                    pauseScreen();
                    break;
                default:
                    printf("\n%sInvalid choice! Please try again.%s\n", COLOR_RED, COLOR_RESET);
                    pauseScreen();
            }
        } else {  // Normal user menu
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
                    generateReceipt(currentAccount);
                    pauseScreen();
                    break;
                case 7:
                    clearScreen();
                    changePIN(&currentAccount, accounts, accountCount);
                    pauseScreen();
                    break;
                case 8:
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
    // Create lockout file if it doesn't exist
    initializeLockoutFile();
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

    // Check for admin login
    if (accountNumber == 999999) {
        printf("%sEnter PIN: %s", COLOR_YELLOW, COLOR_RESET);
        pin = getMaskedPIN();

        if (pin == 123456) {
            currentAccount->accountNumber = 999999;
            currentAccount->pin = 123456;
            strcpy(currentAccount->name, "Administrator");
            currentAccount->balance = 0;
            currentAccount->isAdmin = 1;
            return 2; // Special return code for admin login
        }
        return 0; // Failed admin login
    }

    // Check if account is locked
    checkAndAutoUnlock(accountNumber);  // Try auto-unlock first
    if (isAccountLocked(accountNumber)) {
        displayLockoutStatus(accountNumber);
        return 0;
    }

    printf("%sEnter PIN: %s", COLOR_YELLOW, COLOR_RESET);
    pin = getMaskedPIN();

    // Verify credentials
    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == accountNumber && accounts[i].pin == pin) {
            *currentAccount = accounts[i];
            currentAccount->isAdmin = 0;
            // Reset failed attempts on successful login
            resetLoginAttempts(accountNumber);
            return 1; // Successful login
        }
    }

    // Failed login - record attempt
    recordFailedAttempt(accountNumber);
    // Check if account just got locked
    if (isAccountLocked(accountNumber)) {
        displayLockoutStatus(accountNumber);
    }
    return 0; // Failed login
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
    // Get limits specifically for THIS account number
    float todayWithdrawal = getTodayWithdrawal(accountNumber);
    int todayTransactions = getTodayTransactionCount(accountNumber);

    // Check transaction count limit for THIS account
    if (todayTransactions >= MAX_DAILY_TRANSACTIONS) {
        clearScreen();
        printf("\n%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
        printf("%sDAILY TRANSACTION LIMIT REACHED!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
        printf("%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
        printf("%sAccount %s%d%s has reached the maximum%s\n", COLOR_RED, COLOR_YELLOW, accountNumber, COLOR_RED, COLOR_RESET);
        printf("%snumber of transactions per day (%s%d%s).%s\n", COLOR_RED, COLOR_YELLOW, MAX_DAILY_TRANSACTIONS, COLOR_RED, COLOR_RESET);
        printf("%sCurrent transactions today: %s%d%s\n", COLOR_CYAN, COLOR_YELLOW, todayTransactions, COLOR_RESET);
        printf("%sPlease try again tomorrow.%s\n", COLOR_YELLOW, COLOR_RESET);
        return 0;
    }

    // Check single withdrawal limit
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

    // Check daily withdrawal limit for THIS account
    if (strcmp(type, "Withdraw") == 0 || strcmp(type, "Transfer") == 0) {
        if (todayWithdrawal + amount > MAX_DAILY_WITHDRAWAL) {
            clearScreen();
            printf("\n%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%sDAILY WITHDRAWAL LIMIT EXCEEDED!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%s========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
            printf("%sAccount: %s%d%s\n", COLOR_CYAN, COLOR_YELLOW, accountNumber, COLOR_RESET);
            printf("%sDaily withdrawal limit: %s$%.2f%s\n", COLOR_CYAN, COLOR_WHITE, MAX_DAILY_WITHDRAWAL, COLOR_RESET);
            printf("%sAlready withdrawn today: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, todayWithdrawal, COLOR_RESET);
            printf("%sRemaining limit: %s$%.2f%s\n", COLOR_CYAN, COLOR_GREEN, MAX_DAILY_WITHDRAWAL - todayWithdrawal, COLOR_RESET);
            printf("%sYour requested amount: %s$%.2f%s\n", COLOR_CYAN, COLOR_YELLOW, amount, COLOR_RESET);
            return 0;
        }
    }

    return 1; // All checks passed for THIS account
}

// =============== LOCKOUT / LOGIN SECURITY IMPLEMENTATION ===============

// Initialize lockout file
void initializeLockoutFile() {
    FILE *file = fopen(FILENAME_LOCKOUT, "a");
    if (file != NULL) {
        fclose(file);
    }
}

// Check if account is locked and auto-unlock if time has passed
int isAccountLocked(int accountNumber) {
    FILE *file = fopen(FILENAME_LOCKOUT, "r");
    LoginSecurity record;
    time_t currentTime = time(NULL);
    if (file == NULL) {
        return 0; // Not locked if file doesn't exist
    }
    while (fscanf(file, "%d %d %ld %d",
                  &record.accountNumber,
                  &record.failedAttempts,
                  &record.lockoutTime,
                  &record.isLocked) == 4) {
        if (record.accountNumber == accountNumber) {
            fclose(file);
            if (record.isLocked) {
                double timeElapsed = difftime(currentTime, record.lockoutTime);
                if (timeElapsed >= LOCKOUT_DURATION) {
                    // Auto-unlock
                    unlockAccount(accountNumber);
                    return 0; // Not locked anymore
                }
                return 1; // Still locked
            }
            return 0; // Not locked
        }
    }
    fclose(file);
    return 0; // Account not found, not locked
}

// Get remaining lockout time in seconds
int getRemainingLockoutTime(int accountNumber) {
    FILE *file = fopen(FILENAME_LOCKOUT, "r");
    LoginSecurity record;
    time_t currentTime = time(NULL);
    if (file == NULL) {
        return 0;
    }
    while (fscanf(file, "%d %d %ld %d",
                  &record.accountNumber,
                  &record.failedAttempts,
                  &record.lockoutTime,
                  &record.isLocked) == 4) {
        if (record.accountNumber == accountNumber && record.isLocked) {
            fclose(file);
            double timeElapsed = difftime(currentTime, record.lockoutTime);
            int remainingTime = LOCKOUT_DURATION - (int)timeElapsed;
            return remainingTime > 0 ? remainingTime : 0;
        }
    }
    fclose(file);
    return 0;
}

// Display lockout status with countdown
void displayLockoutStatus(int accountNumber) {
    int remainingSeconds = getRemainingLockoutTime(accountNumber);
    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;
    clearScreen();
    printf("\n%s==========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
    printf("%s          ACCOUNT LOCKED!%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
    printf("%s==========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
    printf("\n%sAccount Number: %s%d%s\n", COLOR_CYAN, COLOR_YELLOW, accountNumber, COLOR_RESET);
    printf("\n%sYour account has been locked due to%s\n", COLOR_RED, COLOR_RESET);
    printf("%smultiple failed login attempts.%s\n", COLOR_RED, COLOR_RESET);
    printf("\n%sTime remaining: %s%02d:%02d minutes%s\n",
           COLOR_CYAN, COLOR_BRIGHT_YELLOW, minutes, seconds, COLOR_RESET);
    printf("\n%sPlease try again after the lockout period.%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("%s==========================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
}

// Record a failed login attempt
void recordFailedAttempt(int accountNumber) {
    FILE *file = fopen(FILENAME_LOCKOUT, "r");
    FILE *tempFile = fopen("temp_lockout.txt", "w");
    LoginSecurity record;
    int found = 0;
    time_t currentTime = time(NULL);
    if (tempFile == NULL) {
        printf("%sError: Unable to update login attempts!%s\n", COLOR_RED, COLOR_RESET);
        if (file) fclose(file);
        return;
    }
    if (file != NULL) {
        while (fscanf(file, "%d %d %ld %d",
                      &record.accountNumber,
                      &record.failedAttempts,
                      &record.lockoutTime,
                      &record.isLocked) == 4) {
            if (record.accountNumber == accountNumber) {
                found = 1;
                record.failedAttempts++;
                if (record.failedAttempts >= MAX_LOGIN_ATTEMPTS) {
                    record.isLocked = 1;
                    record.lockoutTime = currentTime;
                }
            }
            fprintf(tempFile, "%d %d %ld %d\n",
                    record.accountNumber,
                    record.failedAttempts,
                    record.lockoutTime,
                    record.isLocked);
        }
        fclose(file);
    }
    if (!found) {
        fprintf(tempFile, "%d %d %ld %d\n",
                accountNumber, 1, currentTime, 0);
    }
    fclose(tempFile);
    remove(FILENAME_LOCKOUT);
    rename("temp_lockout.txt", FILENAME_LOCKOUT);
    if (found) {
        FILE *checkFile = fopen(FILENAME_LOCKOUT, "r");
        if (checkFile != NULL) {
            while (fscanf(checkFile, "%d %d %ld %d",
                          &record.accountNumber,
                          &record.failedAttempts,
                          &record.lockoutTime,
                          &record.isLocked) == 4) {
                if (record.accountNumber == accountNumber) {
                    int remainingAttempts = MAX_LOGIN_ATTEMPTS - record.failedAttempts;
                    if (remainingAttempts > 0) {
                        printf("\n%sWARNING: Failed login attempt!%s\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
                        printf("%sRemaining attempts: %s%d%s\n",
                               COLOR_CYAN, COLOR_BRIGHT_RED, remainingAttempts, COLOR_RESET);
                        printf("%sAccount will be locked after %d failed attempts.%s\n",
                               COLOR_YELLOW, MAX_LOGIN_ATTEMPTS, COLOR_RESET);
                    }
                    break;
                }
            }
            fclose(checkFile);
        }
    }
}

// Unlock account (called automatically after timeout or manually by admin)
void unlockAccount(int accountNumber) {
    FILE *file = fopen(FILENAME_LOCKOUT, "r");
    FILE *tempFile = fopen("temp_lockout.txt", "w");
    LoginSecurity record;
    if (file == NULL || tempFile == NULL) {
        if (file) fclose(file);
        if (tempFile) fclose(tempFile);
        return;
    }
    while (fscanf(file, "%d %d %ld %d",
                  &record.accountNumber,
                  &record.failedAttempts,
                  &record.lockoutTime,
                  &record.isLocked) == 4) {
        if (record.accountNumber == accountNumber) {
            record.failedAttempts = 0;
            record.isLocked = 0;
            record.lockoutTime = 0;
        }
        fprintf(tempFile, "%d %d %ld %d\n",
                record.accountNumber,
                record.failedAttempts,
                record.lockoutTime,
                record.isLocked);
    }
    fclose(file);
    fclose(tempFile);
    remove(FILENAME_LOCKOUT);
    rename("temp_lockout.txt", FILENAME_LOCKOUT);
}

// Reset login attempts after successful login
void resetLoginAttempts(int accountNumber) {
    unlockAccount(accountNumber); // Same functionality
}

// Check and auto-unlock if time has passed
void checkAndAutoUnlock(int accountNumber) {
    if (isAccountLocked(accountNumber)) {
        int remainingTime = getRemainingLockoutTime(accountNumber);
        if (remainingTime <= 0) {
            unlockAccount(accountNumber);
        }
    }
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

void generateReceipt(Account acc) {
    FILE *file = fopen(FILENAME_TRANSACTIONS, "r");
    FILE *receiptFile;
    char receiptFilename[100];
    char line[200];
    int currentAccNo;
    char type[50];
    float amount;
    char transDate[20];
    char transTime[20];
    int transactionCount = 0;
    float totalDeposits = 0.0;
    float totalWithdrawals = 0.0;
    float totalTransfersIn = 0.0;
    float totalTransfersOut = 0.0;

    sprintf(receiptFilename, "receipt_%d.txt", acc.accountNumber);

    printf("\n%s========================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
    printf("%sGenerating Receipt...%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("%s========================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);

    if (file == NULL) {
        printf("%sNo transaction history available.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    receiptFile = fopen(receiptFilename, "w");
    if (receiptFile == NULL) {
        printf("%sError creating receipt file!%s\n", COLOR_RED, COLOR_RESET);
        fclose(file);
        return;
    }

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char currentDate[20];
    char currentTime[20];
    strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", tm_info);
    strftime(currentTime, sizeof(currentTime), "%H:%M:%S", tm_info);

    fprintf(receiptFile, "===============================================\n");
    fprintf(receiptFile, "           TRANSACTION RECEIPT\n");
    fprintf(receiptFile, "             ATM Simulation \n");
    fprintf(receiptFile, "===============================================\n\n");
    fprintf(receiptFile, "Generated: %s %s\n\n", currentDate, currentTime);
    fprintf(receiptFile, "----------- Account Information -----------\n");
    fprintf(receiptFile, "Account Holder: %s\n", acc.name);
    fprintf(receiptFile, "Account Number: %d\n", acc.accountNumber);
    fprintf(receiptFile, "Current Balance: $%.2f\n", acc.balance);
    fprintf(receiptFile, "-------------------------------------------\n\n");
    fprintf(receiptFile, "----------- Transaction History -----------\n");
    fprintf(receiptFile, "Date         Time       Type                 Amount\n");
    fprintf(receiptFile, "-------------------------------------------\n");

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d %49s %f %19s %19s", &currentAccNo, type, &amount, transDate, transTime) == 5) {
            if (currentAccNo == acc.accountNumber) {
                fprintf(receiptFile, "%s  %s  %-18s  $%.2f\n", transDate, transTime, type, amount);
                transactionCount++;

                if (strcmp(type, "Deposit") == 0) {
                    totalDeposits += amount;
                } else if (strcmp(type, "Withdraw") == 0) {
                    totalWithdrawals += amount;
                } else if (strncmp(type, "Transfer_from_", 14) == 0) {
                    totalTransfersIn += amount;
                } else if (strncmp(type, "Transfer_to_", 12) == 0) {
                    totalTransfersOut += amount;
                }
            }
        }
    }

    if (transactionCount == 0) {
        fprintf(receiptFile, "No transactions found for this account.\n");
    }

    fprintf(receiptFile, "-------------------------------------------\n\n");
    fprintf(receiptFile, "--------------- Summary -------------------\n");
    fprintf(receiptFile, "Total Transactions: %d\n", transactionCount);
    fprintf(receiptFile, "Total Deposits: $%.2f\n", totalDeposits);
    fprintf(receiptFile, "Total Withdrawals: $%.2f\n", totalWithdrawals);
    fprintf(receiptFile, "Total Transfers In: $%.2f\n", totalTransfersIn);
    fprintf(receiptFile, "Total Transfers Out: $%.2f\n", totalTransfersOut);
    fprintf(receiptFile, "-------------------------------------------\n\n");
    fprintf(receiptFile, "Net Change: $%.2f\n", (totalDeposits + totalTransfersIn) - (totalWithdrawals + totalTransfersOut));
    fprintf(receiptFile, "Current Balance: $%.2f\n", acc.balance);
    fprintf(receiptFile, "-------------------------------------------\n\n");
    fprintf(receiptFile, "Thank you for banking with us!\n");
    fprintf(receiptFile, "===============================================\n");

    fclose(file);
    fclose(receiptFile);

    printf("\n%sReceipt generated successfully!%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);

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
    printf("%s6.%s Generate Receipt\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s7.%s Change PIN\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s8.%s Logout\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s=================================================%s\n", COLOR_BRIGHT_CYAN, COLOR_RESET);
}

void displayAdminMenu() {
    printf("\n%s================== Admin Menu ==================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
    printf("%s1.%s Lock Account\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s2.%s Unlock Account\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s3.%s Logout\n", COLOR_BRIGHT_YELLOW, COLOR_RESET);
    printf("%s=================================================%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
}

void adminLockAccount(Account *admin, Account accounts[], int count) {
    int accountNumber;

    printf("\n%s=============== Lock Account ===============%s\n", COLOR_BRIGHT_RED, COLOR_RESET);
    printf("%sEnter account number to lock: %s", COLOR_YELLOW, COLOR_RESET);
    scanf("%d", &accountNumber);
    clearInputBuffer();

    // Verify account exists
    int accountExists = 0;
    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == accountNumber) {
            accountExists = 1;
            break;
        }
    }

    if (!accountExists) {
        printf("\n%sError: Account not found!%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    if (accountNumber == admin->accountNumber) {
        printf("\n%sError: Cannot lock admin account!%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    // Force lock the account
    FILE *file = fopen(FILENAME_LOCKOUT, "r");
    FILE *tempFile = fopen("temp_lockout.txt", "w");
    LoginSecurity record;
    int found = 0;
    time_t currentTime = time(NULL);

    if (file == NULL || tempFile == NULL) {
        if (file) fclose(file);
        if (tempFile) fclose(tempFile);
        printf("\n%sError: Could not access lockout file!%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    // Copy existing records and update/add the target account
    while (file && fscanf(file, "%d %d %ld %d",
                  &record.accountNumber,
                  &record.failedAttempts,
                  &record.lockoutTime,
                  &record.isLocked) == 4) {
        if (record.accountNumber == accountNumber) {
            found = 1;
            record.failedAttempts = MAX_LOGIN_ATTEMPTS;
            record.lockoutTime = currentTime;
            record.isLocked = 2; // 2 indicates manual lock
        }
        fprintf(tempFile, "%d %d %ld %d\n",
                record.accountNumber,
                record.failedAttempts,
                record.lockoutTime,
                record.isLocked);
    }

    if (!found) {
        fprintf(tempFile, "%d %d %ld %d\n",
                accountNumber,
                MAX_LOGIN_ATTEMPTS,
                currentTime,
                2); // 2 indicates manual lock
    }

    if (file) fclose(file);
    fclose(tempFile);
    remove(FILENAME_LOCKOUT);
    rename("temp_lockout.txt", FILENAME_LOCKOUT);

    printf("\n%sAccount %d has been locked successfully!%s\n", COLOR_BRIGHT_GREEN, accountNumber, COLOR_RESET);
    printf("%sOnly an admin can unlock this account.%s\n", COLOR_YELLOW, COLOR_RESET);
}

void adminUnlockAccount(Account *admin, Account accounts[], int count) {
    int accountNumber;

    printf("\n%s=============== Unlock Account ===============%s\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
    printf("%sEnter account number to unlock: %s", COLOR_YELLOW, COLOR_RESET);
    scanf("%d", &accountNumber);
    clearInputBuffer();

    // Verify account exists
    int accountExists = 0;
    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == accountNumber) {
            accountExists = 1;
            break;
        }
    }

    if (!accountExists) {
        printf("\n%sError: Account not found!%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    // Check if account is actually locked
    FILE *file = fopen(FILENAME_LOCKOUT, "r");
    LoginSecurity record;
    int isLocked = 0;

    if (file != NULL) {
        while (fscanf(file, "%d %d %ld %d",
                      &record.accountNumber,
                      &record.failedAttempts,
                      &record.lockoutTime,
                      &record.isLocked) == 4) {
            if (record.accountNumber == accountNumber && record.isLocked > 0) {
                isLocked = record.isLocked;
                break;
            }
        }
        fclose(file);
    }

    if (!isLocked) {
        printf("\n%sThis account is not locked!%s\n", COLOR_YELLOW, COLOR_RESET);
        return;
    }

    // Unlock the account
    unlockAccount(accountNumber);

    printf("\n%sAccount %d has been unlocked successfully!%s\n", COLOR_BRIGHT_GREEN, accountNumber, COLOR_RESET);
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

