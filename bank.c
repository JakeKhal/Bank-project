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
};

// VERIFY USER
int verifyUser(const char *name, const char *pin) {
    FILE *file = fopen("clients.txt", "r+");

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, name) != NULL && strstr(line, pin) != NULL) {
            fclose(file);
            return 1; 
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

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, user->name) != NULL) {
            // Update the balance in the line
            char updatedLine[256];
            sprintf(updatedLine, "Name: %s | Email: %s | PIN: %s | Credit Score: %d | Balance: %.2lf\n", user->name, user->email, user->pin, user->credit_score, newBalance);
            fseek(file, -strlen(line), SEEK_CUR); // Move the file pointer back to the beginning of the line
            fputs(updatedLine, file); 
            break;
        }
    }

    fclose(file);
}

// RECORD STOCK PRICE
void recordStockPurchase(const struct User *user, const char *stockName, int shares) {
    FILE *file = fopen("stock_purchases.csv", "a");

    // SAVE TO RECORDS
    char purchaseRecord[100];
    sprintf(purchaseRecord, "%s | %s | %s | %d\n", user->name, user->email, stockName, shares);

    fprintf(file, "%s", purchaseRecord);

    fclose(file);
}

int main() {
    struct User user;
    int loggedIn = 0; 
    FILE *file;

    TickerInfo tickers[MAX_TICKERS]; // used later for stocks
    int numTickers = 0; // used later for stocks
    FILE *stock_file;
    char line2[100];


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
                scanf("%s", user.email);
                
                do {
                    printf("Enter 4-DIGIT PIN: ");
                    scanf("%s", user.pin);
                } while (!isValidPIN(user.pin));
                
                
                printf("Enter your credit score: ");
                scanf("%d", &user.credit_score);

                // Access clients.txt
                FILE *file = fopen("clients.txt", "a");

                user.balance = 0.0;
                
                // Save client data 
                fprintf(file, "Name: %s | Email: %s | PIN: %s | Credit Score: %d | Balance: %.2lf\n", user.name, user.email, user.pin, user.credit_score, user.balance);
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
            printf("1. Open/deposit a checking account\n");
            printf("2. Access an ATM\n");
            printf("3. Get a loan\n");
            printf("4. Buy stocks\n");
            printf("5. Logout\n");
            
            printf("Enter your choice: ");
            int choice;
            scanf("%d", &choice);
            
            switch (choice) {
                case 1:
                    // CHECKING ACCOUNT
                    printf("Enter the deposit amount: ");
                    double initialDeposit;
                    scanf("%lf", &initialDeposit);
                    
                    if (initialDeposit < 0) {
                        printf("Invalid deposit amount.\n");
                    } else {
                        user.balance += initialDeposit;
                        printf("Checking account opened with a balance of $%.2lf\n\n\n", user.balance);
                        updateUserBalance(&user, user.balance); // Update balance in clients.txt
                    }
                    break;
                case 2:
                    // ATM
                    printf("\nBalance: %.2lf \n", user.balance);
                    printf("1. Withdraw\n");
                    printf("2. Deposit\n");
                    printf("Enter your choice: ");
                    int action;
                    scanf("%d", &action);
                    switch (action) {
                        case 1:        
                            printf("Enter the withdrawal amount: ");
                            double withdrawalAmount;
                            scanf("%lf", &withdrawalAmount);
                            
                            if (withdrawalAmount < 0 || withdrawalAmount > user.balance) {
                                printf("Invalid withdrawal amount or insufficient funds.\n");
                            } else {
                                user.balance -= withdrawalAmount;
                                printf("Withdrawal of $%.2lf successful.\nCurrent balance: $%.2lf\n", withdrawalAmount, user.balance);
                                updateUserBalance(&user, user.balance); // Update balance in clients.txt
                            }
                            break;
                        case 2:        
                            printf("Enter the deposit amount: ");
                            double depositAmount;
                            scanf("%lf", &depositAmount);
                            
                            if (depositAmount < 0) {
                                printf("Invalid deposit amount.\n");
                            } else {
                                user.balance += depositAmount;
                                printf("Deposit of $%.2lf successful.\nCurrent balance: %.2lf\n", depositAmount, user.balance);
                                updateUserBalance(&user, user.balance); // Update balance in clients.txt
                            }
                            break;
                    }
                    break;
                case 3:
                    // LOANS 
                    printf("Loan feature is not implemented.\n");
                    break;
                case 4:
                    stock_file = fopen("stocks.csv", "r");

                    //GET TICKERS AND PRICE
                    while (fgets(line2, sizeof(line2), stock_file) && numTickers < MAX_TICKERS) {
                        if (sscanf(line2, "$%[^,],%f", tickers[numTickers].ticker, &tickers[numTickers].price) == 2) {
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
                        return 1;
                    }

                    printf("Price of $%s: $%.2f\n", tickers[choice - 1].ticker, tickers[choice - 1].price);

                    // QUANTITY OF SHARES
                    int shares;
                    printf("Enter the amount of shares you want to purchase: ");
                    scanf("%d", &shares);

                    double total_purchase = shares * (tickers[choice - 1].price);

                    // CONFIRM STOCK PURCHASE
                    printf("Confirm purchase order of: $%f? (yes/no)", total_purchase);
                    char confirm[10];
                    scanf("%s", confirm);
            
                    if (strcmp(confirm, "yes") == 0) {
                        user.balance -= total_purchase;
                        recordStockPurchase(&user, tickers[choice - 1].ticker, shares);
                        printf("Purchase successful!\nCurrent balance: $%.2lf\n", user.balance);
                        updateUserBalance(&user, user.balance); 
                        }
                    else {
                        printf("Order cancelled"); 
                    }

                    break;

                case 5:
                    // LOGOUT OF USER
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
