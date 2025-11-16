/*
 * citylist.c
 * 
 * This program reads a specified number of city records from a CSV file and stores them
 * in a dynamically allocated vector using generic pointers. The city data includes:
 * - Name
 * - State
 * - Latitude
 * - Longitude
 * - Population
 * 
 * The program prompts the user for the number of cities to read and prints out the
 * corresponding city details. It creates Binary Search Trees (BSTs) ordered by latitude
 * and longitude, then finds the index of "New York" in both cases.
 * 
 * Author: wenxuan yang
 * Date: 03/13/2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 4096
#define MAX_CITY_NAME 200
#define MAX_CITIES 1000

typedef struct {
    char name[MAX_CITY_NAME];
    char state[10];
    double latitude;
    double longitude;
    int population;
} City;

//Binary Search Tree (BST) Structure
typedef struct bstNode {
    City *city;
    struct bstNode *left;
    struct bstNode *right;
} BSTNode;

// Function to clean quotes and special characters
void clean_string(char *str) {
    char *src = str;
    char *dst = str;

    if (*src == '"') {
        src++;
    }

    while (*src) {
        if (*src == '"' && *(src + 1) == '"') {
            *dst++ = '\\';
            *dst++ = 'u';
            src += 2;
        } else if (*src == '"') {
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// Function to remove commas from numbers
void remove_commas(char *str) {
    char *src = str;
    char *dst = str;
    while (*src) {
        if (*src != ',') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

// Function to parse a CSV line into a City struct
void parse_csv_line(char *line, City *city) {
    if (!line || !city) return;

    char name[MAX_CITY_NAME] = {0}, state[10] = {0}, population_str[20] = {0};
    double latitude = 0.0, longitude = 0.0;

    int field_count = 0;
    char *field = strtok(line, ",");
    while (field && field_count < 9) {  // Stop at column 9
        clean_string(field);
        if (field_count == 1) {  // Column 2: City Name
            strncpy(name, field, MAX_CITY_NAME - 1);
        } else if (field_count == 2) {  // Column 3: State
            strncpy(state, field, 9);
        } else if (field_count == 6) {  // Column 7: Latitude
            latitude = atof(field);
        } else if (field_count == 7) {  // Column 8: Longitude
            longitude = atof(field);
        } else if (field_count == 8) {  // Column 9: Population
            strncpy(population_str, field, 19);
            remove_commas(population_str);
        }
        field = strtok(NULL, ",");
        field_count++;
    }

    // Store values in the City struct
    city->latitude = latitude;
    city->longitude = longitude;
    city->population = atoi(population_str);
    strncpy(city->name, name, MAX_CITY_NAME - 1);
    strncpy(city->state, state, 9);
}

//Function to insert a city into the BST ordered by latitude
BSTNode* insert_bst_latitude(BSTNode *root, City *city) {
    if (!root) {
        BSTNode *new_node = (BSTNode *)malloc(sizeof(BSTNode));
        new_node->city = city;
        new_node->left = new_node->right = NULL;
        return new_node;
    }

    if (city->latitude < root->city->latitude) {
        root->left = insert_bst_latitude(root->left, city);
    } else {
        root->right = insert_bst_latitude(root->right, city);
    }
    return root;
}

//Function to insert a city into BST ordered by longitude
BSTNode* insert_bst_longitude(BSTNode *root, City *city) {
    if (!root) {
        BSTNode *new_node = (BSTNode *)malloc(sizeof(BSTNode));
        new_node->city = city;
        new_node->left = new_node->right = NULL;
        return new_node;
    }

    if (city->longitude < root->city->longitude) {
        root->left = insert_bst_longitude(root->left, city);
    } else {
        root->right = insert_bst_longitude(root->right, city);
    }
    return root;
}

// Inorder traversal to store cities in an array (sorted by latitude) 
void inorder_traversal(BSTNode *root, City **array, int *index) {
    if (!root) return;
    inorder_traversal(root->left, array, index);
    array[*index] = root->city;
    (*index)++;
    inorder_traversal(root->right, array, index);
}

// Linear search to find "New York" in the sorted array 
int linear_search(City **array, int size, const char *target) {
    for (int i = 0; i < size; i++) {
        if (strcmp(array[i]->name, target) == 0) {
            return i;
        }
    }
    return -1;
}

// Function to free BST memory 
void free_bst(BSTNode *root) {
    if (!root) return;
    free_bst(root->left);
    free_bst(root->right);
    free(root);
}

// Function to read cities from CSV and store them in an array
void read_csv_file(int num_cities) {
    FILE *file = fopen("resources/uscities.csv", "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char *line = malloc(MAX_LINE_LENGTH * sizeof(char));
    if (!line) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return;
    }

    fgets(line, MAX_LINE_LENGTH, file);  // Skip header line

    City *cities = malloc(sizeof(City) * num_cities);
    if (!cities) {
        printf("Memory allocation failed.\n");
        free(line);
        fclose(file);
        return;
    }

    int count = 0;
    while (count < num_cities && fgets(line, MAX_LINE_LENGTH, file)) {
        parse_csv_line(line, &cities[count]);
        count++;
    }

    fclose(file);
    free(line);

    // Print results
    for (int i = 0; i < count; i++) {
        printf("\"%s\", \"%s\", population %d, at (%.4f, %.4f)\n",
               cities[i].name, cities[i].state, cities[i].population,
               cities[i].latitude, cities[i].longitude);
    }

    // Create BST ordered by latitude
    BSTNode *lat_root = NULL;
    for (int i = 0; i < count; i++) {
        lat_root = insert_bst_latitude(lat_root, &cities[i]);
    }

    // Storing sorted cities in an array via inorder traversal (sorted by latitude)
    City **lat_sorted_cities = malloc(sizeof(City *) * count);
    int lat_index = 0;
    inorder_traversal(lat_root, lat_sorted_cities, &lat_index);

    // Searching for "New York" using linear search in latitude-sorted array
    int ny_lat_index = linear_search(lat_sorted_cities, count, "New York");
    printf("\nBy latitude, New York is index %d\n", ny_lat_index);

    //  Create BST ordered by longitude
    BSTNode *lon_root = NULL;
    for (int i = 0; i < count; i++) {
        lon_root = insert_bst_longitude(lon_root, &cities[i]);
    }

    // Storing sorted cities in an array via inorder traversal (sorted by longitude)
    City **lon_sorted_cities = malloc(sizeof(City *) * count);
    int lon_index = 0;
    inorder_traversal(lon_root, lon_sorted_cities, &lon_index);

    // Searching for "New York" using linear search in longitude-sorted array
    int ny_lon_index = linear_search(lon_sorted_cities, count, "New York");
    printf("\nBy longitude, New York is index %d\n", ny_lon_index);

    // Cleanup
    free(lat_sorted_cities);
    free(lon_sorted_cities);
    free(cities);
    free_bst(lat_root);
    free_bst(lon_root);
}

int main() {
    int num_cities;
    printf("How many cities: ");
    if (scanf("%d", &num_cities) != 1 || num_cities < 1 || num_cities > MAX_CITIES) {
        printf("Invalid number of cities. Enter between 1-%d.\n", MAX_CITIES);
        return 1;
    }

    read_csv_file(num_cities);
    return 0;
}