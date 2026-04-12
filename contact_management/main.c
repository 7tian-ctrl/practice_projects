/* * Original Authors & Contribution:
 * ----------------------------------------------------------------
 * AUTHOR: 7tian-ctrl
 * Role: Core logic, database functions, user inputs, display, 
 * safety functions, sorting, and searching algorithms.
 *
 * COLLABORATOR: faze-x 
 * Role: File I/O functions and stability fixes (undefined behaviors).
 * ----------------------------------------------------------------
 * This software is licensed under the GNU GPLv3.
 */

//SEQUENCE OF PROGRAM
//
//Libraries used	-	{stdio, stdlib, string, ctype, inttypes}
//Defined types and structures		-	{unsigned 64bit int, unsigned 32bit int, string, contact, database}
//
//FUNCTION PROTOTYPES
//
//Global Variable 	-	{database db}
//Database Handling Functions	-	 {string_compare, create_contact, insert_contact, remove_contact}
//Sorting Functions	-	{merge, sort_merge, sort_db}
//Searching Functions	-	{search_by_name, search_by_phone}
//Display Functions	-	{display, display_contact, display_menu, exit_display}
//User Input Fuctions	-	{string_input, u32_input, phone_input, contact_input}
//Safety Functions	-	{free_database}
//Load Off Functions	-	{boiler_work, contact_insertion_work, ask_contact_info, removal_of_contact, phone_search, name_search}
//Save and Load functions	-	{initial_contacts_path, db_load, db_save}

//MAIN Function		-	{main}

//TO DO LIST
//
//search_by_phone	-	completed
//contact_input		-	completed	--update to check for existing names	completed
//contact_removal	-	completed
//menu			-	completed
//read from file	-			--different file support
//write to file
//multi user files
//authentication
//fix string_input

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif		

//------------------------------------
//defined types and structures

typedef uint64_t u64;
typedef uint32_t u32;
typedef int32_t i32;

typedef struct {
        char* data;
        u32 length;
} string;

typedef struct {
        string name;
        u64 phone;
} contact;

typedef struct {
	contact** arr;
	u32 capacity;
	u32 top;
} database;

//GLOBAL VARIABLE

database db = { .arr = NULL, .top = 0, .capacity = 0 };

//----------------------------------------------------------
// Database file path
//----------------------------------------------------------
#define MAX_PATH_LEN 512
static char CONTACTS_FILE[MAX_PATH_LEN];
void init_contacts_path();

//----------------------------------------------------------
// FUNCTION PROTOTYPES
//----------------------------------------------------------

// String & Memory Utilities
int string_compare(string a, string b);
void free_database(database* db);

// Database Core Operations
contact* create_contact(string name, u64 ph);
u32 insert_contact(database* x, contact* n);
void contact_removal(database* x, string n);

// Sorting (Merge Sort)
void merge(contact** arr, u32 left, u32 mid, u32 right);
void sort_merge(contact** arr, u32 left, u32 right);
void sort_db(database* x);

// Searching
contact* search_by_name(database* x, string target);
contact* search_by_phone(database* x, u64 ph);

// Display & UI
void display_db(database* x);
void display_contact(contact* c);
void display_menu();
void exit_display(database* db);

// User Input Wrappers
string string_input(const char* prompt);
u32 u32_input(const char* prompt);
u64 phone_input();
contact* contact_input(database* db);

// High-Level Logic (Load Off)
void boiler_work(database* db, string s, u64 p);
void contact_insertion_work(database* db, contact* c);
void ask_contact_info(database* db);
void removal_of_contact(database* x);
void name_search(database* x);
void phone_search(database* x);

// Save/Load
void db_load(database* x);
void db_save(database* x);

//----------------------------------------------------------

// MAIN FUNCTION

void main() {	
	
	init_contacts_path();
	db_load(&db);
	
	u32 selection = 0;

	while(1) {
		display_menu();

		selection = u32_input("Selection > ");

		switch(selection) {
			case 1:
				ask_contact_info(&db);
				break;
			case 2:
				display_db(&db);
				break;
			case 3:
				name_search(&db);
				break;
			case 4:
				phone_search(&db);
				break;
			case 5:
				removal_of_contact(&db);
				break;
			case 6:
				exit_display(&db);
				break;
			default:
				printf("Invalid choice. Try again.\n");
				break;
		}
	}
}

//----------------------------------------------------------
//DATABASE HANDLING FUNCTIONS

int string_compare(string a, string b) {
	u32 i = 0;

	if (a.length > b.length) return 1;
	if (a.length < b.length) return -1;

	for (i; i < a.length; i++) {
		if (a.data[i] == b.data[i]) {
			continue;
		}

		if (a.data[i] > b.data[i]) return 1;
		if (a.data[i] < b.data[i]) return -1;
	}

	return 0;
}

contact* create_contact(string name, u64 ph) {
	contact* new = (contact*)malloc(sizeof(contact));

	if(new == NULL) return NULL;

	if (name.data != NULL && name.length > 0) {
		name.data[0] = (char)toupper((unsigned char)name.data[0]);
	}

	new->name = name;
	new->phone = ph;

	return new;
}

u32 insert_contact(database* x, contact* n) {
        if(x->top >= x->capacity) {
                u32 new_capacity = (x->capacity == 0) ? 4 : x->capacity * 2;

                contact** temp = realloc(x->arr, sizeof(contact*) * new_capacity);

                if(temp == NULL) return 1;

                x->arr = temp;
                x->capacity = new_capacity;
        }

        x->arr[x->top++] = n;

        return 0;
}

void contact_removal(database* x, string n) {
	if (x == NULL || x->top == 0) {
		printf("Database is empty.\n");
		return;
	}

	if (n.data == NULL || n.length == 0) {
		printf("\n\t\tEmpty Input.\n");
		return;
	}
	
	u32 low = 0;
	u32 high = x->top - 1;
	int found_index = -1;

	while (low <= high) {
		u32 mid = low + (high - low) / 2;
		int res = string_compare(x->arr[mid]->name, n);

		if (res == 0) {
			found_index = mid;
			break;
		}
		if (res < 0) low = mid + 1;
		
		else {
			if (mid == 0) break;
			high = mid - 1;
		}
	}

	if (found_index == -1) {
		printf("Contact '%s' not found.\n", n.data);
		return;
	}

	free(x->arr[found_index]->name.data);
	free(x->arr[found_index]);

	for (u32 i = found_index; i < x->top - 1; i++) {
		x->arr[i] = x->arr[i + 1];
	}

	x->top--;
	printf("\n\t\tContact '%s' removed.\n", n.data);
}

//--------------------------------------------------------------------------

//-------------------------------------------------------------
//SORTING FUNCTIONS

void merge(contact** arr, u32 left, u32 mid, u32 right) {
	u32 n1 = mid - left + 1;
	u32 n2 = right - mid;

	contact** L = malloc(n1 * sizeof(contact *));
	contact** R = malloc(n2 * sizeof(contact *));

	for (u32 i = 0; i < n1; i++) L[i] = arr[left + i];
	for (u32 j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

	u32 i = 0, j = 0, k = left;


	while (i < n1 && j < n2) {
		if (string_compare(L[i]->name, R[j]->name) <= 0) {
		arr[k++] = L[i++];
		} else {
			arr[k++] = R[j++];
		}
	}


	while (i < n1) arr[k++] = L[i++];
	while (j < n2) arr[k++] = R[j++];

	free(L);
	free(R);
}

void sort_merge(contact** arr, u32 left, u32 right) {
	if (left < right) {
        u32 mid = left + (right - left) / 2;

        sort_merge(arr, left, mid);      
        sort_merge(arr, mid + 1, right); 

        merge(arr, left, mid, right);    
    }
}

void sort_db(database* x) {
    if (x == NULL || x->top < 2) return;
    sort_merge(x->arr, 0, x->top - 1);
}

//--------------------------------------------------------------

//-------------------------------------------------------------
//SEARCHING FUNCTIONS

contact* search_by_name(database* x, string target) {
	if (x == NULL || x->arr == NULL || x->top == 0) return NULL;

	u32 low = 0;
	u32 high = x->top - 1;

	while (low <= high) {
		u32 mid = low + (high - low) / 2; 

		int res = string_compare(x->arr[mid]->name, target);

		if (res == 0) {
			return x->arr[mid];
		}

		if (res < 0) {
			low = mid + 1;
		} else {
			if (mid == 0) break;
			high = mid - 1;
		}
	}

	return NULL;
}

contact* search_by_phone(database* x, u64 ph) {
	if (x == NULL || x->top == 0) return NULL;

	for (u32 i = 0; i < x->top; i++) {
		if (x->arr[i]->phone == ph) {
			return x->arr[i];
		}
	}

	return NULL;	
}

//---------------------------------------------------------------

//----------------------------------------------------------
//DISPLAY FUNCTIONS

void display_db(database* x) {

	if (x == NULL || x->top == 0) {
		printf("Database is empty.\n");
		return;
 	}

	printf("\n--- Contact List (%u contacts) ---\n", x->top);
	printf("%-20s | %-15s\n", "Name", "Phone");
	printf("------------------------------------------\n");

	for (u32 i = 0; i < x->top; i++) {
		
		contact* c = x->arr[i];
		
		printf("%-20s | %llu\n", c->name.data, (unsigned long long)c->phone);
	}
	
	printf("------------------------------------------\n");
}

void display_contact(contact* c) {
	if(c == NULL) {
		printf("\n<<<<<<<<<<<<<<<<<              Does not exist.              >>>>>>>>>>>>>>>>>\n");
		return;
	}
	

	printf("\n\n%-20s | %-15s\n", "Name", "Phone");
	printf("------------------------------------------\n");		

	printf("%-20s | %llu\n\n", c->name.data, (unsigned long long)c->phone);
}

void display_menu() {
	printf("\n\033[1;36m==========================================\033[0m\n");
	printf("\033[1;34m        CONTACT MANAGEMENT SYSTEM         \033[0m\n");
	printf("\033[1;36m==========================================\033[0m\n");
	printf("  [1] \033[32mAdd New Contact\033[0m\n");
	printf("  [2] \033[32mDisplay All Contacts\033[0m\n");
	printf("  [3] \033[32mSearch by Name\033[0m\n");
	printf("  [4] \033[32mSearch by Phone\033[0m\n");
	printf("  [5] \033[31mDelete Contact\033[0m\n");
	printf("  [6] \033[33mExit Program\033[0m\n");
	printf("\033[1;36m------------------------------------------\033[0m\n");
}


void exit_display(database* db) {
	printf("\033[H\033[J"); 

	printf("\n\033[1;33m[!] Shutting down Contact Management System...\033[0m\n");
    
	printf("\033[0;90m  > Saving database... \033[0m\n");
	db_save(db);

	printf("\033[0;90m  > Freeing database memory...\033[0m\n");
	free_database(db);
    
	printf("\033[0;90m  > Closing buffers...\033[0m\n");
    

	printf("\n\033[1;32mDone. Goodbye!\033[0m\n");
	printf("\033[1;36m------------------------------------------\033[0m\n\n");
    
	exit(0);
}

//------------------------------------------------------------------------------

//--------------------------------------------------------------------
//USER INPUT FUNCTIONS

string string_input(const char *prompt) {
    string result = {NULL, 0};

	if (prompt)
    	printf("%s", prompt);

    char *buffer = NULL;
    size_t size = 0;
    size_t capacity = 0;

    int c;

    while ((c = fgetc(stdin)) != '\n' && c != EOF) {
        if (size + 1 > capacity) {
            size_t newcap = (capacity == 0) ? 16 : capacity * 2;

            char *temp = realloc(buffer, newcap);
            if (!temp) {
                free(buffer);
                return result;
            }

            buffer = temp;
            capacity = newcap;
        }

        buffer[size++] = (char)c;
    }

    if (size == 0 && c == EOF) {
        free(buffer);
        return result;
    }

	while (size > 0 && buffer[size - 1] == ' ')
        size--;

    size_t start = 0;
    while (start < size && buffer[start] == ' ')
        start++;

    size -= start;

    if (size + 1 > capacity) {
        char *temp = realloc(buffer, size + 1);
        if (!temp) {
            free(buffer);
            return result;
        }
        buffer = temp;
		capacity = size + 1;
    }
	
	if (start > 0)
        memmove(buffer, buffer + start, size);
		
    buffer[size] = '\0';

    result.data = buffer;
    result.length = (u32)size;

    return result;
}

u32 u32_input(const char *prompt)
{
    while (1) {
        if (prompt)
        	printf("%s", prompt);

        string s = string_input(NULL);

        if (s.data == NULL) {
            return 0; 
        }

        char *ptr = s.data;

        while (isspace((unsigned char)*ptr))
            ptr++;

        if (*ptr == '\0') {
            free(s.data);
            continue;
        }

        char *end;
        errno = 0;

        unsigned long val = strtoul(ptr, &end, 10);

        if (ptr != end && errno == 0) {
            while (isspace((unsigned char)*end))
                end++;

            if (*end == '\0' && val <= UINT32_MAX) {
                free(s.data);
                return (u32)val;
            }
        }

        free(s.data);
        printf("Invalid input. Try again.\n");
    }
}

u64 phone_input() {
    u64 phone;

    while (1) {
        string s = string_input("\033[1;32mEnter Phone Number:\033[0m ");

		if (s.data == NULL) {
            return 0; 
        }

        char *ptr = s.data;
		while (isspace((unsigned char)*ptr))
            ptr++;

		if (*ptr == '\0') {
            free(s.data);
            continue;
        }

		char *p = ptr;
        int digits = 0;
        while (isdigit((unsigned char)*p)) {
            digits++;
            p++;
        }

		char *end = p;
        while (isspace((unsigned char)*end))
            end++;

		if (digits == 10 && *end == '\0') {
            errno = 0;
            unsigned long long val = strtoull(ptr, NULL, 10);

            if (errno == 0) {
                free(s.data);
                return (u64)val;
            }
        }

        free(s.data);
        printf("\033[1;31mInvalid input! Please enter a valid phone number.\033[0m\n");
    }
}

contact* contact_input(database* db) {
    string name = string_input("Please enter the name: ");

    if (name.data == NULL || name.length == 0) {
        free(name.data);
        return NULL;
    }

    name.data[0] = (char)toupper((unsigned char)name.data[0]);

    if (search_by_name(db, name) != NULL) {
        printf("\n\t\tName already exists!\t\tTry again.\n");
        free(name.data);
        return NULL;
    }

    u64 ph = phone_input();

    return create_contact(name, ph);
}

//-------------------------------------------------------------------
//SAFETY FUNCTIONS

void free_database(database* db) {
	if (db == NULL || db->arr == NULL) return;

	for (u32 i = 0; i < db->top; i++) {
		if (db->arr[i]->name.data != NULL) {
			free(db->arr[i]->name.data);
		}
		free(db->arr[i]);
	}

	free(db->arr);    

	db->arr = NULL;
	db->top = 0;
	db->capacity = 0;
}

//-------------------------------------------------------------------

//------------------------------------------------------
//LOAD OFF FUNCTIONS

void boiler_work(database* db, string s, u64 p) {
	contact* a = create_contact(s, p);
	insert_contact(db, a);
	
	return;
}

void contact_insertion_work(database* db, contact* c) {
	insert_contact(db, c);
	return;
}

void ask_contact_info(database* db) {
	contact* c = contact_input(db);
	if (c == NULL) return;
	
	contact_insertion_work(db, c);
	
	sort_db(db);
	display_db(db);
	
	return;
}

void removal_of_contact(database* x) {
	string name = string_input("Please enter the name: ");
	if (name.data != NULL && name.length > 0) {
		name.data[0] = (char)toupper((unsigned char)name.data[0]);
	}
	contact_removal(x, name);
	display_db(x);
}

void name_search(database* x) {
	string name = string_input("Please enter the name: ");
	if (name.data != NULL && name.length > 0) {
		name.data[0] = (char)toupper((unsigned char)name.data[0]);
	}
	contact* c = search_by_name(x, name);

	display_contact(c);
}

void phone_search(database* x) {
	contact* c = search_by_phone(x, phone_input());

	display_contact(c);
}

//------------------------------------------------------
//SAVE/LOAD

void init_contacts_path() {
    char exe_path[MAX_PATH_LEN];

#ifdef _WIN32
    DWORD len = GetModuleFileNameA(NULL, exe_path, MAX_PATH_LEN);
    if (len == 0 || len >= MAX_PATH_LEN) {
		goto fallback;
	}
	exe_path[len] = '\0';

#else
    ssize_t len = readlink("/proc/self/exe", exe_path, MAX_PATH_LEN - 1);
    if (len == -1) goto fallback;
    exe_path[len] = '\0';
#endif

    char *last_sep =
#ifdef _WIN32
        strrchr(exe_path, '\\');
#else
        strrchr(exe_path, '/');
#endif

    if (last_sep) {
        *(last_sep + 1) = '\0';
    }

    // final path
    snprintf(CONTACTS_FILE, MAX_PATH_LEN, "%scontacts.bin", exe_path);
	return;

fallback:
	strcpy(CONTACTS_FILE, "contacts.bin");
}

void db_load(database* x) {
	
    FILE *f = fopen(CONTACTS_FILE, "rb");
    if (!f) return;

    database temp = {0};  // temporary db

    u32 top, capacity;

    if (fread(&top, sizeof(u32), 1, f) != 1) goto fail;
    if (fread(&capacity, sizeof(u32), 1, f) != 1) goto fail;
    if (capacity < top) goto fail;

    temp.arr = malloc(sizeof(contact*) * capacity);
    if (!temp.arr) goto fail;

    temp.capacity = capacity;
    temp.top = 0;

    for (u32 i = 0; i < top; i++) {
        contact *c = malloc(sizeof(contact));
        if (!c) goto fail;

        if (fread(&c->name.length, sizeof(u32), 1, f) != 1) {
            free(c);
            goto fail;
        }

        c->name.data = malloc(c->name.length + 1);
        if (!c->name.data) {
            free(c);
            goto fail;
        }

        if (fread(c->name.data, 1, c->name.length, f) != c->name.length) {
            free(c->name.data);
            free(c);
            goto fail;
        }

        c->name.data[c->name.length] = '\0';

        if (fread(&c->phone, sizeof(u64), 1, f) != 1) {
            free(c->name.data);
            free(c);
            goto fail;
        }

        temp.arr[temp.top++] = c;
    }

    fclose(f);
	
    free_database(x);
    *x = temp;
	
    return;
	
	fail:
    fclose(f);
	printf("\033[31m > Error loading database... \033[0m\n");
    free_database(&temp);
}

void db_save(database* x) {
	FILE *f = fopen("contacts.tmp", "wb");
    if (!f) return;

	// write metadta
	if (fwrite(&x->top, sizeof(u32), 1, f) != 1) goto fail;
    if (fwrite(&x->capacity, sizeof(u32), 1, f) != 1) goto fail;

	// write contacts
	for (u32 i = 0; i < x->top; i++) {
        contact *c = x->arr[i];

        // write name length
        if (fwrite(&c->name.length, sizeof(u32), 1, f) != 1) goto fail;

        // write name 
        if (fwrite(c->name.data, sizeof(char), c->name.length, f) != c->name.length) goto fail;

        // write phone
        if (fwrite(&c->phone, sizeof(u64), 1, f) != 1) goto fail;
    }

    fclose(f);
	remove(CONTACTS_FILE);
    rename("contacts.tmp", CONTACTS_FILE);

    return;

fail:
	fclose(f);
	printf("\033[31m  > Error saving database... \033[0m\n");
	remove("contacts.tmp");
	return;
}
//----------------------------------------------------
