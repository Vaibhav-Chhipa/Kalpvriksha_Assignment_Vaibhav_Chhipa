#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define MIN_PRODUCTS 1
#define MAX_PRODUCTS 100
#define MIN_PRODUCT_ID 1
#define MAX_PRODUCT_ID 10000
#define MAX_NAME_LENGTH 50
#define MIN_PRICE 0.0f
#define MAX_PRICE 100000.0f
#define MIN_QUANTITY 0
#define MAX_QUANTITY 1000000

typedef struct{
    unsigned int productID;
    char prodName[MAX_NAME_LENGTH + 1];
    float prodPrice;
    unsigned int prodQuantity;
} Product;

unsigned int inputInt(const char *prompt, unsigned int min, unsigned int max){
    char buffer[100];
    unsigned int value;
    while(1){
        printf("%s", prompt);
        if(!fgets(buffer, sizeof(buffer), stdin)){
            continue;
        }
        if(sscanf(buffer, "%u", &value) != 1 || value < min || value > max){
            printf("Invalid input. Must be between %u and %u.\n", min, max);
            continue;
        }
        return value;
    }
}

unsigned int idExists(Product * products, unsigned int prodCount, unsigned int inputID){
    for(int index = 0; index < prodCount; index++){
        if(products[index].productID == inputID){
            return 1;
        }
    }
    return 0;
}

void inputString(char * prodName, int size, const char *prompt){
    while(1){
        printf("%s", prompt);
        if (!fgets(prodName, size, stdin)) {
            printf("Input error. Try again.\n");
            continue;
        }
        if (prodName[strlen(prodName) - 1] == '\n'){
            prodName[strlen(prodName) - 1] = '\0';
        }
        if (strlen(prodName) == 0) {
            printf("Name cannot be empty.\n");
            continue;
        }
        if (strlen(prodName) > MAX_NAME_LENGTH+1) {
            printf("Name too long (max %d chars).\n", MAX_NAME_LENGTH);
            continue;
        }
        break;
    }
}

float inputFloat(const char *prompt, float minPrice, float maxPrice){
    char buffer[100];
    float price;
    while(1){
        printf("%s", prompt);
        if(!fgets(buffer, sizeof(buffer), stdin)){
            continue;
        }
        if(sscanf(buffer, "%f", &price) != 1 || price < minPrice|| price > maxPrice){
            printf("Invalid input! Must be between %0.2f and %0.2f.\n", minPrice, maxPrice);
            continue;
        }
        return price;
    }
}

void addNewProduct(Product **products, unsigned int *prodCount){
    (*prodCount)++;
    Product *temp = realloc(*products, (*prodCount) * sizeof(Product));
    if(temp == NULL){
        printf("Memory allocation failed!\n");
        (*prodCount)--;
        return;
    }
    *products = temp;

    Product *newProduct = &(*products)[(*prodCount) - 1];
    printf("\nEnter new product details: ");
    while(1){
        newProduct->productID =inputInt("\nProduct ID: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
        if(idExists(*products, *prodCount - 1, newProduct->productID)){
            printf("ID already exists!\n");
        } else {
            break;
        }
    }
    inputString(newProduct->prodName, sizeof(newProduct->prodName), "Product Name: ");
    newProduct->prodPrice = inputFloat("Enter Price : ", MIN_PRICE, MAX_PRICE);
    newProduct->prodQuantity = inputInt("Product Quantity: ", MIN_QUANTITY, MAX_QUANTITY);
    printf("Product added successfully!\n");
}

void viewAllProducts(Product *products, unsigned int prodCount){
    if(prodCount == 0){
        printf("No Product exists in INVENTORY!\n");
        return;
    }
    printf("\n========= PRODUCT LIST =========\n");
    for(int index = 0; index < prodCount; index++){
        printf("Product ID: %u | Name: %s | Price: %0.2f | Quantity: %u\n", products[index].productID, products[index].prodName, products[index].prodPrice, products[index].prodQuantity);
    }
}

void updateQuantity(Product *products, unsigned int prodCount){
    unsigned int prodID;
    prodID = inputInt("\nEnter Product ID to update quantity: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
    if(!idExists(products, prodCount, prodID)){
        printf("ID not found!\n");
        return;
    };
    unsigned int newQuantity = inputInt("Enter new Quantity: ", MIN_QUANTITY, MAX_QUANTITY);
    for(int index = 0; index < prodCount; index++){
        if(prodID == products[index].productID){
            products[index].prodQuantity = newQuantity;
            printf("Quantity updated successfully!\n");
            return;
        }
    }
}

void searchProdByID(Product *products, unsigned int prodCount){
    unsigned int prodID;
    prodID = inputInt("\nEnter Product ID to search: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
    if(!idExists(products, prodCount, prodID)){
        printf("ID not found!\n");
        return;
    };
    for(int index = 0; index < prodCount; index++){
        if(products[index].productID == prodID){
            printf("Product Found: \nProduct ID: %u | Name: %s | Price: %0.2f | Quantity: %u\n", products[index].productID, products[index].prodName, products[index].prodPrice, products[index].prodQuantity);
        }
    }
}

void searchPrdByName(Product *products, unsigned int prodCount){
    char findProdName[MAX_NAME_LENGTH+1];
    unsigned short found = 0;
    inputString(findProdName, sizeof(findProdName), "Enter name to search (partial allowed): ");
    printf("Product Found:\n");

    for (int index = 0; findProdName[index]; index++) {
        findProdName[index] = tolower((char)findProdName[index]);
    }

    for(int index = 0; index < prodCount; index++){
        char tempName[sizeof(products->prodName)];
        strcpy(tempName, products[index].prodName);

        for (int j = 0; tempName[j]; j++) {
            tempName[j] = tolower((char)tempName[j]);
        }

        if (strstr(tempName, findProdName) != NULL) {
            printf("Product ID: %u | Name: %s | Price: %0.2f | Quantity: %u\n", products[index].productID, products[index].prodName, products[index].prodPrice, products[index].prodQuantity);
            found = 1;
        }
    }
    if (!found) {
        printf("No product found matching \"%s\".\n", findProdName);
    }
}

void searchProdByPriceRange(Product *products, unsigned int prodCount){
    float minRange = inputFloat("Enter minimum price: ", MIN_PRICE, MAX_PRICE);
    float maxRange = inputFloat("Enter maximum price: ", MIN_PRICE, MAX_PRICE);
    unsigned short prodFound = 0;

    printf("Products in price range: \n");
    for(int index = 0; index < prodCount; index++){
        if(products[index].prodPrice >= minRange && products[index].prodPrice <= maxRange){
            prodFound = 1;
            printf("Product ID: %u | Name: %s | Price: %0.2f | Quantity: %u\n", products[index].productID, products[index].prodName, products[index].prodPrice, products[index].prodQuantity);
        }
    }
    if(prodFound == 0){
        printf("No product found in this price range!\n");
    }
}

void deleteProduct(Product **products, unsigned int *prodCount) {
    if (*prodCount == 0) {
        printf("\nNo products to delete.\n");
        return;
    }

    unsigned int id = inputInt("\nEnter Product ID to delete: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
    unsigned short found = 0;

    for (int currIndex = 0; currIndex < *prodCount; currIndex++) {
        if ((*products)[currIndex].productID == id) {
            found = 1;
            for (int nextIndex = currIndex; nextIndex < *prodCount - 1; nextIndex++) {
                (*products)[nextIndex] = (*products)[nextIndex + 1];
            }
            (*prodCount)--;
            if (*prodCount == 0) {
                free(*products);
                *products = NULL;
            } else {
                *products = realloc(*products, (*prodCount) * sizeof(Product));
            }
            printf("Product deleted successfully!\n");
            break;
        }
    }

    if (!found) printf("\nProduct with ID %u not found.\n", id);
}

int main(){
    Product *products = NULL;
    unsigned int choice = 0;
    unsigned int prodCount = 0;
    unsigned int initialCount = inputInt("Enter initial number of products: ", MIN_PRODUCTS, MAX_PRODUCTS);

    products = calloc(initialCount, sizeof(Product));
    if(products == NULL){
        printf("Memory allocation failed!\n");
        free(products);
        return 1;
    }

    for(int index = 0; index < initialCount; index++){
        while(1){
            printf("\nEnter details for product %d:\n", index + 1);
            products[index].productID =inputInt("Product ID: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
            if(idExists(products, index, products[index].productID)){
                printf("ID already exists!\n");
            } else {
                break;
            }
        }
        inputString(products[index].prodName, sizeof(products[index].prodName), "Product Name: ");
        products[index].prodPrice = inputFloat("Enter Price : ", MIN_PRICE, MAX_PRICE);
        products[index].prodQuantity = inputInt("Product Quantity: ", MIN_QUANTITY, MAX_QUANTITY);
        prodCount++;
    }

    while(1){
        printf("\n====== INVENTORY MENU ======\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Update Quantity\n");
        printf("4. Search Product by ID\n");
        printf("5. Search Product by Name\n");
        printf("6. Search Product by Price Range\n");
        printf("7. Delete Product\n");
        printf("8. Exit\n");
        choice = inputInt("Enter your choice : ", 1, 8);

        if(choice == 1){
            addNewProduct(&products, &prodCount);
        } else if(choice == 2){
            viewAllProducts(products, prodCount);
        } else if(choice == 3){
            updateQuantity(products, prodCount);
        } else if(choice == 4){
            searchProdByID(products, prodCount);
        } else if(choice == 5){
            searchPrdByName(products, prodCount);
        } else if(choice == 6){
            searchProdByPriceRange(products, prodCount);
        } else if(choice == 7){
            deleteProduct(&products, &prodCount);
        } else{
            free(products);
            printf("Memory released successfully. Exiting program...");
            break;
        }
    }    
    return 0;
}