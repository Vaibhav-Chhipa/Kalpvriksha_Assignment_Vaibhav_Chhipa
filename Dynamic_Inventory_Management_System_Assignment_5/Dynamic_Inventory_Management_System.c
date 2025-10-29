#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<stdbool.h>

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
    unsigned short productID;
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

unsigned int idExists(const Product * products, unsigned int prodCount, unsigned int inputID){
    for(int index = 0; index < prodCount; index++){
        if(products[index].productID == inputID){
            return 1;
        }
    }
    return 0;
}

void getProductName(char * prodName, const char *prompt){
    while(1){
        printf("%s", prompt);
        if (!fgets(prodName, MAX_NAME_LENGTH + 1, stdin)) {
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
        newProduct->productID = (unsigned short)inputInt("\nProduct ID: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
        if(idExists(*products, *prodCount - 1, newProduct->productID)){
            printf("ID already exists!\n");
        } else {
            break;
        }
    }
    getProductName(newProduct->prodName, "Product Name: ");
    newProduct->prodPrice = inputFloat("Enter Price : ", MIN_PRICE, MAX_PRICE);
    newProduct->prodQuantity = inputInt("Product Quantity: ", MIN_QUANTITY, MAX_QUANTITY);
    printf("Product added successfully!\n");
}

void viewAllProducts(const Product *products, unsigned int prodCount){
    if(prodCount == 0){
        printf("No Product exists in INVENTORY!\n");
        return;
    }
    printf("\n========= PRODUCT LIST =========\n");
    for(int index = 0; index < prodCount; index++){
        printf("Product ID: %hu | Name: %s | Price: %0.2f | Quantity: %u\n", products[index].productID, products[index].prodName, products[index].prodPrice, products[index].prodQuantity);
    }
}

void updateQuantity(Product *products, unsigned int prodCount){
    unsigned short prodID;
    prodID = (unsigned short)inputInt("\nEnter Product ID to update quantity: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
    
    for(int index = 0; index < prodCount; index++){
        if(prodID == products[index].productID){
            unsigned int newQuantity = inputInt("Enter new Quantity: ", MIN_QUANTITY, MAX_QUANTITY);
            products[index].prodQuantity = newQuantity;
            printf("Quantity updated successfully!\n");
            return;
        }
    }
    printf("ID not found!\n");
}

void searchProdByID(const Product *products, unsigned int prodCount){
    unsigned short prodID;
    prodID = (unsigned short)inputInt("\nEnter Product ID to search: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);

    for(int index = 0; index < prodCount; index++){
        if(products[index].productID == prodID){
            printf("Product Found: \nProduct ID: %hu | Name: %s | Price: %0.2f | Quantity: %u\n", products[index].productID, products[index].prodName, products[index].prodPrice, products[index].prodQuantity);
            return;
        }
    }
    printf("ID not found!\n");
}

void searchPrdByName(const Product *products, unsigned int prodCount){
    char findProdName[MAX_NAME_LENGTH+1];
    bool found = false;
    getProductName(findProdName, "Enter name to search (partial allowed): ");
    printf("Product Found:\n");

    for(int index = 0; index < prodCount; index++){
        char tempName[MAX_NAME_LENGTH + 1];
        strcpy(tempName, products[index].prodName);

        strlwr(tempName);
        strlwr(findProdName);

        if (strstr(tempName, findProdName) != NULL) {
            printf("Product ID: %hu | Name: %s | Price: %0.2f | Quantity: %u\n", products[index].productID, products[index].prodName, products[index].prodPrice, products[index].prodQuantity);
            found = true;
        }
    }
    if (!found) {
        printf("No product found matching \"%s\".\n", findProdName);
    }
}

void searchProdByPriceRange(const Product *products, unsigned int prodCount){
    float minRange = inputFloat("Enter minimum price: ", MIN_PRICE, MAX_PRICE);
    float maxRange = inputFloat("Enter maximum price: ", MIN_PRICE, MAX_PRICE);
    bool prodFound = false;

    printf("Products in price range: \n");
    for(int index = 0; index < prodCount; index++){
        if(products[index].prodPrice >= minRange && products[index].prodPrice <= maxRange){
            prodFound = true;
            printf("Product ID: %hu | Name: %s | Price: %0.2f | Quantity: %u\n", products[index].productID, products[index].prodName, products[index].prodPrice, products[index].prodQuantity);
        }
    }
    if(!prodFound){
        printf("No product found in this price range!\n");
    }
}

void deleteProduct(Product **products, unsigned int *prodCount) {
    if (*prodCount == 0) {
        printf("\nNo products to delete.\n");
        return;
    }

    unsigned short id = (unsigned short)inputInt("\nEnter Product ID to delete: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
    bool found = false;
    for (int currIndex = 0; currIndex < *prodCount; currIndex++) {
        if ((*products)[currIndex].productID == id) {
            found = true;
            
            if (*prodCount == 1) {
                free(*products);
                *products = NULL;
                (*prodCount)--;
                printf("Product deleted successfully!\n");
            } else {
                Product *temp = malloc((*prodCount - 1) * sizeof(Product));
                if(temp == NULL){
                    printf("Memory allocation failed! Product not deleted.\n");
                    return;
                }
                
                int newIndex = 0;
                for(int index = 0; index < *prodCount; index++){
                    if(index != currIndex){
                        temp[newIndex++] = (*products)[index];
                    }
                }
                
                free(*products);
                *products = temp;
                (*prodCount)--;
                printf("Product deleted successfully!\n");
            }
            break;
        }
    }

    if (!found) printf("\nProduct with ID %hu not found.\n", id);
}

int main(){
    Product *products = NULL;
    unsigned short choice = 0;
    unsigned int prodCount = 0;
    unsigned short initialCount = (unsigned short)inputInt("Enter initial number of products: ", MIN_PRODUCTS, MAX_PRODUCTS);

    products = calloc(initialCount, sizeof(Product));
    if(products == NULL){
        printf("Memory allocation failed!\n");
        return 1;
    }

    for(int index = 0; index < initialCount; index++){
        while(1){
            printf("\nEnter details for product %d:\n", index + 1);
            products[index].productID = (unsigned short)inputInt("Product ID: ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);
            if(idExists(products, index, products[index].productID)){
                printf("ID already exists!\n");
            } else {
                break;
            }
        }
        getProductName(products[index].prodName, "Product Name: ");
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
        choice = (unsigned short)inputInt("Enter your choice : ", 1, 8);

        switch(choice){
            case 1:
                addNewProduct(&products, &prodCount);
                break;
            case 2:
                viewAllProducts(products, prodCount);
                break;
            case 3:
                updateQuantity(products, prodCount);
                break;
            case 4:
                searchProdByID(products, prodCount);
                break;
            case 5:
                searchPrdByName(products, prodCount);
                break;
            case 6:
                searchProdByPriceRange(products, prodCount);
                break;
            case 7:
                deleteProduct(&products, &prodCount);
                break;
            case 8:
                free(products);
                printf("Memory released successfully. Exiting program...");
                exit(0);
                break;
            default:
                printf("Enter correct choice!");
                break;
        }
    }    
    return 0;
}