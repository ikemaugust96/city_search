/*
 * saveourpeople.c
 *
 * This program reads the 100 largest U.S. cities from a CSV file,
 * and finds the subset of cities whose total name length is <= 100 characters
 * and whose total population is maximized.
 *
 * It uses a 0/1 knapsack dynamic programming algorithm.
 *
 * Author: wenxuan yang
 * Date: 04/03/2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 4096
#define MAX_CITY_NAME 200
#define MAX_CITIES 100
#define NAME_LIMIT 100  

typedef struct {
    char name[MAX_CITY_NAME];
    int population;
    int name_length;
} City;

// Clean quotes and special characters
void clean_string(char *str) {
    char *src = str;
    char *dst = str;

    if (*src == '"') src++;

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

// Remove commas from numbers
void remove_commas(char *str) {
    char *src = str;
    char *dst = str;
    while (*src) {
        if (*src != ',') *dst++ = *src;
        src++;
    }
    *dst = '\0';
}

// Parse CSV line into a City struct
void parse_csv_line(char *line, City *city) {
    char name[MAX_CITY_NAME] = {0}, population_str[20] = {0};
    int field_count = 0;
    char *field = strtok(line, ",");

    while (field && field_count < 9) {
        clean_string(field);
        if (field_count == 1) { // column 2: city name
            strncpy(name, field, MAX_CITY_NAME - 1);
        } else if (field_count == 8) { // column 9: population
            strncpy(population_str, field, 19);
            remove_commas(population_str);
        }
        field = strtok(NULL, ",");
        field_count++;
    }

    strncpy(city->name, name, MAX_CITY_NAME - 1);
    city->population = atoi(population_str);
    city->name_length = strlen(name);
}

int main() {
    FILE *file = fopen("uscities.csv", "r");  // file assumed to be in same directory
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    City cities[MAX_CITIES];

    fgets(line, MAX_LINE_LENGTH, file); // Skip header

    int count = 0;
    while (count < MAX_CITIES && fgets(line, MAX_LINE_LENGTH, file)) {
        parse_csv_line(line, &cities[count]);
        count++;
    }
    fclose(file);

    long long dp[NAME_LIMIT + 1] = {0};
    int path[MAX_CITIES][NAME_LIMIT + 1] = {0};

    for (int i = 0; i < count; i++) {
        for (int j = NAME_LIMIT; j >= cities[i].name_length; j--) {
            long long new_pop = dp[j - cities[i].name_length] + cities[i].population;
            if (new_pop > dp[j]) {
                dp[j] = new_pop;
                path[i][j] = 1;
            }
        }
    }

    // Find best total population
    int best_char_count = 0;
    for (int i = 1; i <= NAME_LIMIT; i++) {
        if (dp[i] > dp[best_char_count]) {
            best_char_count = i;
        }
    }

    // Traceback in reverse and store results
    char *saved_cities[MAX_CITIES];
    int saved_count = 0;
    int remaining = best_char_count;

    for (int i = count - 1; i >= 0; i--) {
        if (remaining >= cities[i].name_length && path[i][remaining]) {
            saved_cities[saved_count++] = cities[i].name;
            remaining -= cities[i].name_length;
        }
    }

    // Output results
    printf("Total rescued population: %lld\n", dp[best_char_count]);
    printf("Cities saved:\n");

    for (int i = saved_count - 1; i >= 0; i--) {
        printf("- %s\n", saved_cities[i]);
    }

    return 0;
}
