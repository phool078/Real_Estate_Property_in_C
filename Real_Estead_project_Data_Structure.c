#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 100

// Property structure
typedef struct Property {
    char propertyID[20];
    char categoryType[15];     // Residential / Commercial / Land
    char propertyType[20];     // Office, Plot, Apartment...
    float price;
    float size;
    int floor;
    char area[50];             // City / location name
    char address[100];         // Full address
    char lift[5];              // Yes/No
    char parking[5];           // Yes/No
    char fireExit[5];          // Yes/No
    char ownerEmail[50];       // Owner who added this property
    struct Property *next;
} Property;

// User structure
typedef struct User {
    char name[50];
    char email[50];
    char phone[20];
    char password[20];
    struct User *next;
} User;

// Wishlist structure
typedef struct Wishlist {
    char userEmail[50];
    char propertyID[20];
    char propertyType[20];  // For easier file access later
    struct Wishlist *next;
} Wishlist;

// Global pointers for linked lists
Property *propertyHead = NULL;
User *userHead = NULL;
Wishlist *wishlistHead = NULL;

// Logged in user email
char loggedInEmail[50] = "";

// Global property count to generate unique IDs
int propCount = 0;

// Function prototypes
void welcome();
void mainMenu();
void loadUsers();
void saveUsers();
void loadWishlist();
void saveWishlist();
void loadProperties();
void saveProperties();
void addPropertyToList(Property **head, Property *newProp);
void addUserToList(User **head, User *newUser);
void addWishlistToList(Wishlist **head, Wishlist *newWishlist);
void userSignUp();
int userSignIn();
void userPanel();
void searchPropertiesByType(const char *type);
void filterAndShowProperties(Property *head);
int stringContainsIgnoreCase(const char *str, const char *substr);
void bookOrWishlist(Property *prop);
void addProperty();
void showUserProperties();
void showUserWishlist();
Property* findPropertyByID(const char* id, const char* type);
void freeProperties(Property *head);
void freeUsers(User *head);
void freeWishlist(Wishlist *head);
void toLowerCase(char *str);
int confirmPassword(char *password);
int compareNormalized(const char* s1, const char* s2);

const char *phoneNumber = "01701623316";

// Category file names
const char *categoryFiles[] = {
    "office.txt",
    "plot.txt",
    "apartment.txt",
    "readyflat.txt",
    "house.txt",
    "duplex.txt",
    "factory.txt",
    "resort.txt"
};

// Normalized category types (all lowercase, spaces kept)
const char *categoryTypes[] = {
    "office",
    "plot",
    "apartment",
    "ready flat",
    "house",
    "duplex",
    "factory",
    "resort"
};

int main() {
    loadUsers();
    loadWishlist();
    loadProperties();
    welcome();
    mainMenu();
    // Before exit save all data
    saveUsers();
    saveWishlist();
    saveProperties();
    // Free memory
    freeUsers(userHead);
    freeWishlist(wishlistHead);
    freeProperties(propertyHead);
    return 0;
}

// Welcome message
void welcome() {
    printf("========================================\n");
    printf(" WELCOME TO DREAMSPACE REAL ESTATE PORTAL\n");
    printf(" Find Your Dream Property Easily & Fast!\n");
    printf("========================================\n\n");
}

// Main menu with Create Account option added
void mainMenu() {
    int choice;
    while (1) {
        printf("\nSelect Category or Option:\n");
        for (int i = 0; i < 8; i++) {
            printf("%d. %s\n", i+1, categoryTypes[i]);
        }
        printf("9. Add Property\n");
        printf("10. Sign In / User Panel\n");
        printf("11. Create Account\n");
        printf("12. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar();

        if (choice >= 1 && choice <= 8) {
            searchPropertiesByType(categoryTypes[choice - 1]);
        } else if (choice == 9) {
            if (strlen(loggedInEmail) == 0) {
                printf("You need to login first!\n");
                if (userSignIn() == 0) continue;
            }
            addProperty();
        } else if (choice == 10) {
            if (strlen(loggedInEmail) == 0) {
                if (userSignIn() == 0) continue;
            }
            userPanel();
        } else if (choice == 11) {
            userSignUp();  // Create account option
        } else if (choice == 12) {
            printf("Thank you for using DreamSpace. Goodbye!\n");
            break;
        } else {
            printf("Invalid choice! Try again.\n");
        }
    }
}

// Load users from file
void loadUsers() {
    FILE *fp = fopen("users.txt", "r");
    if (!fp) return;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        User *newUser = malloc(sizeof(User));
        if (sscanf(line, "%49[^,],%49[^,],%19[^,],%19[^\n]",
                   newUser->name, newUser->email, newUser->phone, newUser->password) == 4) {
            newUser->next = NULL;
            addUserToList(&userHead, newUser);
        } else {
            free(newUser);
        }
    }
    fclose(fp);
}

// Save users to file
void saveUsers() {
    FILE *fp = fopen("users.txt", "w");
    if (!fp) return;

    User *temp = userHead;
    while (temp) {
        fprintf(fp, "%s,%s,%s,%s\n", temp->name, temp->email, temp->phone, temp->password);
        temp = temp->next;
    }
    fclose(fp);
}

// Load wishlist
void loadWishlist() {
    FILE *fp = fopen("wishlist.txt", "r");
    if (!fp) return;

    char line[200];
    while (fgets(line, sizeof(line), fp)) {
        Wishlist *newWish = malloc(sizeof(Wishlist));
        if (sscanf(line, "%49[^,],%19[^,],%19[^\n]", newWish->userEmail, newWish->propertyID, newWish->propertyType) == 3) {
            newWish->next = NULL;
            addWishlistToList(&wishlistHead, newWish);
        } else {
            free(newWish);
        }
    }
    fclose(fp);
}

// Save wishlist
void saveWishlist() {
    FILE *fp = fopen("wishlist.txt", "w");
    if (!fp) return;

    Wishlist *temp = wishlistHead;
    while (temp) {
        fprintf(fp, "%s,%s,%s\n", temp->userEmail, temp->propertyID, temp->propertyType);
        temp = temp->next;
    }
    fclose(fp);
}

// Normalize string to lowercase (helper)
void normalizeStr(char *str) {
    int i, j=0;
    char temp[MAX_LEN];
    for (i=0; str[i]; i++) {
        if (str[i] != ' ') {
            temp[j++] = (char)tolower(str[i]);
        }
    }
    temp[j] = '\0';
    strcpy(str, temp);
}

// Compare strings ignoring case and spaces
int compareNormalized(const char* s1, const char* s2) {
    char t1[MAX_LEN], t2[MAX_LEN];
    int i, j;
    // Remove spaces and lowercase s1
    for (i = 0, j = 0; s1[i]; i++) {
        if (s1[i] != ' ') t1[j++] = (char)tolower(s1[i]);
    }
    t1[j] = '\0';
    // Remove spaces and lowercase s2
    for (i = 0, j = 0; s2[i]; i++) {
        if (s2[i] != ' ') t2[j++] = (char)tolower(s2[i]);
    }
    t2[j] = '\0';
    return strcmp(t1, t2) == 0;
}

// Load properties from all category files
void loadProperties() {
    for (int i = 0; i < 8; i++) {
        FILE *fp = fopen(categoryFiles[i], "r");
        if (!fp) continue;

        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            Property *newProp = malloc(sizeof(Property));
            if (sscanf(line, "%19[^,],%14[^,],%19[^,],%f,%f,%d,%49[^,],%99[^,],%4[^,],%4[^,],%4[^,],%49[^\n]",
                newProp->propertyID, newProp->categoryType, newProp->propertyType, &newProp->price, &newProp->size,
                &newProp->floor, newProp->area, newProp->address, newProp->lift, newProp->parking, newProp->fireExit,
                newProp->ownerEmail) == 12) {

                // Normalize propertyType to lowercase & no spaces for uniformity
                toLowerCase(newProp->propertyType);
                // Also remove spaces in propertyType for matching
                normalizeStr(newProp->propertyType);

                // Update propCount based on propertyID
                int num = 0;
                if (sscanf(newProp->propertyID, "P%d", &num) == 1) {
                    if (num > propCount) propCount = num;
                }

                newProp->next = NULL;
                addPropertyToList(&propertyHead, newProp);
            } else {
                free(newProp);
            }
        }
        fclose(fp);
    }
}

// Save properties to respective category files
void saveProperties() {
    // Open 8 files and clear them
    FILE *fps[8];
    for (int i = 0; i < 8; i++) {
        fps[i] = fopen(categoryFiles[i], "w");
        if (!fps[i]) {
            printf("Error opening file %s for writing\n", categoryFiles[i]);
            // Close any opened files
            for (int k = 0; k < i; k++) fclose(fps[k]);
            return;
        }
    }

    Property *temp = propertyHead;
    while (temp) {
        int idx = -1;
        // Normalize propertyType for comparison
        char normalizedPropType[MAX_LEN];
        strcpy(normalizedPropType, temp->propertyType);
        toLowerCase(normalizedPropType);
        normalizeStr(normalizedPropType);

        for (int i = 0; i < 8; i++) {
            if (compareNormalized(normalizedPropType, categoryTypes[i])) {
                idx = i;
                break;
            }
        }
        if (idx != -1) {
            fprintf(fps[idx], "%s,%s,%s,%.2f,%.2f,%d,%s,%s,%s,%s,%s,%s\n",
                temp->propertyID, temp->categoryType, temp->propertyType, temp->price, temp->size,
                temp->floor, temp->area, temp->address, temp->lift, temp->parking, temp->fireExit,
                temp->ownerEmail);
        }
        temp = temp->next;
    }

    for (int i = 0; i < 8; i++) fclose(fps[i]);
}

// Add Property to linked list
void addPropertyToList(Property **head, Property *newProp) {
    if (*head == NULL) {
        *head = newProp;
    } else {
        Property *temp = *head;
        while (temp->next) temp = temp->next;
        temp->next = newProp;
    }
}

// Add User to linked list
void addUserToList(User **head, User *newUser) {
    if (*head == NULL) {
        *head = newUser;
    } else {
        User *temp = *head;
        while (temp->next) temp = temp->next;
        temp->next = newUser;
    }
}

// Add Wishlist to linked list
void addWishlistToList(Wishlist **head, Wishlist *newWish) {
    if (*head == NULL) {
        *head = newWish;
    } else {
        Wishlist *temp = *head;
        while (temp->next) temp = temp->next;
        temp->next = newWish;
    }
}

// Search properties by type (e.g. Office)
void searchPropertiesByType(const char *type) {
    printf("\nSearch filters for %s:\n", type);
    char location[50], categoryFilter[15];
    float minPrice = 0, maxPrice = 0, sizeFilter = 0;
    printf("Enter Location keyword (or press enter to skip): ");
    fgets(location, sizeof(location), stdin);
    location[strcspn(location, "\n")] = 0;

    printf("Enter Category (Residential/Commercial/Land or press enter to skip): ");
    fgets(categoryFilter, sizeof(categoryFilter), stdin);
    categoryFilter[strcspn(categoryFilter, "\n")] = 0;

    printf("Enter Price range minimum (0 to skip): ");
    scanf("%f", &minPrice);
    printf("Enter Price range maximum (0 to skip): ");
    scanf("%f", &maxPrice);

    printf("Enter minimum Size (0 to skip): ");
    scanf("%f", &sizeFilter);
    getchar();

    // Filter and show
    printf("\nMatching properties:\n");
    Property *temp = propertyHead;
    int found = 0;
    while (temp) {
        // Compare normalized propertyType with normalized requested type
        char tempPropType[MAX_LEN];
        strcpy(tempPropType, temp->propertyType);
        toLowerCase(tempPropType);
        normalizeStr(tempPropType);

        char searchType[MAX_LEN];
        strcpy(searchType, type);
        toLowerCase(searchType);
        normalizeStr(searchType);

        if (strcmp(tempPropType, searchType) == 0) {
            // Filters
            int match = 1;

            if (strlen(location) > 0 && !stringContainsIgnoreCase(temp->area, location)) {
                match = 0;
            }
            if (strlen(categoryFilter) > 0 && strcasecmp(temp->categoryType, categoryFilter) != 0) {
                match = 0;
            }
            if (minPrice > 0 && temp->price < minPrice) {
                match = 0;
            }
            if (maxPrice > 0 && temp->price > maxPrice) {
                match = 0;
            }
            if (sizeFilter > 0 && temp->size < sizeFilter) {
                match = 0;
            }

            if (match) {
                printf("\nProperty ID: %s\n", temp->propertyID);
                printf("Category: %s\n", temp->categoryType);
                printf("Type: %s\n", temp->propertyType);
                printf("Price: %.2f\n", temp->price);
                printf("Size: %.2f sqft\n", temp->size);
                printf("Floor: %d\n", temp->floor);
                printf("Area: %s\n", temp->area);
                printf("Address: %s\n", temp->address);
                printf("Lift: %s\n", temp->lift);
                printf("Parking: %s\n", temp->parking);
                printf("Fire Exit: %s\n", temp->fireExit);
                printf("Phone Number (for booking): %s\n", phoneNumber);
                printf("----\n");
                found = 1;
            }
        }
        temp = temp->next;
    }
    if (!found) {
        printf("No matching properties found.\n");
        return;
    }

    char pid[20];
    printf("\nEnter Property ID to Book/Add to Wishlist or press enter to go back: ");
    fgets(pid, sizeof(pid), stdin);
    pid[strcspn(pid, "\n")] = 0;
    if (strlen(pid) == 0) return;

    Property *selected = findPropertyByID(pid, type);
    if (selected) {
        bookOrWishlist(selected);
    } else {
        printf("Invalid Property ID.\n");
    }
}

// Case-insensitive substring search
int stringContainsIgnoreCase(const char *str, const char *substr) {
    if (!str || !substr) return 0;
    char s1[MAX_LEN], s2[MAX_LEN];
    strncpy(s1, str, MAX_LEN);
    strncpy(s2, substr, MAX_LEN);
    s1[MAX_LEN-1] = 0; s2[MAX_LEN-1] = 0;
    toLowerCase(s1);
    toLowerCase(s2);
    return strstr(s1, s2) != NULL;
}

void toLowerCase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = (char)tolower(str[i]);
    }
}

// Book or add to wishlist
void bookOrWishlist(Property *prop) {
    printf("1. Book Property\n2. Add to Wishlist\n3. Go Back\nChoice: ");
    int choice;
    scanf("%d", &choice);
    getchar();

    if (choice == 1 || choice == 2) {
        if (strlen(loggedInEmail) == 0) {
            printf("Please sign in to continue.\n");
            if (userSignIn() == 0) return;
        }
        if (choice == 1) {
            printf("Property booked! Contact phone number: %s\n", phoneNumber);
        } else {
            // Add wishlist
            Wishlist *temp = wishlistHead;
            while(temp) {
                if (strcmp(temp->userEmail, loggedInEmail) == 0 &&
                    strcmp(temp->propertyID, prop->propertyID) == 0) {
                    printf("Property already in wishlist.\n");
                    return;
                }
                temp = temp->next;
            }
            Wishlist *newWish = malloc(sizeof(Wishlist));
            strcpy(newWish->userEmail, loggedInEmail);
            strcpy(newWish->propertyID, prop->propertyID);
            strcpy(newWish->propertyType, prop->propertyType);
            newWish->next = NULL;
            addWishlistToList(&wishlistHead, newWish);
            saveWishlist();
            printf("Property added to wishlist!\n");
        }
    }
}

// Add property
void addProperty() {
    Property *newProp = malloc(sizeof(Property));
    if (!newProp) {
        printf("Memory error!\n");
        return;
    }

    propCount++;
    snprintf(newProp->propertyID, sizeof(newProp->propertyID), "P%04d", propCount);

    printf("Adding New Property (Property ID: %s)\n", newProp->propertyID);

    // Category Type input (Residential/Commercial/Land)
    printf("Enter Category Type (Residential/Commercial/Land): ");
    fgets(newProp->categoryType, sizeof(newProp->categoryType), stdin);
    newProp->categoryType[strcspn(newProp->categoryType, "\n")] = 0;

    // Property Type input (Office, Plot, Apartment...)
    printf("Enter Property Type (Office/Plot/Apartment/Ready Flat/House/Duplex/Factory/Resort): ");
    fgets(newProp->propertyType, sizeof(newProp->propertyType), stdin);
    newProp->propertyType[strcspn(newProp->propertyType, "\n")] = 0;

    // Normalize propertyType for internal consistency
    toLowerCase(newProp->propertyType);
    normalizeStr(newProp->propertyType);

    printf("Enter Price: ");
    scanf("%f", &newProp->price);
    printf("Enter Size (sqft): ");
    scanf("%f", &newProp->size);
    printf("Enter Floor number: ");
    scanf("%d", &newProp->floor);
    getchar();

    printf("Enter Area/Location: ");
    fgets(newProp->area, sizeof(newProp->area), stdin);
    newProp->area[strcspn(newProp->area, "\n")] = 0;

    printf("Enter Full Address: ");
    fgets(newProp->address, sizeof(newProp->address), stdin);
    newProp->address[strcspn(newProp->address, "\n")] = 0;

    printf("Lift (Yes/No): ");
    fgets(newProp->lift, sizeof(newProp->lift), stdin);
    newProp->lift[strcspn(newProp->lift, "\n")] = 0;

    printf("Parking (Yes/No): ");
    fgets(newProp->parking, sizeof(newProp->parking), stdin);
    newProp->parking[strcspn(newProp->parking, "\n")] = 0;

    printf("Fire Exit (Yes/No): ");
    fgets(newProp->fireExit, sizeof(newProp->fireExit), stdin);
    newProp->fireExit[strcspn(newProp->fireExit, "\n")] = 0;

    // Owner email from logged in user
    strcpy(newProp->ownerEmail, loggedInEmail);

    newProp->next = NULL;
    addPropertyToList(&propertyHead, newProp);

    saveProperties();

    printf("Property added successfully!\n");
}

// User signup
void userSignUp() {
    User *newUser = malloc(sizeof(User));
    if (!newUser) {
        printf("Memory error!\n");
        return;
    }

    printf("Enter your full name: ");
    fgets(newUser->name, sizeof(newUser->name), stdin);
    newUser->name[strcspn(newUser->name, "\n")] = 0;

    printf("Enter your email: ");
    fgets(newUser->email, sizeof(newUser->email), stdin);
    newUser->email[strcspn(newUser->email, "\n")] = 0;

    // Check if email already exists
    User *temp = userHead;
    while (temp) {
        if (strcasecmp(temp->email, newUser->email) == 0) {
            printf("Email already registered! Try signing in.\n");
            free(newUser);
            return;
        }
        temp = temp->next;
    }

    printf("Enter your phone number: ");
    fgets(newUser->phone, sizeof(newUser->phone), stdin);
    newUser->phone[strcspn(newUser->phone, "\n")] = 0;

    printf("Enter password (min 6 chars): ");
    fgets(newUser->password, sizeof(newUser->password), stdin);
    newUser->password[strcspn(newUser->password, "\n")] = 0;

    if (!confirmPassword(newUser->password)) {
        printf("Password confirmation failed!\n");
        free(newUser);
        return;
    }

    newUser->next = NULL;
    addUserToList(&userHead, newUser);
    saveUsers();

    printf("Account created successfully! You can now sign in.\n");
}

// Confirm password (ask re-enter)
int confirmPassword(char *password) {
    char confirm[20];
    printf("Re-enter password: ");
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = 0;
    return strcmp(password, confirm) == 0;
}

// User sign-in
int userSignIn() {
    char email[50], password[20];
    printf("Enter Email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;
    printf("Enter Password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    User *temp = userHead;
    while (temp) {
        if (strcasecmp(temp->email, email) == 0 && strcmp(temp->password, password) == 0) {
            strcpy(loggedInEmail, temp->email);
            printf("Sign-in successful! Welcome, %s\n", temp->name);
            return 1;
        }
        temp = temp->next;
    }
    printf("Invalid email or password.\n");
    return 0;
}

// User panel
void userPanel() {
    int choice;
    while (1) {
        printf("\nUser Panel\n");
        printf("1. Add Property\n");
        printf("2. View My Properties\n");
        printf("3. View Wishlist\n");
        printf("4. Sign Out\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            addProperty();
        } else if (choice == 2) {
            showUserProperties();
        } else if (choice == 3) {
            showUserWishlist();
        } else if (choice == 4) {
            loggedInEmail[0] = 0;
            printf("Signed out.\n");
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
}

// Show user's properties
void showUserProperties() {
    printf("\nYour Properties:\n");
    int count = 0;
    Property *temp = propertyHead;
    while (temp) {
        if (strcmp(temp->ownerEmail, loggedInEmail) == 0) {
            printf("ID: %s, Type: %s, Price: %.2f, Area: %s\n", temp->propertyID, temp->propertyType, temp->price, temp->area);
            count++;
        }
        temp = temp->next;
    }
    if (count == 0) {
        printf("You have no properties listed.\n");
    }
}

// Show user's wishlist
void showUserWishlist() {
    printf("\nYour Wishlist:\n");
    int count = 0;
    Wishlist *temp = wishlistHead;
    while (temp) {
        if (strcmp(temp->userEmail, loggedInEmail) == 0) {
            printf("Property ID: %s, Type: %s\n", temp->propertyID, temp->propertyType);
            count++;
        }
        temp = temp->next;
    }
    if (count == 0) {
        printf("Your wishlist is empty.\n");
    }
}

// Find property by ID and type
Property* findPropertyByID(const char* id, const char* type) {
    Property *temp = propertyHead;
    char normalizedType[MAX_LEN];
    strcpy(normalizedType, type);
    toLowerCase(normalizedType);
    normalizeStr(normalizedType);

    while (temp) {
        char tempType[MAX_LEN];
        strcpy(tempType, temp->propertyType);
        toLowerCase(tempType);
        normalizeStr(tempType);
        if (strcmp(temp->propertyID, id) == 0 && strcmp(tempType, normalizedType) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// Free linked lists
void freeProperties(Property *head) {
    while (head) {
        Property *temp = head;
        head = head->next;
        free(temp);
    }
}
void freeUsers(User *head) {
    while (head) {
        User *temp = head;
        head = head->next;
        free(temp);
    }
}
void freeWishlist(Wishlist *head) {
    while (head) {
        Wishlist *temp = head;
        head = head->next;
        free(temp);
    }
}
