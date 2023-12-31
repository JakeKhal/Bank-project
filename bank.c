#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TICKERS 100

typedef struct {
    char ticker[20];
    float price;
} TickerInfo;

struct User {
    char name[50];
    char email[100];
    char pin[5];
    double balance;
    int credit_score;
    double debt;
};

// VERIFY USER
int verifyUser(const char *name, const char *pin) {
    FILE *file = fopen("clients.txt", "r+");

    if (file == NULL) {
        printf("ERROR: Could not open or find the 'clients.txt' file.\n");
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char clientName[50], clientPin[5];
        if (sscanf(line, "%49s | Email: %*s | PIN: %4s | Credit Score: %*d | Balance: %*f | Debt: %*f", clientName, clientPin) == 2) {
            if (strcmp(clientName, name) == 0 && strcmp(clientPin, pin) == 0) {
                fclose(file);
                return 1;
            }
        }
    }
    fclose(file);
    return 0;
}

// VERIFY PIN IS 4 DIGITS
bool isValidPIN(const char *pin) {
    // Check if the PIN is exactly four characters long and consists of digits
    return (strlen(pin) == 4 && strspn(pin, "0123456789") == 4);
}

// UPDATE USER BALANCE
void updateUserBalance(const struct User *user, double newBalance) {
    FILE *file = fopen("clients.txt", "r+");
    FILE *tempFile = fopen("temp.txt", "w");

    if (file == NULL) {
        printf("ERROR: Could not open or find the 'clients.txt' file.\n");
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char clientName[50];
        if (sscanf(line, "%49s", clientName) == 1) {
            if (strcmp(clientName, user->name) == 0) {
                // Update the balance in the line
                char email[100];
                int credit_score;

                // Parse the existing email and credit score values
                if (sscanf(line, "%*[^|] | Email: %[^|] | PIN: %*s | Credit Score: %d | Balance: %*f | Debt: %*f", email, &credit_score) == 2) {
                    // Write the updated line with the correct email and credit score
                    fprintf(tempFile, "%s | Email: %s | PIN: %s | Credit Score: %d | Balance: %.2lf | Debt: %.2lf\n", user->name, email, user->pin, credit_score, newBalance, user->debt);
                }
            } else {
                fputs(line, tempFile);
            }
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the temporary file
    remove("clients.txt");
    rename("temp.txt", "clients.txt");
}

// RECORD STOCK PRICE
void recordStockPurchase(const struct User *user, const char *stockName, int shares) {
    FILE *file = fopen("stock_purchases.csv", "a");

    if (file == NULL) {
        printf("ERROR: Could not open or find the 'stock_purchases.csv' file.\n");
    }

    // SAVE TO RECORDS
    char purchaseRecord[100];
    sprintf(purchaseRecord, "%s | %s | %d\n", user->name, stockName, shares);

    fprintf(file, "%s", purchaseRecord);
    fclose(file);
}

// APPLY FOR A LOAN
void applyForLoan(struct User *user) {
    printf("Current Credit Score: %d\n", user->credit_score);

    double loanAmount;
    printf("Enter the loan amount: $");
    scanf("%lf", &loanAmount);

    if (loanAmount <= 0) {
        printf("Invalid loan amount.\n");
        return;
    }

    double interestRate;

    if (user->credit_score >= 800) {
        interestRate = 0.01; // 1%
    } else if (user->credit_score >= 740) {
        interestRate = 0.02; // 2%
    } else if (user->credit_score >= 670) {
        interestRate = 0.03; // 3%
    } else if (user->credit_score >= 580) {
        interestRate = 0.04; // 4%
    } else { // credit too low
        printf("Sorry, you did not qualify for a loan.\n");
        return;
    }

    // Process the loan
    double debt_from_loan = loanAmount * (1 + interestRate); // total amount to repay
    user->debt += debt_from_loan;                             // Add debt with interest
    user->balance = user->balance + loanAmount;          // Add loan to balance

    printf("Loan of $%.2lf approved with %.2lf%% interest. Debt: $%.2lf\n", loanAmount, interestRate * 100, user->debt);

    updateUserBalance(user, user->balance);
}

// BUY STOCKS
void buyStocks(struct User *user) {
    TickerInfo tickers[MAX_TICKERS];
    int numTickers = 0;
    FILE *stock_file;
    char line2[100];

    stock_file = fopen("stocks.csv", "r");

    if (stock_file == NULL) {
        printf("ERROR: Could not open or find the 'stocks.csv' file.\n");
    }

    // GET TICKERS AND PRICE
    while (fgets(line2, sizeof(line2), stock_file) && numTickers < MAX_TICKERS) {
        if (sscanf(line2, "%[^,],%f", tickers[numTickers].ticker, &tickers[numTickers].price) == 2) {
            numTickers++;
        }
    }
    fclose(stock_file);

    printf("Available Tickers:\n");
    for (int i = 0; i < numTickers; i++) {
        printf("%d. %s\n", i + 1, tickers[i].ticker);
    }

    // CHOOSE TICKER
    int choice;
    printf("Enter the number corresponding to the ticker you want to purchase: ");
    scanf("%d", &choice);

    // CHECK IF CHOICE IS VALID
    if (choice < 1 || choice > numTickers) {
        printf("Invalid choice. Please enter a valid number.\n");
        return;
    }

    printf("Price of %s: $%.2f\n", tickers[choice - 1].ticker, tickers[choice - 1].price);

    // QUANTITY OF SHARES
    int shares;
    printf("Enter the amount of shares you want to purchase: ");
    scanf("%d", &shares);

    double total_purchase = shares * (tickers[choice - 1].price);

    // CONFIRM STOCK PURCHASE
    printf("Confirm purchase order of: $%.2lf? (yes/no)", total_purchase);
    char confirm[10];
    scanf("%s", confirm);

    if (strcmp(confirm, "yes") == 0) {
        user->balance -= total_purchase;
        recordStockPurchase(user, tickers[choice - 1].ticker, shares);
        printf("Purchase successful!\nCurrent balance: $%.2lf\n", user->balance);
        updateUserBalance(user, user->balance);
    } else {
        printf("Order cancelled");
    }
}

// ACCESS ATM
void accessATM(struct User *user) {
    FILE *file = fopen("clients.txt", "r");

    if (file == NULL) {
        printf("ERROR: Could not open or find the 'clients.txt' file.\n");
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char clientName[50];
        if (sscanf(line, "%49s", clientName) == 1) {
            if (strcmp(clientName, user->name) == 0) {
                sscanf(line, "%*[^|] | Email: %*s | PIN: %*s | Credit Score: %*d | Balance: %lf | Debt: %*lf", &(user->balance));
                break;
            }
        }
    }
    fclose(file);

    printf("\nBalance: %.2lf \n", user->balance);
    printf("1. Withdraw\n");
    printf("2. Deposit\n");
    printf("Enter your choice: ");
    int action;
    scanf("%d", &action);

    switch (action) {
    case 1: // Withdraw
        printf("Enter the withdrawal amount: ");
        double withdrawalAmount;
        scanf("%lf", &withdrawalAmount);

        if (withdrawalAmount < 0 || withdrawalAmount > user->balance) {
            printf("Invalid withdrawal amount or insufficient funds.\n");
        } else {
            user->balance -= withdrawalAmount;
            printf("Withdrawal of $%.2lf successful.\nCurrent balance: $%.2lf\n", withdrawalAmount, user->balance);
            updateUserBalance(user, user->balance);
        }
        break;

    case 2: // Deposit
        printf("Enter the deposit amount: ");
        double depositAmount;
        scanf("%lf", &depositAmount);

        if (depositAmount < 0) {
            printf("Invalid deposit amount.\n");
        } else {
            user->balance += depositAmount;
            printf("Deposit of $%.2lf successful.\nCurrent balance: %.2lf\n", depositAmount, user->balance);
            updateUserBalance(user, user->balance);
        }
        break;
    }
}

int main() {
    struct User user;
    int loggedIn = 0;

    while (1) {
        if (!loggedIn) {
            // NEW OR RETURNING USER
            printf("Are you a returning user? (yes/no/exit): ");
            char response[10];
            scanf("%s", response);

            if (strcmp(response, "yes") == 0) {
                // SIGN IN
                printf("Enter your name: ");
                scanf("%s", user.name);

                printf("Enter your PIN: ");
                scanf("%s", user.pin);

                if (verifyUser(user.name, user.pin)) {
                    printf("\n\n Welcome back, %s!\n", user.name);
                    loggedIn = 1;
                } else {
                    printf("Name or PIN incorrect. Please try again.\n");
                }
            } else if (strcmp(response, "no") == 0) {
                // NEW ACCOUNT
                printf("Enter name: ");
                scanf("%s", user.name);

                printf("Enter email: ");
                scanf("%99s", user.email);

                do {
                    printf("Enter 4-DIGIT PIN: ");
                    scanf("%s", user.pin);
                } while (!isValidPIN(user.pin));

                user.credit_score = 0;
                printf("Enter Credit Score: ");
                scanf("%d", &user.credit_score);

                user.balance = 0.0;
                user.debt = 0.0;

                // Access clients.txt
                FILE *file = fopen("clients.txt", "a");
                if (file == NULL) {
                    printf("ERROR: Could not open or find the 'clients.txt' file.\n");
                    return 0;
                }

                // Save client data
                fprintf(file, "%s | Email: %s | PIN: %s | Credit Score: %d | Balance: %.2lf | Debt: %.2lf\n", user.name, user.email, user.pin, user.credit_score, user.balance, user.debt);
                fclose(file);

                printf("\n\nWelcome, account created!\n");
                loggedIn = 1;
            } else if (strcmp(response, "exit") == 0) {
                printf("Have a great day!\n");
                break;
            } else {
                printf("Invalid response. Please enter 'yes', 'no', or 'exit'.\n");
            }
        } else {
            // LOGGED IN, BANK OPEN
            printf("\nOptions:\n");
            printf("1. Access an ATM\n");
            printf("2. Get a loan\n");
            printf("3. Buy stocks\n");
            printf("4. Logout\n");

            printf("Enter your choice: ");
            int choice;
            scanf("%d", &choice);
            
            switch (choice) {
            case 1:
                accessATM(&user); // ATM: Deposit or Withdraw
                break;
            case 2:
                applyForLoan(&user); // Apply/get a loan
                break;
            case 3:
                buyStocks(&user); //buy stocks
                break;
            case 4:
                // LOGOUT
                loggedIn = 0;
                printf("Logged out successfully.\n");
                break;
            default:
                printf("Invalid choice. Please select a valid option.\n");
            }
        }
    }

    return 0;
}
