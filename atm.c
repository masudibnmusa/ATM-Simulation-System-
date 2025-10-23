#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

    printf(" Welcome to ATM Simulation System\n");
    printf("===================================\n");

    while (1) {
        if (!loggedIn) {
            displayMainMenu();
            printf("Enter your choice: ");
            scanf("%d", &choice);
            clearInputBuffer();

            switch (choice) {
                case 1:
                    if (login(accounts, accountCount, &currentAccount)) {
                        loggedIn = 1;
                        printf("\n Login successful! Welcome, %s!\n", currentAccount.name);
                    } else {
                        printf("\n Login failed! Invalid account number or PIN.\n");
                    }
                    break;
                case 2:
                    printf("\nThank you for using our ATM. Goodbye! \n");
                    exit(0);
                default:
                    printf("\n Invalid choice! Please try again.\n");
            }
        } else {
            displayUserMenu();
            printf("Enter your choice: ");
            scanf("%d", &choice);
            clearInputBuffer();

            switch (choice) {
                case 1:
                    deposit(&currentAccount, accounts, accountCount);
                    break;
                case 2:
                    withdraw(&currentAccount, accounts, accountCount);
                    break;
                case 3:
                    checkBalance(currentAccount);
                    break;
                case 4:
                    viewTransactionHistory(currentAccount.accountNumber);
                    break;
                case 5:
                    loggedIn = 0;
                    printf("\n Logged out successfully!\n");
                    break;
                default:
                    printf("\n Invalid choice! Please try again.\n");
            }
        }
        printf("\n");
    }

    return 0;
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

    printf("\n Login\n");
    printf("Enter account number: ");
    scanf("%d", &accountNumber);
    printf("Enter PIN: ");
    scanf("%d", &pin);
    clearInputBuffer();

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

    printf("\n Deposit\n");
    printf("Enter amount to deposit: ");
    scanf("%f", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf(" Invalid amount! Amount must be positive.\n");
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

    printf(" Deposit successful! New balance: $%.2f\n", acc->balance);
}

void withdraw(Account *acc, Account accounts[], int count) {
    float amount;

    printf("\n Withdraw\n");
    printf("Enter amount to withdraw: ");
    scanf("%f", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf(" Invalid amount! Amount must be positive.\n");
        return;
    }

    if (amount > acc->balance) {
        printf(" Insufficient funds! Current balance: $%.2f\n", acc->balance);
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

    printf(" Withdrawal successful! New balance: $%.2f\n", acc->balance);
}

void checkBalance(Account acc) {
    printf("\n Balance Inquiry\n");
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
    char datetime[20];

    printf("\n Transaction History for Account: %d\n", accountNumber);
    printf("==========================================\n");

    if (file == NULL) {
        printf(" No transaction history available.\n");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d %19s %f %19s", &currentAccNo, type, &amount, datetime) == 4) {
            if (currentAccNo == accountNumber) {
                printf("%s - %s: $%.2f\n", datetime, type, amount);
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
    char datetime[20];

    if (file == NULL) {
        printf(" Error saving transaction!\n");
        return;
    }

    // Format date as YYYY-MM-DD HH:MM
    strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M", tm_info);

    fprintf(file, "%d %s %.2f %s\n", accNo, type, amount, datetime);
    fclose(file);
}

void displayMainMenu() {
    printf("\n Main Menu\n");
    printf("1. Login\n");
    printf("2. Exit\n");
}

void displayUserMenu() {
    printf("\n User Menu\n");
    printf("1. Deposit\n");
    printf("2. Withdraw\n");
    printf("3. Balance Inquiry\n");
    printf("4.Transaction History\n");
    printf("5. Logout\n");
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
