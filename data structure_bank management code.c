/*
 * Complete Bank Management System in C
 * Features:
 * - Login System (Admin & Customer)
 * - Account Management
 * - Transactions (Deposit, Withdraw)
 * - Transaction History
 * - Interest Calculation
 * - Input Validation
 * - Account Search and Filtering
 * - Admin Account Editing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME 50
#define MAX_PASS 20
#define ADMIN_USER "admin"
#define ADMIN_PASS "admin123"

struct Account {
    int accNo;
    char name[MAX_NAME];
    float balance;
    float rate; // interest rate
};

// Function declarations
int login();
void createAccount();
void viewAccounts();
void deposit();
void withdraw();
void deleteAccount();
void viewTransactions(int accNo);
void calculateInterest();
int validateAmount(float amount);
void searchAccount();
void editAccount();

// File paths
const char *ACCOUNT_FILE = "accounts.dat";

int main() {
    if (!login()) {
        printf("\nLogin failed. Exiting.\n");
        return 0;
    }

    int choice;
    do {
        printf("\n=== Bank Management Menu ===\n");
        printf("1. Create Account\n");
        printf("2. View Accounts\n");
        printf("3. Deposit\n");
        printf("4. Withdraw\n");
        printf("5. View Transactions\n");
        printf("6. Calculate Interest\n");
        printf("7. Delete Account\n");
        printf("8. Search Account\n");
        printf("9. Edit Account\n");
        printf("10. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: createAccount(); break;
            case 2: viewAccounts(); break;
            case 3: deposit(); break;
            case 4: withdraw(); break;
            case 5: {
                int accNo;
                printf("Enter Account Number: ");
                scanf("%d", &accNo);
                viewTransactions(accNo);
                break;
            }
            case 6: calculateInterest(); break;
            case 7: deleteAccount(); break;
            case 8: searchAccount(); break;
            case 9: editAccount(); break;
            case 10: printf("Exiting...\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 10);

    return 0;
}

int login() {
    char user[MAX_NAME], pass[MAX_PASS];
    printf("\n=== Login ===\n");
    printf("Username: ");
    scanf("%s", user);
    printf("Password: ");
    scanf("%s", pass);

    if (strcmp(user, ADMIN_USER) == 0 && strcmp(pass, ADMIN_PASS) == 0) {
        return 1;
    }
    return 0;
}

void createAccount() {
    FILE *fp = fopen(ACCOUNT_FILE, "ab");
    if (!fp) {
        printf("Error: Could not open file.\n");
        return;
    }

    struct Account acc;
    printf("Enter Account Number: ");
    scanf("%d", &acc.accNo);
    printf("Enter Name: ");
    scanf(" %49[^\n]", acc.name); // Fixed format specifier
    do {
        printf("Enter Initial Deposit: ");
        scanf("%f", &acc.balance);
    } while (!validateAmount(acc.balance));

    printf("Enter Annual Interest Rate (%%): ");
    scanf("%f", &acc.rate);

    fwrite(&acc, sizeof(acc), 1, fp);
    fclose(fp);

    printf("Account created successfully.\n");
}

void viewAccounts() {
    FILE *fp = fopen(ACCOUNT_FILE, "rb");
    if (!fp) {
        printf("Error: Could not open file.\n");
        return;
    }

    struct Account acc;
    printf("\n%-10s %-20s %-10s %-10s\n", "Acc No", "Name", "Balance", "Rate");
    while (fread(&acc, sizeof(acc), 1, fp)) {
        printf("%-10d %-20s %-10.2f %-10.2f\n", acc.accNo, acc.name, acc.balance, acc.rate);
    }
    fclose(fp);
}

void logTransaction(int accNo, const char *type, float amount) {
    char filename[30];
    sprintf(filename, "txn_%d.log", accNo);
    FILE *fp = fopen(filename, "a");

    time_t now = time(NULL);
    fprintf(fp, "%s - %s: $%.2f\n", strtok(ctime(&now), "\n"), type, amount);
    fclose(fp);
}

void deposit() {
    FILE *fp = fopen(ACCOUNT_FILE, "rb+");
    if (!fp) {
        printf("Error: Could not open file.\n");
        return;
    }

    struct Account acc;
    int accNo;
    float amount;

    printf("Enter Account Number: ");
    scanf("%d", &accNo);
    printf("Enter Amount to Deposit: ");
    scanf("%f", &amount);
    if (!validateAmount(amount)) return;

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accNo == accNo) {
            acc.balance += amount;
            fseek(fp, -(long)sizeof(acc), SEEK_CUR); // Fixed overflow issue
            fwrite(&acc, sizeof(acc), 1, fp);
            logTransaction(accNo, "Deposit", amount);
            printf("Deposit successful.\n");
            fclose(fp);
            return;
        }
    }
    printf("Account not found.\n");
    fclose(fp);
}

void withdraw() {
    FILE *fp = fopen(ACCOUNT_FILE, "rb+");
    struct Account acc;
    int accNo;
    float amount;

    printf("Enter Account Number: ");
    scanf("%d", &accNo);
    printf("Enter Amount to Withdraw: ");
    scanf("%f", &amount);
    if (!validateAmount(amount)) return;

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accNo == accNo) {
            if (acc.balance >= amount) {
                acc.balance -= amount;
                fseek(fp, -sizeof(acc), SEEK_CUR);
                fwrite(&acc, sizeof(acc), 1, fp);
                logTransaction(accNo, "Withdraw", amount);
                printf("Withdrawal successful.\n");
            } else {
                printf("Insufficient funds.\n");
            }
            fclose(fp);
            return;
        }
    }
    printf("Account not found.\n");
    fclose(fp);
}

void deleteAccount() {
    FILE *fp = fopen(ACCOUNT_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    struct Account acc;
    int accNo;

    printf("Enter Account Number to Delete: ");
    scanf("%d", &accNo);

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accNo != accNo) {
            fwrite(&acc, sizeof(acc), 1, temp);
        }
    }

    fclose(fp);
    fclose(temp);
    remove(ACCOUNT_FILE);
    rename("temp.dat", ACCOUNT_FILE);

    printf("Account deleted if it existed.\n");
}

void viewTransactions(int accNo) {
    char filename[30];
    sprintf(filename, "txn_%d.log", accNo);
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("No transactions found.\n");
        return;
    }

    char line[100];
    printf("\nTransactions for Account %d:\n", accNo);
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    fclose(fp);
}

void calculateInterest() {
    FILE *fp = fopen(ACCOUNT_FILE, "rb+");
    struct Account acc;
    float timePeriod;

    printf("Enter Time Period in Years: ");
    scanf("%f", &timePeriod);

    while (fread(&acc, sizeof(acc), 1, fp)) {
        float interest = acc.balance * acc.rate * timePeriod / 100.0;
        acc.balance += interest;
        fseek(fp, -sizeof(acc), SEEK_CUR);
        fwrite(&acc, sizeof(acc), 1, fp);
        logTransaction(acc.accNo, "Interest", interest);
    }
    fclose(fp);
    printf("Interest applied to all accounts.\n");
}

int validateAmount(float amount) {
    if (amount <= 0) {
        printf("Amount must be positive.\n");
        return 0;
    }
    return 1;
}

void searchAccount() {
    FILE *fp = fopen(ACCOUNT_FILE, "rb");
    struct Account acc;
    char keyword[MAX_NAME];
    printf("Enter name keyword to search: ");
    scanf(" %49[^\n]", keyword);

    printf("\n%-10s %-20s %-10s %-10s\n", "Acc No", "Name", "Balance", "Rate");
    int found = 0;
    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strstr(acc.name, keyword)) {
            printf("%-10d %-20s %-10.2f %-10.2f\n", acc.accNo, acc.name, acc.balance, acc.rate);
            found = 1;
        }
    }
    if (!found) printf("No matching accounts found.\n");
    fclose(fp);
}

void editAccount() {
    FILE *fp = fopen(ACCOUNT_FILE, "rb+");
    struct Account acc;
    int accNo;
    printf("Enter account number to edit: ");
    scanf("%d", &accNo);

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accNo == accNo) {
            printf("Editing Account %d\n", acc.accNo);
            printf("Current Name: %s\n", acc.name);
            printf("Enter New Name: ");
            scanf(" %49[^\n]", acc.name);
            printf("Current Interest Rate: %.2f%%\n", acc.rate);
            printf("Enter New Interest Rate: ");
            scanf("%f", &acc.rate);

            fseek(fp, -sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);
            printf("Account updated successfully.\n");
            fclose(fp);
            return;
        }
    }
    printf("Account not found.\n");
    fclose(fp);
}
