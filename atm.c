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
void changePIN(Account *acc, Account accounts[], int count);
void fundTransfer(Account *acc, Account accounts[], int count);
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
    printf("            version 4.1"          "\n");
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
                    printf("\n Thank you for using our ATM.\n Goodbye!\n Version 4.1\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nDeveloped by Masud\n ");
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
void changePIN(Account *acc, Account accounts[], int count) {
    int oldPin, newPin, confirmPin;

    printf("\n===============Change PIN===============\n");

    // Verify current PIN
    printf("Enter current PIN: ");
    oldPin = getMaskedPIN();

    if (oldPin != acc->pin) {
        printf("\nIncorrect current PIN! PIN change failed.\n");
        return;
    }

    // Get new PIN
    printf("Enter new PIN (4-6 digits): ");
    newPin = getMaskedPIN();

    // Validate new PIN
    if (newPin < 1000 || newPin > 999999) {
        printf("\nInvalid PIN! PIN must be between 4-6 digits.\n");
        return;
    }

    // Check if new PIN is same as old PIN
    if (newPin == oldPin) {
        printf("\nNew PIN cannot be the same as current PIN!\n");
        return;
    }

    // Confirm new PIN
    printf("Confirm new PIN: ");
    confirmPin = getMaskedPIN();

    if (newPin != confirmPin) {
        printf("\nPINs do not match! PIN change failed.\n");
        return;
    }

    // Update PIN in current account
    acc->pin = newPin;

    // Update PIN in accounts array
    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == acc->accountNumber) {
            accounts[i].pin = newPin;
            break;
        }
    }

    // Save updated accounts to file
    saveAccounts(accounts, count);

    // Log the PIN change transaction
    saveTransaction(acc->accountNumber, "PIN_Change", 0.00);

    printf("\n\nPIN changed successfully!\n");
    printf("Please remember your new PIN.\n");
}
void fundTransfer(Account *acc, Account accounts[], int count) {
    int recipientAccountNumber;
    float amount;
    int recipientIndex = -1;
    char confirm;

    printf("\n===============Fund Transfer===============\n");

    // Get recipient account number
    printf("Enter recipient account number: ");
    scanf("%d", &recipientAccountNumber);
    clearInputBuffer();

    // Check if trying to transfer to own account
    if (recipientAccountNumber == acc->accountNumber) {
        printf("\nError: Cannot transfer to your own account!\n");
        return;
    }

    // Find recipient account
    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == recipientAccountNumber) {
            recipientIndex = i;
            break;
        }
    }

    if (recipientIndex == -1) {
        printf("\nError: Recipient account not found!\n");
        return;
    }

    // Display recipient name for confirmation
    printf("\nRecipient: %s\n", accounts[recipientIndex].name);
    printf("Recipient Account: %d\n", recipientAccountNumber);

    // Get transfer amount
    printf("\nEnter amount to transfer: $");
    scanf("%f", &amount);
    clearInputBuffer();

    // Validate amount
    if (amount <= 0) {
        printf("\nInvalid amount! Amount must be positive.\n");
        return;
    }

    // Check sufficient balance
    if (amount > acc->balance) {
        printf("\nInsufficient funds!\n");
        printf("Your current balance: $%.2f\n", acc->balance);
        printf("Transfer amount: $%.2f\n", amount);
        return;
    }

    // Confirm transfer
    clearScreen();
    printf("\n--- Transfer Summary ---\n");
    printf("From: %s (Account: %d)\n", acc->name, acc->accountNumber);
    printf("To: %s (Account: %d)\n", accounts[recipientIndex].name, recipientAccountNumber);
    printf("Amount: $%.2f\n", amount);
    printf("\nYour new balance will be: $%.2f\n", acc->balance - amount);
    printf("\nConfirm transfer? (y/n): ");
    scanf("%c", &confirm);
    clearInputBuffer();

    if (confirm != 'y' && confirm != 'Y') {
        clearScreen();
        printf("\nTransfer cancelled.\n");
        return;
    }

    // Perform transfer
    acc->balance -= amount;
    accounts[recipientIndex].balance += amount;

    // Update sender account in array
    for (int i = 0; i < count; i++) {
        if (accounts[i].accountNumber == acc->accountNumber) {
            accounts[i].balance = acc->balance;
            break;
        }
    }

    // Save updated accounts to file
    saveAccounts(accounts, count);

    // Save transactions for both accounts
    char transactionDesc[50];

    // Save sender transaction
    sprintf(transactionDesc, "Transfer_to_%d", recipientAccountNumber);
    saveTransaction(acc->accountNumber, transactionDesc, amount);

    // Save recipient transaction
    sprintf(transactionDesc, "Transfer_from_%d", acc->accountNumber);
    saveTransaction(recipientAccountNumber, transactionDesc, amount);

    clearScreen();
    printf("\n========================================\n");
    printf("         Transfer successful!\n");
    printf("========================================\n");
    printf("Amount transferred: $%.2f\n", amount);
    printf("To: %s (Account: %d)\n", accounts[recipientIndex].name, recipientAccountNumber);
    printf("Your new balance: $%.2f\n", acc->balance);
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
    printf("3. Fund Transfer\n");
    printf("4. Balance Inquiry\n");
    printf("5. Transaction History\n");
    printf("6. Change PIN\n");
    printf("7. Logout\n");
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
